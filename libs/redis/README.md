# Redis for Unikraft
This is the port of Redis for Unikraft as external library.

## Build
Redis depends on the following libraries, that need to
be added to `Makefile` in this order:

* `pthreads`, e.g. `pthread-embedded`
* `libc`, e.g. `newlib`
* network stack, e.g. `lwip`

Before you proceed to writing your own application, you can use the `main()`
function provided in the Redis glue code by enabling it in its configuration
menu.

## Root filesystem
The steps for creating and using a filesystem are the same as the ones used for
[nginx](https://github.com/unikraft/lib-nginx/blob/staging/README.md). However,
it is not mandatory to use a configuration file because Redis has its own
default values.

## Running modes
We currently support the Redis server and CLI. You may find a `main()`
implementation example which shows how to call the main functions of server and
CLI.  If no custom parameters are passed, then the CLI will try to connect to
the local Redis server.  In order to do this you need to enable the loopback
interface (e.g. `Loopback interface` configuration option under `lwip` menu).

## Further information
Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository.
