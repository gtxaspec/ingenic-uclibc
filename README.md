# uClibc Compatibility Shim for Ingenic Libraries

## Overview

This project provides a small compatibility shim that lets Ingenic SDK
binary libraries (`libimp`, `libalog`, `libsysutils`, ...) run on systems
built with a modern uClibc-ng toolchain.

Modern uClibc-ng exports nearly everything the blobs reference, so the shim
has shrunk to the two pieces uClibc-ng genuinely does not provide:

- **`mmap` offset workaround**: Ingenic libraries map rmem with byte
  offsets that are not always page aligned. uClibc-ng's `mmap` rejects
  those with `EINVAL`; the shim's `mmap` silently truncates to the
  containing page via the `mmap2` syscall, matching the vendor libc
  behaviour the blobs were built against. The blobs pass a 32-bit byte
  offset (pre-LFS `off_t` ABI).
- **Bare ctype table pointers** (`__ctype_b`, `__ctype_tolower`,
  `__ctype_toupper`): a glibc 2.2 era interface also exported by uClibc
  0.9.x but dropped by uClibc-ng. Still referenced by gcc 4.7.2 era blobs
  (for example T10/T20/T21/T30 `libalog`). The shim points them at
  uClibc-ng's own C locale tables, which use the same `1<<n` mask encoding
  those blobs were compiled with.

## Do not add libc wrappers back

Earlier versions also defined `fopen64`, `open64`, `fseeko64`, `mmap64`,
`__fgetc_unlocked`, `__fputc_unlocked`, `__assert`,
`__pthread_register_cancel` and `__pthread_unregister_cancel`. uClibc-ng
provides all of them natively, and redefining the LFS ones is fatal:
uClibc-ng force-enables `_FILE_OFFSET_BITS=64`, so its headers redirect
`fopen` to `fopen64`, `open` to `open64` and `fseeko` to `fseeko64` at
compile time (asm renames on the declarations, immune to `#undef`). A
wrapper like `fopen64() { return fopen(...); }` compiles into a call to
itself, and since the shim precedes libc in symbol lookup order, every
caller in the process hangs in an infinite tail-call loop on the first
file open.

For the same reason `uclibc_shim.c` must never include `<sys/mman.h>`:
the header would rename the shim's `mmap` definition to `mmap64`.

## Requirements

- A uClibc-ng toolchain with LFS (mandatory in current uClibc-ng) and NPTL
  threads. Both are checked at compile time.
- An Ingenic platform: xburst1 (T10 through T33, C100) or xburst2
  (T40/T41/A1).

## Building

Build as a shared library:

```sh
${CROSS_COMPILE}gcc -fPIC -shared -o libuclibcshim.so uclibc_shim.c
```

Or as a static archive:

```sh
${CROSS_COMPILE}gcc -c -o uclibc_shim.o uclibc_shim.c
${CROSS_COMPILE}gcc-ar rcs libuclibcshim.a uclibc_shim.o
```

## Usage

Link your application with `-luclibcshim` alongside the Ingenic SDK
libraries:

```makefile
LDFLAGS += -limp -lalog -lsysutils -luclibcshim
```

## Contributing

Contributions are welcome! Please feel free to submit issues and pull
requests through GitHub.

## License

This project is licensed under the MIT License.
