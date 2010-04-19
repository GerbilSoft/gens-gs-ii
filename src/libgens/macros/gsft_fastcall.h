/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_bool.h: FASTCALL macros.                                           *
 *                                                                         *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef __GSFT_FASTCALL_H
#define __GSFT_FASTCALL_H

#if !defined(__i386__)

/* FASTCALL only exists on i386. */
#define FASTCALL
#define __fastcall

#else /* defined(__i386__) */

/* i386 system. */

#if defined(_WIN32)
/* Win32 has __fastcall. */
#define FASTCALL	__fastcall
#else /* !defined(_WIN32) */

/* gcc-3.4 or later is required on non-Windows systems for __attribute__ ((fastcall)). */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#if (__GNUC__ < 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__) < 4)
#error This program uses __attribute__ ((fastcall)), which requires gcc-3.4 or later.
#endif
#endif

#define __fastcall	__attribute__ ((fastcall))
#define FASTCALL	__attribute__ ((fastcall))

#endif /* defined(_WIN32) */

#endif /* !defined(__i386__) */

#endif /* __GSFT_FASTCALL_H */
