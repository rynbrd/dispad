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

#include "control.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONTROL_FIND_SLEEP 2

static Bool control_get_value(Control* obj, int device_index, unsigned char* value) {
	Atom type;
	int format;
	unsigned long size, bytes;
	unsigned char* data;

	if (XGetDeviceProperty(obj->display, obj->devices[device_index], obj->property, 0, 1,
			False, XA_INTEGER, &type, &format, &size, &bytes, &data) == Success &&
			type != None) {
		*value = data[0];
		free(data);
		return True;
	}
	return False;
}

static void control_set_value(Control* obj, int device_index, unsigned char value) {
	XChangeDeviceProperty(obj->display, obj->devices[device_index],
		obj->property, XA_INTEGER, 8, PropModeReplace, &value, 1);
}

static int control_load_devices(Control* obj) {
	int i;
	int ndev = 0;
	int nprops = 0;
	unsigned char value;
	Atom touchpad_type = XInternAtom(obj->display, XI_TOUCHPAD, True);
	Atom* properties = NULL;
	XDevice* dev = NULL;
	XDeviceInfo* info = XListInputDevices(obj->display, &ndev);

	obj->device_count = 0;
	DEBUG("searching %d devices for %s\n", ndev, obj->property_name);

	while (ndev--) {
		if (info[ndev].type == touchpad_type) {
			DEBUG("found touchpad device %s\n", info[ndev].name);
			dev = XOpenDevice(obj->display, info[ndev].id);
			if (!dev) {
				WARN("failed to open device %s\n", info[ndev].name);
				continue;
			}

			properties = XListDeviceProperties(obj->display, dev, &nprops);
			if (properties) {
				while (nprops--) {
					if (properties[nprops] == obj->property) {
						DEBUG("found property on device %s\n", info[ndev].name);
						obj->devices[obj->device_count++] = dev;
						break;
					}
				}
			}
			else {
				DEBUG("no properties on device %s\n", info[ndev].name);
			}
			XFree(properties);

			if (obj->device_count == 0 || obj->devices[obj->device_count-1] != dev)
				XCloseDevice(obj->display, dev);
		}
		else {
			DEBUG("not a trackpad: %s\n", info[ndev].name);
		}

		if (obj->device_count == MTRACKD_MAX_DEVICES)
			break;
	}

	for (i = 0; i < obj->device_count; i++) {
		if (control_get_value(obj, i, &value))
			obj->start_values[i] = value;
	}

	XFreeDeviceList(info);
	return obj->device_count;
}

void control_find_devices(Control* obj) {
	while (True) {
		if (control_load_devices(obj)) {
			DEBUG("found %d controllable devices\n", obj->device_count);
			control_toggle(obj, True);
			return;
		}
		DEBUG("no controllable devices found, sleeping for %d seconds\n", CONTROL_FIND_SLEEP);
		sleep(CONTROL_FIND_SLEEP);
	}
}

Bool control_init(Control* obj, Display* display, char* property_name,
		int enable_value, int disable_value) {
	obj->property_name = strdup(property_name);
	obj->enable_value = enable_value;
	obj->disable_value = disable_value;
	obj->display = display;
	obj->property = XInternAtom(obj->display, property_name, True);

	if (obj->property == 0) {
		ERROR("property not found: %s\n", obj->property_name);
		XCloseDisplay(obj->display);
		return False;
	}

	return True;
}

void control_free(Control* obj) {
	int i;
	for (i = 0; i < obj->device_count; i++) {
		control_set_value(obj, i, obj->start_values[i]);
		XCloseDevice(obj->display, obj->devices[i]);
	}
	free(obj->property_name);
}

void control_toggle(Control* obj, int enable) {
	int i;
	unsigned char current_value;
	unsigned char new_value = enable ? obj->enable_value : obj->disable_value;

	for (i = 0; i < obj->device_count; i++) {
		if (control_get_value(obj, i, &current_value)) {
			if (current_value != new_value) {
				DEBUG("setting state to %u for device at index %d\n", new_value, i);
				control_set_value(obj, i, new_value);
			}
		}
		else
			DEBUG("could not get current value of property %s\n", obj->property_name);
	}
}

