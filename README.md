# uClibc Compatibility Shim for Ingenic Libraries

## Overview

This project provides a compatibility shim to enable Ingenic libraries, such as `libimp`, to work seamlessly with the uClibc compiler. It serves to bridge compatibility issues and facilitate the development of applications on Ingenic platforms using uClibc as the standard C library.

## Features

- Compatibility layer for Ingenic libraries to work with uClibc
- glibc ctype table emulation (`__ctype_b_loc`, `__ctype_tolower_loc`, `__ctype_toupper_loc`)
- pthread cancel/unregister stubs
- mmap offset fixup for Ingenic memory mapping
- 64-bit file operation wrappers (`fopen64`, `open64`, `fseeko64`, `mmap64`)
- Supports both xburst1 (T10-T32) and xburst2 (T40/T41) platforms

## Getting Started

### Prerequisites

- An Ingenic-based platform
- A uClibc cross-compiler toolchain

### Building

Build as a shared library:

```sh
${CROSS_COMPILE}gcc -fPIC -shared -o libuclibcshim.so uclibc_shim.c
```

### Usage

Link your application with `-luclibcshim` alongside the Ingenic SDK libraries:

```makefile
LDFLAGS += -limp -lalog -lsysutils -luclibcshim
```

The shim provides missing symbols that the Ingenic SDK expects from glibc but are not present in uClibc.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests through GitHub.

## License

This project is licensed under the MIT License.
