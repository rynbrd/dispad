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

#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "common.h"
#include "config.h"
#include "control.h"
#include "listen.h"

int log_level = LOG_INFO;
Display* display = NULL;
Control* control = NULL;
Listen* listen = NULL;

static void cleanup() {
	if (control != NULL) {
		control_free(control);
		free(control);
		control = NULL;
	}
	if (listen != NULL) {
		free(listen);
		listen = NULL;
	}
	if (display != NULL) {
		XCloseDisplay(display);
		display = NULL;
	}
}

static void signal_handler(int signum) {
	cleanup();
	kill(getpid(), signum);
}

static void signal_installer() {
	int signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
		SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM, SIGTERM,
#ifdef SIGPWR
		SIGPWR
#endif
    };

	int i;
	struct sigaction action;
	sigset_t set;

	sigemptyset(&set);
	action.sa_handler = signal_handler;
	action.sa_mask = set;
#ifdef SA_ONESHOT
	action.sa_flags = SA_ONESHOT;
#else
	action.sa_flags = 0;
#endif

	for (i = 0; i < sizeof(signals) / sizeof(int); i++) {
		if (sigaction(signals[i], &action, NULL) == -1) {
			perror("sigaction");
			exit(2);
		}
	}
}

void background() {
	int pid = fork();
	if (pid > 0)
		exit(0);
	else if (pid == -1) {
		ERROR("failed to fork process, exitting\n");
		exit(2);
	}
}

int main(int argc, char** argv) {
	Config config;
	if (!config_init(&config, argc, argv)) {
		INFO("quitting at config\n");
		return 1;
	}

	if (config.foreground)
		log_level = config.debug ? LOG_DEBUG : LOG_INFO;
	else
		log_level = LOG_NONE;

	INFO("configured with:\n");
	INFO("  property = %s\n", config.property);
	INFO("  enable = %u\n", config.enable);
	INFO("  disable = %u\n", config.disable);
	INFO("  modifiers = %s\n", config.modifiers ? "true" : "false");
	INFO("  poll = %d\n", config.poll);
	INFO("  delay = %d\n", config.delay);

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		ERROR("failed to open display\n");
		return 1;
	}
	DEBUG("X display opened\n");

	control = malloc(sizeof(Control));
	if (!control_init(control, display, config.property, config.enable, config.disable)) {
		ERROR("failed to initialize control object\n");
		return 1;
	}
	DEBUG("control initialized\n");

	listen = malloc(sizeof(Listen));
	if (!listen_init(listen, display, config.modifiers, config.delay, config.poll)) {
		ERROR("failed to initialize listen object\n");
		cleanup();
		return 1;
	}
	DEBUG("listen initialized\n");

	config_free(&config);
	DEBUG("config object freed\n");

	signal_installer();
	DEBUG("signal handling enabled\n");

	if (!config.foreground)
		background();

	INFO("listener running\n");
	listen_run(listen, control);

	ERROR("listener failed to start, this should not happen\n");
	return 1;
}

