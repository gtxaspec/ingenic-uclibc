/*
 * Compatibility shim for Ingenic SDK blobs (libimp, libalog, libsysutils, ...)
 * on uClibc-ng based systems.
 *
 * The blobs were built against Ingenic's own uClibc / uClibc-ng toolchains.
 * Modern uClibc-ng already exports nearly everything they reference,
 * including symbols this shim historically provided: fopen64, open64,
 * fseeko64, mmap64, __fgetc_unlocked, __fputc_unlocked, __assert,
 * __pthread_register_cancel, __pthread_unregister_cancel and the
 * __ctype_*_loc functions.  They come straight from libc now and MUST NOT
 * be redefined here:
 *
 *   uClibc-ng force-enables _FILE_OFFSET_BITS=64, so its headers redirect
 *   fopen to fopen64, open to open64, fseeko to fseeko64 and mmap to mmap64
 *   at compile time (__REDIRECT asm renames on the declarations, immune to
 *   #undef).  A wrapper like "fopen64() { return fopen(...); }" therefore
 *   compiles into a call to itself, and because the shim precedes libc in
 *   the symbol lookup order, every caller in the process hangs in a
 *   tail-call loop.  The old 64-bit wrappers also read the wrong argument
 *   slots for the o32 off64_t register pair, so handing these symbols back
 *   to libc fixes latent bugs as well.
 *
 * What legitimately remains:
 *
 * 1. mmap: Ingenic libraries map rmem with byte offsets that are not
 *    always page aligned.  uClibc-ng's mmap rejects those with EINVAL;
 *    this version silently truncates to the containing page via the mmap2
 *    syscall, matching the vendor libc behaviour the blobs rely on.
 *    The blobs pass a 32-bit byte offset (pre-LFS off_t ABI), hence the
 *    uint32_t parameter.
 *
 * 2. __ctype_b / __ctype_tolower / __ctype_toupper: bare table pointers
 *    (glibc 2.2 era interface, also exported by uClibc 0.9.x) referenced
 *    by gcc 4.7.2 era blobs such as T10/T20/T21/T30 libalog.  uClibc-ng
 *    dropped them; point them at uClibc-ng's own C locale tables, which
 *    use the same 1<<n mask encoding those blobs were compiled with.
 */

#include <features.h>

#ifndef __UCLIBC__
# error "This shim must be built against uClibc-ng"
#endif
#ifndef __UCLIBC_HAS_LFS__
# error "uClibc-ng without LFS: blobs need fopen64/open64/fseeko64/mmap64 from libc"
#endif
#ifndef __UCLIBC_HAS_THREADS_NATIVE__
# error "uClibc-ng without NPTL: blobs need __pthread_register_cancel from libc"
#endif

#include <ctype.h>       /* __ctype_mask_t, __ctype_touplow_t, __ctype_*_loc */
#include <stdint.h>      /* uint32_t */
#include <unistd.h>      /* size_t, syscall */
#include <sys/syscall.h> /* SYS_mmap2 */

/*
 * Deliberately NOT including <sys/mman.h>: with mandatory
 * _FILE_OFFSET_BITS=64 it would rename this definition to mmap64.
 */
void *mmap(void *start, size_t len, int prot, int flags, int fd, uint32_t off)
{
	return (void *)syscall(SYS_mmap2, start, len, prot, flags, fd, off >> 12);
}

/* glibc 2.2 style bare pointers, aliased to libc's C locale tables. */
const __ctype_mask_t *__ctype_b;
const __ctype_touplow_t *__ctype_tolower;
const __ctype_touplow_t *__ctype_toupper;

static void __attribute__((constructor)) init_ctype_compat(void)
{
	__ctype_b = *__ctype_b_loc();
	__ctype_tolower = *__ctype_tolower_loc();
	__ctype_toupper = *__ctype_toupper_loc();
}
