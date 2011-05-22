dispad
======

A small daemon for temporarily disabling trackpads on keyboard events. By
default dispad is configured to work with the xf86-input-mtrack multitouch
trackpad driver.

The source code for dispad is hosted at [Github][1].

License
-------

This software is licensed under the [GPLv2][2] and is copyright (C) 2011 Ryan
Bourgeois <bluedragonx@gmail.com>.

Binaries
--------

Binary packages will be made available at the first official release.

Configuration
-------------

dispad can be configured either at the command line or using a config file.
When started for the first time dispad will create a config file at ~/.dispad
containining the default configuration. The location of this config file can
changed with the --config commandline option. Configuration options given at
the commandline will override those in the config file. use the --help
commandline option to see all of the available commandline options.

The config file uses key = value pairs as its syntax. Strings must be double
quotes. The following config file options are accepted.

**property** -
The name of the XInput property which is used to disable/enable the
trackpad(s). dispad will modify this property on all trackpad devices which
contain it. String value. Defaults to "Trackpad Disable Input".

**enable** -
The specified XInput property is set to this value when enabling trackpad
input. Unsigned 8-bit integer value. Defaults to 0.

**disable** -
The specified XInput property is set to this value when disabling trackpad
input.  Unsigned 8-bit integer value. Defaults to 1.

**modifiers** -
Whether or not modifier keys (alt, ctrl, etc) should effect the trackpad state.
Boolean value. Defaults to false.

**poll** -
How long (in milliseconds) that dispad will wait after polling the keyboard
before polling again. Integer value. Default to 100.

**delay** -
How long after the trackpad(s) should be disabled after a keystroke.

**pidfile** -
The location of the PID file dispad will create when running. If this option is
commented or not present then a PID file will not be created. dispad will
remove this file if it shuts down cleanly. If dispad is configured to create a
PID file and that PID file already exists it will refuse to start. By default
this option is set to ~/.dispad.pid but is commented out.

[1]: https://github.com/BlueDragonX/dispad
[2]: http://www.gnu.org/licenses/gpl-2.0.html	"GNU General Public License, version 2"

