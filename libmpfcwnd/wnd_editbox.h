/******************************************************************
 * Copyright (C) 2004 by SG Software.
 ******************************************************************/

/* FILE NAME   : wnd_editbox.h
 * PURPOSE     : MPFC Window Library. Interface for edit box
 *               functions.
 * PROGRAMMER  : Sergey Galanov
 * LAST UPDATE : 16.08.2004
 * NOTE        : Module prefix 'editbox'.
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

#ifndef __SG_MPFC_WND_EDITBOX_H__
#define __SG_MPFC_WND_EDITBOX_H__

#include "types.h"
#include "mystring.h"
#include "wnd.h"
#include "wnd_dlgitem.h"

/* Edit box type */
typedef struct
{
	/* Dialog item part */
	dlgitem_t m_wnd;

	/* Edit box text */
	str_t *m_text;

	/* Cursor position */
	int m_cursor;

	/* Scroll value */
	int m_scrolled;

	/* The desired width */
	int m_width;
} editbox_t;

/* Convert window object to edit box type */
#define EDITBOX_OBJ(wnd)	((editbox_t *)wnd)

/* Access edit box data */
#define EDITBOX_TEXT(wnd)	(STR_TO_CPTR(EDITBOX_OBJ(wnd)->m_text))
#define EDITBOX_LEN(wnd)	(STR_LEN(EDITBOX_OBJ(wnd)->m_text))

/* Create a new edit box */
editbox_t *editbox_new( wnd_t *parent, char *id, char *text, int width );

/* Edit box constructor */
bool_t editbox_construct( editbox_t *eb, wnd_t *parent, char *id, char *text,
		int width );

/* Destructor */
void editbox_destructor( wnd_t *wnd );

/* Get edit box desired size */
void editbox_get_desired_size( dlgitem_t *di, int *width, int *height );

/* Set edit box text */
void editbox_set_text( editbox_t *eb, char *text );

/* Add character into the current cursor position */
void editbox_addch( editbox_t *eb, char ch );

/* Delete character from the current or previous cursor position */
void editbox_delch( editbox_t *eb, int pos );

/* Move cursor */
void editbox_move( editbox_t *eb, int new_pos );

/* 'display' message handler */
wnd_msg_retcode_t editbox_on_display( wnd_t *wnd );

/* 'keydown' message handler */
wnd_msg_retcode_t editbox_on_keydown( wnd_t *wnd, wnd_key_t key );

/* 'mouse_ldown' message handler */
wnd_msg_retcode_t editbox_on_mouse( wnd_t *wnd, int x, int y,
		wnd_mouse_button_t mb, wnd_mouse_event_t type );

#endif

/* End of 'wnd_editbox.h' file */

