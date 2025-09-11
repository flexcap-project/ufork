tinyalloc for Unikraft
======================

This is the port of the tinyalloc [0] memory allocator for Unikraft as an
external library.

tinyalloc is a minimalist allocator implementation, meant for use in systems
with unmanaged linear memory such as WebAssembly or embedded systems. tinyalloc
is highly configurable and offers high performance and reasonable memory usage
when used and configured appropriately.

How to use this allocator in your unikernel application:

- select "tinyalloc" in `ukboot > Default memory allocator`

[0] https://github.com/thi-ng/tinyalloc
