/******************************************************************
 * Copyright (C) 2004 by SG Software.
 ******************************************************************/

/* FILE NAME   : wnd_msg.c
 * PURPOSE     : MPFC Window Library. Message functions 
 *               implementation.
 * PROGRAMMER  : Sergey Galanov
 * LAST UPDATE : 28.06.2004
 * NOTE        : Module prefix 'wnd_msg'.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
 * MA 02111-1307, USA.
 */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "wnd.h"
#include "wnd_msg.h"

/* Initialize message queue */
wnd_msg_queue_t *wnd_msg_queue_init( void )
{
	wnd_msg_queue_t *queue;

	/* Allocate memory */
	queue = (wnd_msg_queue_t *)malloc(sizeof(wnd_msg_queue_t));
	if (queue == NULL)
		return NULL;

	/* Set fields */
	queue->m_base = NULL;
	queue->m_last = NULL;
	pthread_mutex_init(&queue->m_mutex, NULL);
	return queue;
} /* End of 'wnd_msg_queue_init' function */

/* Get a message from queue */
bool_t wnd_msg_get( wnd_msg_queue_t *queue, wnd_msg_t *msg )
{
	assert(queue);

	/* Queue is empty */
	if (queue->m_base == NULL)
		return FALSE;

	/* Save message */
	*msg = queue->m_base->m_msg;

	/* Delete from the queue */
	wnd_msg_rem(queue, queue->m_base);
	return TRUE;
} /* End of 'wnd_msg_get' function */

/* Remove a given item from the queue */
void wnd_msg_rem( wnd_msg_queue_t *queue, struct wnd_msg_queue_item_t *item )
{
	assert(queue);
	assert(item);

	/* Lock queue */
	wnd_msg_lock_queue(queue);

	/* Remove message from queue */
	if (item->m_next != NULL)
		item->m_next->m_prev = item->m_prev;
	else
		queue->m_last = item->m_prev;
	if (item->m_prev != NULL)
		item->m_prev->m_next = item->m_next;
	else
		queue->m_base = item->m_next;
	free(item);

	/* Unlock queue */
	wnd_msg_unlock_queue(queue);
} /* End of 'wnd_msg_rem' function */

/* Send a message */
void wnd_msg_send( wnd_t *wnd, wnd_msg_type_t type, wnd_msg_data_t data )
{
	wnd_msg_queue_t *queue;
	struct wnd_msg_queue_item_t *node;

	assert(wnd);
	assert(WND_GLOBAL(wnd));

	/* Lock queue */
	queue = WND_MSG_QUEUE(wnd);
	assert(queue);
	wnd_msg_lock_queue(queue);

	/* Allocate new queue node */
	node = (struct wnd_msg_queue_item_t *)malloc(
			sizeof(struct wnd_msg_queue_item_t));
	node->m_msg.m_wnd = wnd;
	node->m_msg.m_type = type;
	node->m_msg.m_data = data;
	node->m_next = NULL;
	node->m_prev = NULL;

	/* Add it to the list */
	if (queue->m_last == NULL)
	{
		queue->m_base = queue->m_last = node;
	}
	else
	{
		queue->m_last->m_next = node;
		node->m_prev = queue->m_last;
		queue->m_last = node;
	}
	
	/* Unlock queue */
	wnd_msg_unlock_queue(queue);
} /* End of 'wnd_msg_send' function */

/* Lock message queue */
void wnd_msg_lock_queue( wnd_msg_queue_t *queue )
{
	pthread_mutex_lock(&queue->m_mutex);
} /* End of 'wnd_msg_lock_queue' function */

/* Unlock message queue */
void wnd_msg_unlock_queue( wnd_msg_queue_t *queue )
{
	pthread_mutex_unlock(&queue->m_mutex);
} /* End of 'wnd_msg_unlock_queue' function */

/* Free message queue */
void wnd_msg_queue_free( wnd_msg_queue_t *queue )
{
	struct wnd_msg_queue_item_t *ptr;

	assert(queue);

	/* Free queue */
	wnd_msg_lock_queue(queue);
	for ( ptr = queue->m_base; ptr != NULL; )
	{
		struct wnd_msg_queue_item_t *next = ptr->m_next;
		wnd_msg_free(&ptr->m_msg);
		free(ptr);
		ptr = next;
	}
	wnd_msg_unlock_queue(queue);

	/* Destroy mutex */
	pthread_mutex_destroy(&queue->m_mutex);
	free(queue);
} /* End of 'wnd_msg_queue_free' function */

/* Free message data */
void wnd_msg_free( wnd_msg_t *msg )
{
	assert(msg);
	if (msg->m_data.m_data != NULL)
	{
		if (msg->m_data.m_destructor != NULL)
			(msg->m_data.m_destructor)(msg->m_data.m_data);
		free(msg->m_data.m_data);
	}
} /* End of 'wnd_msg_free' function */

/* Add a handler to the handlers chain */
void wnd_msg_add_handler( wnd_msg_handler_t **chain, void *h )
{
	wnd_msg_handler_t *handler;

	assert(chain);
	assert(h);

	/* Allocate memory for handler item */
	handler = (wnd_msg_handler_t *)malloc(sizeof(*handler));
	assert(handler);
	handler->m_func = h;

	if (*chain == NULL)
	{
		*chain = handler;
		handler->m_next = NULL;
	}
	else
	{
		handler->m_next = *chain;
		*chain = handler;
	}
} /* End of 'wnd_msg_add_handler' function */

/* Remove handler from the handlers chain beginning */
void wnd_msg_rem_handler( wnd_msg_handler_t **chain )
{
	wnd_msg_handler_t *next;

	assert(chain);
	assert(*chain);

	/* Delete handler and move pointer */
	next = (*chain)->m_next;
	free(*chain);
	*chain = next;
} /* End of 'wnd_msg_rem_handler' function */

/* Remove messages with specified target window */
void wnd_msg_queue_remove_by_window( wnd_msg_queue_t *queue, wnd_t *wnd,
		bool with_descendants )
{
	struct wnd_msg_queue_item_t *item;

	assert(queue);
	assert(wnd);

	for ( item = queue->m_base; item != NULL; )
	{
		wnd_t *target = item->m_msg.m_wnd;
		if (target == wnd || (with_descendants && 
					wnd_is_descendant(wnd, target)))
		{
			struct wnd_msg_queue_item_t *prev = item->m_prev;
			wnd_msg_rem(queue, item);
			if (prev == NULL)
				item = queue->m_base;
			else
				item = prev->m_next;
		}
		else
			item = item->m_next;
	}
} /* End of 'wnd_msg_queue_remove_by_window' function */

/* End of 'wnd_msg.c' file */

