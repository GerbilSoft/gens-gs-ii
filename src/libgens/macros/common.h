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
 * FUNC_FORCE_INLINE: Force a function to be marked as inline.
 */
#if defined(NDEBUG)
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define FORCE_INLINE __attribute__ ((always_inline))
#else
#define FORCE_INLINE __inline__
#endif
#else /* !defined(NDEBUG) */
#define FORCE_INLINE
#endif /* defined(NDEBUG) */

/** Variable attributes. **/
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

/** Typedefs. **/

/**
 * utf8_str: UTF-8 C-string typedef.
 * Usage is similar to char.
 */
typedef char utf8_str;

/** Miscellaneous. **/

#ifdef _WIN32
#define LG_PATH_SEP_CHR '\\'
#define LG_PATH_SEP_STR "\\"
#else
#define LG_PATH_SEP_CHR '/'
#define LG_PATH_SEP_STR "/"
#endif

#endif /* __LIBGENS_MACROS_COMMON_H__ */
