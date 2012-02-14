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

#include "conf.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <confuse.h>

static void usage() {
	fprintf(stderr, "Usage: dispad [-hmFD] [-c file] [-p name] [-e value] [-d value]\n");
	fprintf(stderr, "            [-s time] [-i time] [-P file]\n");
}

static void help() {
	usage();
	fprintf(stderr, "\nDaemon for disabling trackpad input while typing. Options given at the\n");
	fprintf(stderr, "command-line override those in the config file.\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -c, --config=FILE         Load the specified config file. Will load\n");
	fprintf(stderr, "                            ~/.dispad by default.\n");
	fprintf(stderr, "  -p, --property=NAME       The name of the property used to enable/disable the\n");
	fprintf(stderr, "                            trackpad.\n");
	fprintf(stderr, "  -e, --enable=VALUE        The value used to enable the trackpad. Must fit in\n");
	fprintf(stderr, "                            an 8-bit unsigned integer.\n");
	fprintf(stderr, "  -d, --disable=VALUE       The value used to disable the trackpad. Must fit in\n");
	fprintf(stderr, "                            an 8-bit unsigned integer.\n");
	fprintf(stderr, "  -m, --modifiers           Also disable the trackpads when modifier keys are\n");
	fprintf(stderr, "                            pressed.\n");
	fprintf(stderr, "  -s, --poll=MS             How long (in ms) to sleep between keyboard polls.\n");
	fprintf(stderr, "  -i, --delay=MS            How long (in ms) to disable the trackpad after a\n");
	fprintf(stderr, "                            keystroke.\n");
	fprintf(stderr, "  -P, --pidfile=FILE        Create a pid file at the given location. Only\n");
	fprintf(stderr, "                            useful when daemonizing.\n");
	fprintf(stderr, "  -F, --foreground          Start in the foreground. We daemonize by default.\n");
	fprintf(stderr, "  -D, --debug               Enable debug output. Only useful when combined with\n");
	fprintf(stderr, "                            -F.\n");
	fprintf(stderr, "  -h, --help                Display this help.\n");
}

static Bool file_exists(char* file) {
	struct stat buf;
	if (stat(file, &buf) == 0)
		return True;
	return False;
}

static char* config_file_default() {
	char* home = getenv("HOME");
	char* file = malloc(strlen(home) + strlen(MTRACKD_DEFAULT_CONF) + 2);
	strcpy(file, "");
	strcat(file, home);
	strcat(file, "/");
	strcat(file, MTRACKD_DEFAULT_CONF);
	return file;
}

static Bool config_file_create(char* file) {
	FILE* fd = fopen(file, "w");
	if (fd == NULL)
		return False;

	fprintf(fd, "# default dispad config file\n\n");
	fprintf(fd, "# name of the property used to enable/disable the trackpad\n");
	fprintf(fd, "property = \"%s\"\n\n", MTRACKD_DEFAULT_PROP);
	fprintf(fd, "# the value used to enable the trackpad\n");
	fprintf(fd, "enable = %d\n\n", MTRACKD_DEFAULT_ENABLE);
	fprintf(fd, "# the value used to disable the trackpad\n");
	fprintf(fd, "disable = %d\n\n", MTRACKD_DEFAULT_DISABLE);
	fprintf(fd, "# whether or not modifier keys disable the trackpad\n");
	fprintf(fd, "modifiers = %s\n\n", MTRACKD_DEFAULT_MODIFIERS ? "true" : "false");
	fprintf(fd, "# how long (in ms) to sleep between keyboard polls\n");
	fprintf(fd, "poll = %d\n\n", MTRACKD_DEFAULT_POLL);
	fprintf(fd, "# how long (in ms) to disable the trackpad after a keystroke\n");
	fprintf(fd, "delay = %d\n\n", MTRACKD_DEFAULT_DELAY);
	fprintf(fd, "# create a pid file at the given location; not created if left commented\n");
	fprintf(fd, "#pidfile = \"%s/.dispad.pid\"\n", getenv("HOME"));
	fclose(fd);
	return True;
}

static Bool config_file_parse(Config* obj, char* file) {
	cfg_bool_t modifiers = obj->modifiers ? cfg_true : cfg_false;
	cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("property", &obj->property),
		CFG_SIMPLE_INT("enable", &obj->enable),
		CFG_SIMPLE_INT("disable", &obj->disable),
		CFG_SIMPLE_BOOL("modifiers", &modifiers),
		CFG_SIMPLE_INT("poll", &obj->poll),
		CFG_SIMPLE_INT("delay", &obj->delay),
		CFG_SIMPLE_STR("pidfile", &obj->pid_file),
		CFG_END()
	};
	cfg_t* cfg = cfg_init(opts, 0);
	int res = cfg_parse(cfg, file);
	cfg_free(cfg);
	if (res == CFG_SUCCESS) {
		return True;
	}
	else if (res == CFG_FILE_ERROR) {
		ERROR("could not read config file\n");
	}
	else {
		ERROR("could not parse config file\n");
	}
	return False;
}

Bool config_init(Config* obj, int argc, char** argv) {
	int c;
	Bool res = True;
	char* file = NULL;
	char* opts = "c:p:e:d:ms:i:P:FDh";
	struct option lopts[] = {
		{"config", 1, 0, 'c'},
		{"property", 1, 0, 'p'},
		{"enable", 1, 0, 'e'},
		{"disable", 1, 0, 'd'},
		{"modifiers", 0, 0, 'm'},
		{"poll", 1, 0, 's'},
		{"delay", 1, 0, 'i'},
		{"pidfile", 1, 0, 'P'},
		{"foreground", 0, 0, 'F'},
		{"debug", 0, 0, 'D'},
		{0, 0, 0, 0,}
	};

	Config tmp;
	Bool has_prop = False;
	Bool has_enable = False;
	Bool has_disable = False;
	Bool has_modifiers = False;
	Bool has_poll = False;
	Bool has_delay = False;
	Bool has_pid_file = False;
	Bool has_fg = False;
	Bool has_debug = False;

	tmp.property = NULL;
	tmp.pid_file = NULL;
	obj->pid_file_created = False;
	obj->property = NULL;
	obj->enable = MTRACKD_DEFAULT_ENABLE;
	obj->disable = MTRACKD_DEFAULT_DISABLE;
	obj->modifiers = MTRACKD_DEFAULT_MODIFIERS;
	obj->poll = MTRACKD_DEFAULT_POLL;
	obj->delay = MTRACKD_DEFAULT_DELAY;
	obj->pid_file = NULL;
	obj->foreground = MTRACKD_DEFAULT_FG;
	obj->debug = MTRACKD_DEFAULT_DEBUG;

	while ((c = getopt_long(argc, argv, opts, lopts, NULL)) != -1) {
		switch (c) {
		case 'c':
			file = strdup(optarg);
			break;
		case 'p':
			if (strlen(optarg) > 0) {
				tmp.property = strdup(optarg);
				has_prop = True;
			}
			else {
				ERROR("property name is empty\n");
				res = False;
				goto cleanup;
			}
			break;
		case 'e':
			tmp.enable = atoi(optarg);
			has_enable = True;
			break;
		case 'd':
			tmp.disable = atoi(optarg);
			has_disable = True;
			break;
		case 'm':
			tmp.modifiers = True;
			has_modifiers = True;
			break;
		case 's':
			tmp.poll = atoi(optarg);
			if (tmp.poll <= 0) {
				ERROR("invalid poll value: %s\n", optarg);
				res = False;
				goto cleanup;
			}
			has_poll = True;
			break;
		case 'i':
			tmp.delay = atoi(optarg);
			if (tmp.delay <= 0) {
				ERROR("invalid poll value: %s\n", optarg);
				res = False;
				goto cleanup;
			}
			has_delay = True;
			break;
		case 'P':
			if (strlen(optarg) > 0) {
				tmp.pid_file = strdup(optarg);
				has_pid_file = True;
			}
			else {
				ERROR("pid file is empty\n");
				res = False;
				goto cleanup;
			}
			break;
		case 'F':
			tmp.foreground = True;
			has_fg = True;
			break;
		case 'D':
			tmp.debug = True;
			has_debug = True;
			break;
		case 'h':
			help();
			res = False;
			goto cleanup;
		case '?':
		case ':':
			ERROR("Unrecognized options. Use -h to see what you did wrong.\n");
			usage();
			res = False;
			goto cleanup;
		}
	}

	if (file == NULL) {
		file = config_file_default();
		if (!file_exists(file)) {
			if (!config_file_create(file)) {
				ERROR("failed to create default config file: %s\n", file);
				res = False;
				goto cleanup;
			}
		}
	}
	
	INFO("using config file: %s\n", file);
	if (!config_file_parse(obj, file)) {
		ERROR("failed to parse config file: %s\n", file);
		res = False;
		goto cleanup;
	}

	if (has_prop) {
		if (obj->property != NULL)
			free(obj->property);
		obj->property = strdup(tmp.property);
	}
	else if (obj->property == NULL && MTRACKD_DEFAULT_PROP != NULL)
		obj->property = strdup(MTRACKD_DEFAULT_PROP);

	if (has_pid_file) {
		if (obj->pid_file != NULL)
			free(obj->pid_file);
		obj->pid_file = strdup(tmp.pid_file);
	}
	else if (obj->pid_file == NULL && MTRACKD_DEFAULT_PID_FILE != NULL)
		obj->pid_file = MTRACKD_DEFAULT_PID_FILE;

	if (has_enable)
		obj->enable = tmp.enable;
	if (has_disable)
		obj->disable = tmp.disable;
	if (has_modifiers)
		obj->modifiers = tmp.modifiers;
	if (has_poll)
		obj->poll = tmp.poll;
	if (has_delay)
		obj->delay = tmp.delay;
	if (has_fg)
		obj->foreground = tmp.foreground;
	if (has_debug)
		obj->debug = tmp.debug;

cleanup:
	if (file != NULL)
		free(file);
	if (tmp.property != NULL)
		free(tmp.property);
	return res;
}

int config_create_pid_file(Config* obj) {
	if (obj->pid_file == NULL)
		return True;
	if (file_exists(obj->pid_file)) {
		ERROR("pid file exists: %s\n", obj->pid_file);
		return False;
	}

	FILE* f = fopen(obj->pid_file, "w");
	if (f == NULL) {
		ERROR("could not write to pid file: %s\n", obj->pid_file)
		return False;
	}

	fprintf(f, "%d", getpid());
	fclose(f);
	obj->pid_file_created = True;
	return True;
}

int config_remove_pid_file(Config* obj) {
	if (obj->pid_file == NULL || !obj->pid_file_created)
		return True;
	if (!file_exists(obj->pid_file)) {
		WARN("pid file does not exist: %s\n", obj->pid_file);
		return True;
	}

	if (unlink(obj->pid_file) != 0) {
		ERROR("could not delete pid file: %s\n", obj->pid_file);
		return False;
	}
	return True;
}

void config_free(Config* obj) {
	if (obj->property != NULL)
		free(obj->property);
	if (obj->pid_file != NULL)
		free(obj->pid_file);
}

