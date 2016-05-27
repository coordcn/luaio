/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_CONFIG_H
#define LUAIO_CONFIG_H

/* @brief: operating system detection
 * @refer: v8/include/v8config.h
 *    LUAIO_ANDROID       - Android
 *    LUAIO_BSD           - BSDish (Mac OS X, Net/Free/Open/DragonFlyBSD)
 *    LUAIO_CYGWIN        - Cygwin
 *    LUAIO_DRAGONFLYBSD  - DragonFlyBSD
 *    LUAIO_FREEBSD       - FreeBSD
 *    LUAIO_LINUX         - Linux
 *    LUAIO_MACOSX        - Mac OS X
 *    LUAIO_NACL          - Native Client
 *    LUAIO_NETBSD        - NetBSD
 *    LUAIO_OPENBSD       - OpenBSD
 *    LUAIO_POSIX         - POSIX compatible (mostly everything except Windows)
 *    LUAIO_QNX           - QNX Neutrino
 *    LUAIO_SOLARIS       - Sun Solaris and OpenSolaris
 *    LUAIO_AIX           - AIX
 *    LUAIO_WINDOWS       - Microsoft Windows
 */
#if defined(__ANDROID__)
# define LUAIO_ANDROID 1
# define LUAIO_LINUX 1
# define LUAIO_POSIX 1
#elif defined(__APPLE__)
# define LUAIO_BSD 1
# define LUAIO_MACOSX 1
# define LUAIO_POSIX 1
#elif defined(__CYGWIN__)
# define LUAIO_CYGWIN 1
# define LUAIO_POSIX 1
#elif defined(__linux__)
# define LUAIO_LINUX 1
# define LUAIO_POSIX 1
#elif defined(__sun)
# define LUAIO_POSIX 1
# define LUAIO_SOLARIS 1
#elif defined(_AIX)
#define LUAIO_POSIX 1
#define LUAIO_AIX 1
#elif defined(__FreeBSD__)
# define LUAIO_BSD 1
# define LUAIO_FREEBSD 1
# define LUAIO_POSIX 1
#elif defined(__DragonFly__)
# define LUAIO_BSD 1
# define LUAIO_DRAGONFLYBSD 1
# define LUAIO_POSIX 1
#elif defined(__NetBSD__)
# define LUAIO_BSD 1
# define LUAIO_NETBSD 1
# define LUAIO_POSIX 1
#elif defined(__OpenBSD__)
# define LUAIO_BSD 1
# define LUAIO_OPENBSD 1
# define LUAIO_POSIX 1
#elif defined(__QNXNTO__)
# define LUAIO_POSIX 1
# define LUAIO_QNX 1
#elif defined(_WIN32)
# define LUAIO_WINDOWS 1
#endif

/* @brief: gnu/clang/intel compiler version test
 * @refer: v8/include/v8config.h
 */
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define LUAIO_GNUC_PREREQ(major, minor, patchlevel)                         \
    ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >=   \
     ((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define LUAIO_GNUC_PREREQ(major, minor, patchlevel)       \
    ((__GNUC__ * 10000 + __GNUC_MINOR__) >=             \
     ((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define LUAIO_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

/* @brief: compiler detection
 * @refer: v8/include/v8config.h
 * LUAIO_CC_GNU     - GCC, or clang in gcc mode
 * LUAIO_CC_INTEL   - Intel C++
 * LUAIO_CC_MINGW   - Minimalist GNU for Windows
 * LUAIO_CC_MINGW32 - Minimalist GNU for Windows (mingw32)
 * LUAIO_CC_MINGW64 - Minimalist GNU for Windows (mingw-w64)
 * LUAIO_CC_MSVC    - Microsoft Visual C/C++, or clang in cl.exe mode
 */
#if defined(__clang__)

#if defined(__GNUC__)  // Clang in gcc mode.
# define LUAIO_CC_GNU 1
#elif defined(_MSC_VER)  // Clang in cl mode.
# define LUAIO_CC_MSVC 1
#endif

# define LUAIO_HAS_BUILTIN_CLZ (__has_builtin(__builtin_clz))
# define LUAIO_HAS_BUILTIN_CTZ (__has_builtin(__builtin_ctz))
# define LUAIO_HAS_BUILTIN_POPCOUNT (__has_builtin(__builtin_popcount))
# define LUAIO_HAS_BUILTIN_EXPECT (__has_builtin(__builtin_expect))

#elif defined(__GNUC__)

# define LUAIO_CC_GNU 1
// Intel C++ also masquerades as GCC 3.2.0
# define LUAIO_CC_INTEL (defined(__INTEL_COMPILER))
# define LUAIO_CC_MINGW32 (defined(__MINGW32__))
# define LUAIO_CC_MINGW64 (defined(__MINGW64__))
# define LUAIO_CC_MINGW (LUAIO_CC_MINGW32 || LUAIO_CC_MINGW64)

# define LUAIO_HAS_BUILTIN_CLZ (LUAIO_GNUC_PREREQ(3, 4, 0))
# define LUAIO_HAS_BUILTIN_CTZ (LUAIO_GNUC_PREREQ(3, 4, 0))
# define LUAIO_HAS_BUILTIN_POPCOUNT (LUAIO_GNUC_PREREQ(3, 4, 0))
# define LUAIO_HAS_BUILTIN_EXPECT (LUAIO_GNUC_PREREQ(2, 96, 0))

#elif defined(_MSC_VER)

# define LUAIO_CC_MSVC 1

#endif

/* @brief: compile with branch prediction information
 * @refer: v8/include/v8config.h
 */
#if LUAIO_HAS_BUILTIN_EXPECT
# define LUAIO_UNLIKELY(condition) (__builtin_expect(!!(condition), 0))
# define LUAIO_LIKELY(condition) (__builtin_expect(!!(condition), 1))
#else
# define LUAIO_UNLIKELY(condition) (condition)
# define LUAIO_LIKELY(condition) (condition)
#endif

#ifdef LUAIO_LINUX
#include "luaio_linux_config.h"
#endif

#define LUAIO_VERSION         "0.1.0"

#define LUAIO_BITS            64
#define LUAIO_POINTER_SIZE    (sizeof(void*))

#ifndef offset_of
# define offset_of(type, member) ((intptr_t)((char*)(&(((type*)(0))->member))))
#endif

#ifndef container_of
# define container_of(ptr, type, member) ((type*)((char*)(ptr) - offset_of(type, member)))
#endif

#ifndef IS_POWER_OF_TWO
#define IS_POWER_OF_TWO(x) ((x) && (((x) & ((x) - 1)) == 0))
#endif

#define luaio_align(size, alignment) (((size) + (alignment - 1)) & ~(alignment - 1))

#define LUAIO_2_SHIFT             1
#define LUAIO_4_SHIFT             2
#define LUAIO_8_SHIFT             3
#define LUAIO_16_SHIFT            4
#define LUAIO_32_SHIFT            5
#define LUAIO_64_SHIFT            6
#define LUAIO_128_SHIFT           7
#define LUAIO_256_SHIFT           8
#define LUAIO_512_SHIFT           9
#define LUAIO_1K_SHIFT            10
#define LUAIO_2K_SHIFT            11
#define LUAIO_4K_SHIFT            12
#define LUAIO_8K_SHIFT            13
#define LUAIO_16K_SHIFT           14
#define LUAIO_32K_SHIFT           15
#define LUAIO_64K_SHIFT           16
#define LUAIO_128K_SHIFT          17
#define LUAIO_256K_SHIFT          18
#define LUAIO_512K_SHIFT          19
#define LUAIO_1M_SHIFT            20

#define luaio_malloc    malloc
#define luaio_realloc   realloc
#define luaio_free      free

/* Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */
#define HAVE_POSIX_MEMALIGN 1

/*#define HAVE_MEMALIGN 1*/

#if HAVE_POSIX_MEMALIGN

static inline void *luaio_memalign(size_t align, size_t size) {
  void *p;
  int err = posix_memalign(&p, align, size);
  return err ? NULL : p;
}

#elif HAVE_MEMALIGN

#define luaio_memalign(align, size) memalign(align, size) 

#else

#define luaio_memalign(align, size) luaio_malloc(size)

#endif

#define luaio_memset                memset
#define luaio_memcpy                memcpy
#define luaio_memmove               memmove
#define luaio_memcmp                memcmp
#define luaio_memchr                memchr
#define luaio_memzero(p, size)      memset(p, 0, size)

#define LUAIO_OPENSSL_NO_ENGINE     0
#define LUAIO_USE_PMEMORY           1
#define LUAIO_MAX_FREE_TIMERS       1024

#endif /* LUAIO_CONFIG_H */
