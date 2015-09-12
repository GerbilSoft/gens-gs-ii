#ifndef __C99_COMPAT_MSVCRT_H__
#define __C99_COMPAT_MSVCRT_H__

#ifndef _WIN32
#error c99-compat.msvcrt.h should only be included in Win32 builds.
#endif

#if defined(__GNUC__)

// MinGW-w64.
// TODO: Check standard MinGW?
#if !defined(__USE_MINGW_ANSI_STDIO)
// Using MSVCRT's snprintf().
#define ENABLE_C99_SNPRINTF_WRAPPERS 1
#define snprintf(str, size, format, ...)  C99_snprintf(str, size, format, __VA_ARGS__)
#define vsnprintf(str, size, format, ...) C99_vsnprintf(str, size, format, __VA_ARGS__)
#endif

#elif defined(_MSC_VER)

/**
 * MSVC 2015 added proper support for C99 snprintf().
 * Older versions have _snprintf(), which isn't fully compatible.
 */

#if _MSC_VER < 1400
/**
 * MSVC 2005 added support for variadic macros.
 * https://msdn.microsoft.com/en-US/library/ms177415(v=vs.80).aspx
 * TODO: Verify MSVC 2002 and 2003.
 */
#define ENABLE_C99_SNPRINTF_WRAPPERS 1
#define snprintf  C99_snprintf
#define vsnprintf C99_vsnprintf
#elif _MSC_VER < 1900
/** MSVC 2005-2013: variadic macros are supported, but still no snprintf(). */
#define ENABLE_C99_SNPRINTF_WRAPPERS 1
#define snprintf(str, size, format, ...)  C99_snprintf(str, size, format, __VA_ARGS__)
#define vsnprintf(str, size, format, ...) C99_vsnprintf(str, size, format, __VA_ARGS__)
#endif

#endif

#ifdef ENABLE_C99_SNPRINTF_WRAPPERS
#include <stdarg.h>
#include <stdio.h>

static
#if defined(_MSC_VER)
__forceinline
#elif defined(__GNUC__)
__inline
__attribute__ ((__format__ (gnu_printf, 3, 0)))
__attribute__ ((__nonnull__ (3)))
__attribute__ ((always_inline))
__attribute__ ((unused))
#endif
int C99_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int ret = _vsnprintf(str, size, format, ap);
	if (ret >= (int)size) {
		// Make sure the buffer is NULL-terminated.
		str[size-1] = 0;
	} else if (ret < 0) {
		// Make sure the buffer is empty.
		// MSVCRT *should* do this, but just in case...
		str[0] = 0;
	}
	return ret;
}

static
#if defined(_MSC_VER)
__forceinline
#elif defined(__GNUC__)
__inline
__attribute__ ((__format__ (gnu_printf, 3, 0)))
__attribute__ ((__nonnull__ (3)))
/* NOTE: gcc complains that this function cannot be inlined
 * becuase it uses variable argument lists.
__attribute__ ((always_inline))*/
__attribute__ ((unused))
#endif
int C99_snprintf(char *str, size_t size, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = C99_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}
#endif /* ENABLE_C99_SNPRINTF_WRAPPERS */

#endif /* __C99_COMPAT_MSVCRT_H__ */
