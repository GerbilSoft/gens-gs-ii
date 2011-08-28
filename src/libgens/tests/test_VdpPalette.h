/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette.h: VdpPalette tests.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_TEST_VDPPALETTE_H__
#define __LIBGENS_TEST_VDPPALETTE_H__

#include <stdint.h>

// Packed struct attribute.
 #if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

#ifdef __cplusplus
extern "C" {
#endif

#define PALTEST_MAGIC "PalTest"
#define PALTEST_VERSION 0x0001

// Color scaling methods.
#define PALTEST_CMD_COLORSCALE     "ColorScale"
#define PALTEST_COLORSCALE_RAW     "Raw"
#define PALTEST_COLORSCALE_FULL    "Full"
#define PALTEST_COLORSCALE_FULL_SH "Full+SH"

// Shadow/Highlight mode.
#define PALTEST_CMD_SHMODE       "SHMode"
#define PALTEST_SHMODE_NORMAL    "Normal"
#define PALTEST_SHMODE_SHADOW    "Shadow"
#define PALTEST_SHMODE_HIGHLIGHT "Highlight"

// Palette modes.
#define PALTEST_CMD_PALMODE "PalMode"
#define PALTEST_PALMODE_MD "MD"
#define PALTEST_PALMODE_SMS "SMS"
#define PALTEST_PALMODE_GG "GG"

// Color entry.
#define PALTEST_CMD_COLORENTRY "C"

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_TEST_VDPPALETTE_H__ */
