/******************************************************************
 * Copyright (C) 2004 by SG Software.
 ******************************************************************/

/* FILE NAME   : wnd.h
 * PURPOSE     : MPFC Window Library. Interface for basic window
 *               functions.
 * PROGRAMMER  : Sergey Galanov
 * LAST UPDATE : 23.07.2004
 * NOTE        : Module prefix 'wnd'.
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

#ifndef __SG_MPFC_WND_H__
#define __SG_MPFC_WND_H__

#include <curses.h>
#include "types.h"
#include "cfg.h"
#include "wnd_common_msg.h"
#include "wnd_def_handlers.h"
#include "wnd_kbd.h"
#include "wnd_mouse.h"
#include "wnd_msg.h"
#include "wnd_print.h"
#include "wnd_root.h"

/* Window flags */
typedef enum
{
	WND_FLAG_ROOT 			= 1 << 0,
	WND_FLAG_BORDER 		= 1 << 1,
	WND_FLAG_CAPTION		= 1 << 2,
	WND_FLAG_CLOSE_BOX		= 1 << 3,
	WND_FLAG_MAX_BOX		= 1 << 4,
	WND_FLAG_FULL_BORDER	= WND_FLAG_BORDER | WND_FLAG_CAPTION | 
								WND_FLAG_CLOSE_BOX | WND_FLAG_MAX_BOX,
	WND_FLAG_OWN_DECOR		= 1 << 5,
	WND_FLAG_MAXIMIZED		= 1 << 6,
} wnd_flags_t;

/* States for pushing/popping */
typedef enum
{
	WND_STATE_FG_COLOR	= 1 << 0,
	WND_STATE_BG_COLOR	= 1 << 1,
	WND_STATE_COLOR		= (WND_STATE_FG_COLOR|WND_STATE_BG_COLOR),
	WND_STATE_ATTRIB	= 1 << 2,
	WND_STATE_CURSOR	= 1 << 3,
	WND_STATE_ALL		= 0xFFFFFFFF
} wnd_state_t;

/* Current window mode */
typedef enum
{
	WND_MODE_NORMAL,
	WND_MODE_REPOSITION,
	WND_MODE_RESIZE
} wnd_mode_t;

/* Window destructor type */
struct tag_wnd_t;
typedef void (*wnd_destructor_t)( struct tag_wnd_t *wnd );

/* Window system global data structure */
typedef struct tag_wnd_global_data_t
{
	/* Root window */
	struct tag_wnd_t *m_root;

	/* Focus window */
	struct tag_wnd_t *m_focus;

	/* NCURSES window */
	WINDOW *m_curses_wnd;

	/* The last allocated window ID */
	dword m_last_id;
	
	/* Windows states stack */
#define WND_STATES_STACK_SIZE 32
	struct wnd_state_data_t
	{
		struct tag_wnd_t *m_wnd;
		wnd_state_t m_mask;
		wnd_color_t m_fg_color;
		wnd_color_t m_bg_color;
		int m_attrib;
		int m_cursor_x, m_cursor_y;
	} m_states_stack[WND_STATES_STACK_SIZE];
	int m_states_stack_pos;

	/* Keyboard module data */
	wnd_kbd_data_t *m_kbd_data;

	/* Message queue */
	wnd_msg_queue_t *m_msg_queue;

	/* Display buffers */
	struct wnd_display_buf_t
	{
		struct wnd_display_buf_symbol_t
		{
			chtype m_char;
			int m_attr;
		} *m_data;
		int m_width, m_height;
		bool_t m_dirty;
	} m_display_buf;

	/* Mouse data */
	wnd_mouse_data_t m_mouse_data;
} wnd_global_data_t;

/* Window type */
typedef struct tag_wnd_t
{
	/* Window title */
	char *m_title;

	/* Window flags */
	wnd_flags_t m_flags;

	/* Parent window */
	struct tag_wnd_t *m_parent;

	/* First child and next and siblings */
	struct tag_wnd_t *m_child, *m_next, *m_prev;

	/* Sibling with next z value */
	struct tag_wnd_t *m_lower_sibling;

	/* The number of children */
	int m_num_children;

	/* Focus child */
	struct tag_wnd_t *m_focus_child;

	/* Message handlers. Chain for each message doesn't contain
	 * default handler (so respective field is NULL if only default
	 * handler is used). It is called anyway by dispatcher unless 
	 * previous-level handler has stopped handling message */
	wnd_msg_handler_t *m_on_display,
					  *m_on_keydown,
					  *m_on_close,
					  *m_on_erase_back,
					  *m_on_update_screen,
					  *m_on_parent_repos,
					  *m_on_mouse_ldown,
					  *m_on_mouse_mdown,
					  *m_on_mouse_rdown,
					  *m_on_mouse_ldouble,
					  *m_on_mouse_mdouble,
					  *m_on_mouse_rdouble;

	/* Window destructors chain. It is implemented like message handlers,
	 * though it is not called by dispatcher. */
	wnd_msg_handler_t *m_destructor;

	/* Window position (in parent window and screen coordinates) */
	int m_x, m_y;
	int m_screen_x, m_screen_y;

	/* Window size */
	int m_width, m_height;

	/* Client area */
	int m_client_x, m_client_y, m_client_w, m_client_h;

	/* Cursor position (in client coordinates) */
	int m_cursor_x, m_cursor_y;

	/* Window position before maximization */
	struct
	{
		int x, y, w, h;
	} m_pos_before_max;

	/* Is cursor hidden? */
	bool_t m_cursor_hidden;
	
	/* Current symbol color and attributes */
	wnd_color_t m_fg_color, m_bg_color;
	int m_attrib;

	/* Current window mode */
	wnd_mode_t m_mode;

	/* Window z-value */
	int m_zval;

	/* Window identifier */
	int m_id;

	/* Configuration list for storing window parameters */
	cfg_node_t *m_cfg_list;

	/* Global data */
	wnd_global_data_t *m_global;
} wnd_t;

/* Helper macros */
#define WND_OBJ(wnd)		((wnd_t *)(wnd))
#define WND_FLAGS(wnd)		(WND_OBJ(wnd)->m_flags)
#define WND_WIDTH(wnd)		(WND_OBJ(wnd)->m_client_w)
#define WND_HEIGHT(wnd)		(WND_OBJ(wnd)->m_client_h)
#define WND_IS_ROOT(wnd)	(WND_FLAGS(wnd) & WND_FLAG_ROOT)

/* Access global data */
#define WND_GLOBAL(wnd)			(WND_OBJ(wnd)->m_global)
#define WND_ROOT(wnd)			(WND_GLOBAL(wnd)->m_root)
#define WND_FOCUS(wnd)			(WND_GLOBAL(wnd)->m_focus)
#define WND_CURSES(wnd)			(WND_GLOBAL(wnd)->m_curses_wnd)
#define WND_LAST_ID(wnd)		(WND_GLOBAL(wnd)->m_last_id)
#define WND_DISPLAY_BUF(wnd)	(WND_GLOBAL(wnd)->m_display_buf)
#define WND_MSG_QUEUE(wnd)		(WND_GLOBAL(wnd)->m_msg_queue)
#define WND_KBD_DATA(wnd)		(WND_GLOBAL(wnd)->m_kbd_data)
#define WND_MOUSE_DATA(wnd)		(&(WND_GLOBAL(wnd)->m_mouse_data))
#define WND_STATES_STACK(wnd)	(WND_GLOBAL(wnd)->m_states_stack)
#define WND_STATES_POS(wnd)		(WND_GLOBAL(wnd)->m_states_stack_pos)

/* Convert client coordinates to absolute window or screen */
#define WND_CLIENT2ABS_X(wnd, x) (WND_OBJ(wnd)->m_client_x + (x))
#define WND_CLIENT2ABS_Y(wnd, y) (WND_OBJ(wnd)->m_client_y + (y))
#define WND_CLIENT2SCREEN_X(wnd, x) (WND_OBJ(wnd)->m_screen_x + \
		WND_OBJ(wnd)->m_client_x + (x))
#define WND_CLIENT2SCREEN_Y(wnd, y) (WND_OBJ(wnd)->m_screen_y + \
		WND_OBJ(wnd)->m_client_y + (y))

/* A macro for closing window */
#define wnd_close(wnd) (wnd_msg_send(wnd, WND_MSG_CLOSE, \
			wnd_msg_data_close_new()))

/* Initialize window system */
wnd_t *wnd_init( cfg_node_t *cfg_list );

/* Create a window */
wnd_t *wnd_new( char *title, wnd_t *parent, int x, int y, 
				int width, int height, dword flags );

/* Initialize window fields
 * Main job for creating window is done here. Constructors for
 * various window classes should call this function to initialize 
 * common window part. */
bool_t wnd_construct( wnd_t *wnd, char *title, wnd_t *parent, int x, int y,
		int width, int height, dword flags );

/* Run main window loop */
void wnd_main( wnd_t *wnd_root );

/* Invalidate window */
void wnd_invalidate( wnd_t *wnd );

/* Push window states */
void wnd_push_state( wnd_t *wnd, wnd_state_t mask );

/* Pop window states */
void wnd_pop_state( wnd_t *wnd );

/* Set focus window */
void wnd_set_focus( wnd_t *wnd );

/* Set focus to the next child of this window */
void wnd_next_focus( wnd_t *wnd );

/* Set focus to the previous child of this window */
void wnd_prev_focus( wnd_t *wnd );

/* Initialize color pairs array */
void wnd_init_pairs( void );

/* Convert color is our format to color is NCURSES format */
int wnd_color_our2curses( wnd_color_t col );

/* Draw window decorations */
void wnd_draw_decorations( wnd_t *wnd );

/* Display windows bar (in root window) */
void wnd_display_wnd_bar( wnd_t *wnd_root );

/* Get window setting value */
char *wnd_get_setting( wnd_t *wnd, char *name );

/* Set window style */
void wnd_set_style( wnd_t *wnd, char *name );

/* Parse style value */
void wnd_parse_style( char *str, wnd_color_t *fg_color, wnd_color_t *bg_color,
		int *attrib );

/* Get color from its textual representation */
wnd_color_t wnd_string2color( char *str );

/* Get attribute from its textual representation */
int wnd_string2attrib( char *str );

/* Synchronize screen contents with the display buffer */
void wnd_sync_screen( wnd_t *wnd );

/* Call message handlers chain */
wnd_msg_retcode_t wnd_call_handler( wnd_t *wnd, wnd_msg_handler_t *h,
		wnd_msg_callback_t callback, wnd_msg_data_t *data );

/* Call window destructor */
void wnd_call_destructor( wnd_t *wnd );

/* Callback for destructor */
wnd_msg_retcode_t wnd_callback_destructor( wnd_t *wnd, wnd_msg_handler_t *h,
		wnd_msg_data_t *data );

/* Set window mode */
void wnd_set_mode( wnd_t *wnd, wnd_mode_t mode );

/* Handler for key pressing when in reposition or resize window mode */
wnd_msg_retcode_t wnd_repos_on_key( wnd_t *wnd, wnd_key_t *keycode );

/* Move and resize window */
void wnd_repos( wnd_t *wnd, int x, int y, int width, int height );

/* Toggle the window maximization flag */
void wnd_toggle_maximize( wnd_t *wnd );

/* Check if specified window is another windows descendant */
bool_t wnd_is_descendant( wnd_t *wnd, wnd_t *base );

/* Reset the global focus */
void wnd_set_global_focus( wnd_global_data_t *global );

/* Redisplay the screen (discarding all the optimization stuff) */
void wnd_redisplay( wnd_t *wnd );

#endif

/* End of 'wnd.h' file */

