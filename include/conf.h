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

#ifndef __MTRACKD_CONFIG__
#define __MTRACKD_CONFIG__

#include <X11/Xlib.h>
#include <stdint.h>

#define MTRACKD_DEFAULT_CONF ".dispad"
#define MTRACKD_DEFAULT_PROP "Trackpad Disable Input"
#define MTRACKD_DEFAULT_ENABLE 0
#define MTRACKD_DEFAULT_DISABLE 1
#define MTRACKD_DEFAULT_MODIFIERS False
#define MTRACKD_DEFAULT_POLL 100
#define MTRACKD_DEFAULT_DELAY 1000
#define MTRACKD_DEFAULT_PID_FILE NULL
#define MTRACKD_DEFAULT_FG False
#define MTRACKD_DEFAULT_DEBUG False

typedef struct {
	char* property;
	uint8_t enable;
	uint8_t disable;
	Bool modifiers;
	int poll;
	int delay;
	char* pid_file;
	Bool pid_file_created;
	Bool foreground;
	Bool debug;
} Config;

/* Initialize a Config object. Parses commandline and config file options.
 * Returns False if the application should not continue.
 */
Bool config_init(Config* obj, int argc, char** argv);

/* Create a PID file if it was configured to do so.
 */
int config_create_pid_file(Config* obj);

/* Remove a PID file if one was created.
 */
int config_remove_pid_file(Config* obj);

/* Free any memory associated with a Config object.
 */
void config_free(Config* obj);

#endif

