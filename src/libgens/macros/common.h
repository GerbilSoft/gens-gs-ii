/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * common.h: Common macros.                                                *
 *                                                                         *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef __LIBGENS_MACROS_COMMON_H__
#define __LIBGENS_MACROS_COMMON_H__

/** Function attributes. **/

/**
 * FUNC_PURE: Indicates that a function depends only on the function's parameters.
 */
#ifndef FUNC_PURE
#ifdef __GNUC__
#define FUNC_PURE __attribute__ ((pure))
#else
#define FUNC_PURE
#endif /* __GNUC__ */
#endif /* PURE */

/**
 * Force a function to be marked as inline.
 * FORCE_INLINE: Release builds only.
 * FORCE_INLINE_DEBUG: Debug and release builds.
 */
#if defined(__GNUC__) && (__GNUC__ >= 4)
// FIXME: gcc complains that these functions
// might not be inlinable.
//#define FORCE_INLINE_DEBUG __attribute__ ((always_inline))
#define FORCE_INLINE_DEBUG __inline
#elif defined(_MSC_VER)
#define FORCE_INLINE_DEBUG __forceinline
#else
#define FORCE_INLINE_DEBUG __inline__
#endif

#ifdef NDEBUG
#define FORCE_INLINE FORCE_INLINE_DEBUG
#else /* !NDEBUG */
#define FORCE_INLINE
#endif /* NDEBUG */

/** Variable attributes. **/
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

/** Typedefs. **/

/** Miscellaneous. **/

#ifdef _WIN32
#define LG_PATH_SEP_CHR '\\'
#define LG_PATH_SEP_STR "\\"
#else
#define LG_PATH_SEP_CHR '/'
#define LG_PATH_SEP_STR "/"
#endif

/**
 * Number of elements in an array.
 *
 * Includes a static check for pointers to make sure
 * a dynamically-allocated array wasn't specified.
 * Reference: http://stackoverflow.com/questions/8018843/macro-definition-array-size
 */
#define ARRAY_SIZE(x) \
	((int)(((sizeof(x) / sizeof(x[0]))) / \
	       (size_t)(!(sizeof(x) % sizeof(x[0])))))

#endif /* __LIBGENS_MACROS_COMMON_H__ */
