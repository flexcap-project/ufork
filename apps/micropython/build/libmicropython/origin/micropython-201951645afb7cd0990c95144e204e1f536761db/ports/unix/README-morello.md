# Compiling MicroPython on Morello

MicroPython can be compiled using hybrid tools on a CheriBSD/Morello machine.
It's possible to compile both hybrid and purecap builds for MicroPython.

## Dependencies

On CheriBSD 22.12, the following packages need to be installed:

```
pkg64 install git gmake libffi llvm-morello pkgconf python3
```

For building purecap MicroPython, the purecap libffi is also required:

```
pkg64c install libffi
```

Note the requirement to use `gmake` rather than the default BSD `make`.
Also note that MicroPython build requires a working Python interpreter.

## Hybrid ABI

To compile MicroPython for the Hybrid ABI, execute the following commands in this folder:

```
gmake CC='clang-morello' CXX='clang++-morello' submodules
gmake CC='clang-morello' CXX='clang++-morello'
```

## Purecap ABI

To compile MicroPython for the purecap ABI, execute the following commands in this folder:

```
gmake CC='clang-morello -march=morello -mabi=purecap' CXX='clang++-morello -march=morello -mabi=purecap' LIBFFI_LDFLAGS='-L/usr/local/lib -lffi' submodules
gmake CC='clang-morello -march=morello -mabi=purecap' CXX='clang++-morello -march=morello -mabi=purecap' LIBFFI_LDFLAGS='-L/usr/local/lib -lffi'
```

The `LIBFFI_LDFLAGS` argument can be omitted if FFI has been turned off in `mpconfigport.mk` 

## Coverage variant

To compile the coverage variant, simply append `VARIANT=coverage` to the `gmake` command.
If you get an internal error in Clang, remove the two lines in
`variants/coverage/mpconfigvariant.h` which add the flags`-fprofile-arcs -ftest-coverage`.
