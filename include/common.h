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

#ifndef __MTRACKD_GLOBAL__
#define __MTRACKD_GLOBAL__

#include <stdio.h>

#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2

extern int log_level;

#define INFO(...)  { if (log_level >= LOG_INFO)  { fprintf(stderr, "[I] "); fprintf(stderr, __VA_ARGS__); } }
#define WARN(...)  { if (log_level >= LOG_INFO)  { fprintf(stderr, "[W] "); fprintf(stderr, __VA_ARGS__); } }
#define ERROR(...) { if (log_level >= LOG_INFO)  { fprintf(stderr, "[E] "); fprintf(stderr, __VA_ARGS__); } }
#define DEBUG(...) { if (log_level >= LOG_DEBUG) { fprintf(stderr, "[D] "); fprintf(stderr, __VA_ARGS__); } }

#endif

