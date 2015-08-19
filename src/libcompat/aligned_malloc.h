/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * aligned_malloc.h: Aligned memory allocation compatibility header.       *
 *                                                                         *
 * Copyright (c) 2015 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

// References:
// - http://www.gnu.org/software/libc/manual/html_node/Aligned-Memory-Blocks.html
// - https://msdn.microsoft.com/en-us/library/8z34s9c6.aspx
// - http://linux.die.net/man/3/aligned_alloc (needs _ISOC11_SOURCE ?)

/**
 * TODO: Check if the following functions are present:
 * - _aligned_malloc (MSVC)
 * - aligned_alloc (C11)
 * - posix_memalign
 * - memalign
 * If none of these are present, we'll use our own custom
 * aligned malloc().
 */

#ifndef __LIBGENS_MACROS_ALIGNED_MALLOC_H__
#define __LIBGENS_MACROS_ALIGNED_MALLOC_H__

#include <libcompat/config.libcompat.h>

/**
 * This header defines two functions if they aren't present:
 * - aligned_malloc(): Allocate aligned memory.
 *   - Same signature as C11 aligned_alloc().
 *   - Returns NULL and sets errno on error.
 *   - NOTE: When using posix_memalign(), alignment must be a
 *     power of two multiple of sizeof(void*).
 * - aligned_free(): Free aligned memory.
 *   - Required for MSVC and custom implementations.
 */

// FORCE_INLINE macro from libgens, but without checking
// for ndebug, since these functions are simply wrappers.
#if defined(__GNUC__) && (__GNUC__ >= 4)
// FIXME: gcc complains that these functions
// might not be inlinable.
//#define FORCE_INLINE_MALLOC __attribute__ ((always_inline))
#define FORCE_INLINE_MALLOC __inline
#elif defined(_MSC_VER)
#define FORCE_INLINE_MALLOC __forceinline
#else
#define FORCE_INLINE_MALLOC __inline__
#endif

#include <errno.h>

// Alignment for statically-allocated data.
#if defined(_MSC_VER)
#define ALIGN(x) __declspec(align(x))
#elif defined(__GNUC__)
#define ALIGN(x) __attribute__ ((aligned (x)))
#else
#error Missing ALIGN() implementation for this compiler.
#endif

#if defined(HAVE_MSVC_ALIGNED_MALLOC)

// MSVC _aligned_malloc()
#include <malloc.h>

static FORCE_INLINE_MALLOC void *aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

static FORCE_INLINE_MALLOC void aligned_free(void *memptr)
{
	_aligned_free(memptr);
}

#elif defined(HAVE_ALIGNED_ALLOC)

// C11 aligned_alloc()
#include <stdlib.h>

static FORCE_INLINE_MALLOC void *aligned_malloc(size_t alignment, size_t size)
{
	return aligned_alloc(alignment, size);
}

static FORCE_INLINE_MALLOC void aligned_free(void *memptr)
{
	free(memptr);
}

#elif defined(HAVE_POSIX_MEMALIGN)

// posix_memalign()
#include <stdlib.h>

static FORCE_INLINE_MALLOC void *aligned_malloc(size_t alignment, size_t size)
{
	void *ptr;
	int ret = posix_memalign(&ptr, alignment, size);
	if (ret != 0) {
		// posix_memalign() returns errno instead of setting it.
		errno = ret;
		return NULL;
	}
	return ptr;
}

static FORCE_INLINE_MALLOC void aligned_free(void *memptr)
{
	free(memptr);
}

#elif defined(HAVE_MEMALIGN)

// memalign()
#include <stdlib.h>

static FORCE_INLINE_MALLOC void *aligned_malloc(size_t alignment, size_t size)
{
	return memalign(alignment, size);
}

static FORCE_INLINE_MALLOC void aligned_free(void *memptr)
{
	free(memptr);
}

#else
#error FIXME: System does not have an aligned malloc(), so a custom one needs to be implemented.
#endif

#endif /* __LIBGENS_MACROS_ALIGNED_MALLOC_H__ */
