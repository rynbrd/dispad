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

#ifndef __MTRACKD_CONTROL__
#define __MTRACKD_CONTROL__

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

#define MTRACKD_MAX_DEVICES 4

typedef struct {
	char* property_name;
	Atom property;
	unsigned int start_values[MTRACKD_MAX_DEVICES];
	unsigned int enable_value;
	unsigned int disable_value;
	Display* display;
	XDevice* devices[MTRACKD_MAX_DEVICES];
	int device_count;
	int device_state[MTRACKD_MAX_DEVICES];
} Control;

/* Initialize a Control object. Returns False on error.
 */
Bool control_init(Control* obj, Display* display, char* property_name,
		int enable_value, int disable_value);

/* Find and load devices to control. Blocks until devices are found.
 */
void control_find_devices(Control* obj);

/* Free a Control object.
 */
void control_free(Control* obj);

/* Toggle the touchpads on/off.
 */
void control_toggle(Control* obj, int enable);

#endif

