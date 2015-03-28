/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * zomg_md_tmss_reg.h: ZOMG save definitions for the MD TMSS registers.    *
 * MD TMSS registers: $A14000, $A14101                                     *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_MD_TMSS_REG_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_MD_TMSS_REG_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MD TMSS register save struct.
 * ZOMG file: MD/TMSS_reg.bin
 */
#define ZOMG_MD_TMSS_REG_HEADER	0x544D5353
#pragma pack(1)
typedef struct PACKED _Zomg_MD_TMSS_reg_t {
	uint32_t header;	// 32BE:  Should be "TMSS" (0x544D5353)
	uint32_t a14000;	// 32BE:  $A14000 - "SEGA" register
	uint8_t n_cart_ce;	// 8-bit: $A14101 - ROM mapping
} Zomg_MD_TMSS_reg_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_MD_TMSS_REG_H__ */
