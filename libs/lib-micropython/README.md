Micropython for Unikraft
=============================

This is a port of Micropython for Unikraft as external library. By
default, the port will go into interactive mode (though note that for
now there is a slight problem with it in that characters are echoed
twice). Alternatively, you can run a script from a file if passed as an
initrd parameter (e.g., in QEMU add -initrd "filename.py").

Note that for the build to work, in the menu you need to select:

lwip - Lightweight TCP/IP stack  ---> Operation mode (Mainloop (non-threaded))

Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository.
