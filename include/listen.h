/***************************************************************************
 *
 * dispad - Disable trackpads on keyboard events.
 * Copyright (C) 2011 Ryan Bourgeois <bluedragonx@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **************************************************************************/

#ifndef __MTRACKD_LISTEN__
#define __MTRACKD_LISTEN__

#include <X11/Xlib.h>
#include "control.h"

#define MTRACKD_KEYMAP_SIZE 32

typedef struct {
	Bool modifiers;
	double idle_time;
	int poll_time;
	Display* display;
	unsigned char mask[MTRACKD_KEYMAP_SIZE];
	unsigned char current[MTRACKD_KEYMAP_SIZE];
	unsigned char previous[MTRACKD_KEYMAP_SIZE];
} Listen;

/* Initialize a listener object. Returns False on error.
 */
Bool listen_init(Listen* obj, Display* display, Bool modifiers,
		int idle_time, int poll_time);

/* Run the listener loop. Calls control_toggle on the given Control object.
 */
void listen_run(Listen* obj, Control* ctrl);

#endif

