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

#include "listen.h"
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static double now() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void clear_bit(unsigned char *ptr, int bit)
{
    int byte_num = bit / 8;
    int bit_num = bit % 8;
    ptr[byte_num] &= ~(1 << bit_num);
}

static Bool listen_activity(Listen* obj) {
	int i;
	Bool res = False;
	XQueryKeymap(obj->display, (char*)obj->current);

	for (i = 0; i < MTRACKD_KEYMAP_SIZE; i++) {
		if ((obj->current[i] & ~obj->previous[i]) & obj->mask[i]) {
			res = True;
			break;
		}
	}

	if (res && !obj->modifiers) {
		for (i = 0; i < MTRACKD_KEYMAP_SIZE; i++) {
			if (obj->current[i] & ~obj->mask[i]) {
				res = False;
				break;
			}
		}
	}

	memcpy(obj->current, obj->previous,
		sizeof(unsigned char)*MTRACKD_KEYMAP_SIZE);
	return res;
}

Bool listen_init(Listen* obj, Display* display, Bool modifiers,
		int idle_time, int poll_time) {
	int i;
	KeyCode kc;
	XModifierKeymap* modmap;
	
	obj->modifiers = modifiers;
	obj->idle_time = ((double)idle_time)/1000.0;
	obj->poll_time = poll_time*1000;
	obj->display = display;

	for (i = 0; i < MTRACKD_KEYMAP_SIZE; i++)
		obj->mask[i] = 0xff;

	if (!modifiers) {
		modmap = XGetModifierMapping(obj->display);

		for (i = 0; i < 8 * modmap->max_keypermod; i++) {
			kc = modmap->modifiermap[i];
			if (kc != 0)
				clear_bit(obj->mask, kc);
		}

		XFreeModifiermap(modmap);
	}

	XQueryKeymap(obj->display, (char*)obj->current);
	memcpy(obj->previous, obj->current,
		sizeof(unsigned char)*MTRACKD_KEYMAP_SIZE);
	return True;
}

void listen_run(Listen* obj, Control* ctrl) {
	double current_time, last_activity = 0;
	while(True) {
		current_time = now();
		if (listen_activity(obj))
			last_activity = current_time;
		
		if (current_time > last_activity + obj->idle_time)
			control_toggle(ctrl, True);
		else
			control_toggle(ctrl, False);

		usleep(obj->poll_time);
	}
}

