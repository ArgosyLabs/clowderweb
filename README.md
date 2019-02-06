clowderweb
==========

clowderweb is a modular web application server written in C++

Requires
	* [civetweb](https://github.com/civetweb/civetweb).
	* [libace](https://github.com/pallas/libace).

Individual modules are compiled as shared libraries, linked against
libclowderweb, and loaded at a specific prefix when starting the clowderweb
application.  All civetweb config options are supported as command line
arguments, but currently duplicate options are not combined in any way as
they would be in civetweb.

The example system_info module is also provided and can be mounted at / via

	clowderweb /usr/local/lib/clowderweb/system_info.so

Multiple modules can be specified on the command line but should be mounted
at different path prefixes.  Failure of any module to load does *not*
prevent the server from starting.

To use HTTPS, IPv6, and mount at /system_info, try

	clowderweb
		--listening_ports=+8000s
		--ssl_certificate=/path/to/server.pem
		/usr/local/lib/clowderweb/system_info.so=/system_info

At the moment, the server will fork/exec a process with identical arguments
every 10 minutes and then exit.  This is currently not configurable, but
patches are welcome.

The API and command line arguments should be considered unstable at this
time.

Happy hacking.
