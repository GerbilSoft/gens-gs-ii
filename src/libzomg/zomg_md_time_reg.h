/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * zomg_md_time_reg.h: ZOMG save definitions for the MD /TIME region.      *
 * MD /TIME region: $A13000-$A130FF                                        *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_MD_TIME_REG_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_MD_TIME_REG_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MD /TIME region save struct.
 * ZOMG file: MD/TIME_reg.bin
 */
#pragma pack(1)
typedef struct PACKED _Zomg_MD_TimeReg_t
{
	union
	{
		uint8_t reg[256];			// All registers.
		struct
		{
			uint8_t reserved_00_EF[240];	// $A13000 - $A130EF: Reserved.
			uint8_t reserved_F0;		// $A130F0: Reserved.
			uint8_t SRAM_ctrl;		// $A130F1: SRAM control.
			uint8_t reserved_F2;		// $A130F2: Reserved.
			uint8_t SSF2_bank1;		// $A130F3: SSF2 Bank 1. ($080000 - $0FFFFF)
			uint8_t reserved_F4;		// $A130F4: Reserved.
			uint8_t SSF2_bank2;		// $A130F5: SSF2 Bank 2. ($100000 - $17FFFF)
			uint8_t reserved_F6;		// $A130F6: Reserved.
			uint8_t SSF2_bank3;		// $A130F7: SSF2 Bank 3. ($180000 - $1FFFFF)
			uint8_t reserved_F8;		// $A130F8: Reserved.
			uint8_t SSF2_bank4;		// $A130F9: SSF2 Bank 4. ($200000 - $27FFFF)
			uint8_t reserved_FA;		// $A130FA: Reserved.
			uint8_t SSF2_bank5;		// $A130FB: SSF2 Bank 5. ($280000 - $2FFFFF)
			uint8_t reserved_FC;		// $A130FC: Reserved.
			uint8_t SSF2_bank6;		// $A130FD: SSF2 Bank 6. ($300000 - $37FFFF)
			uint8_t reserved_FE;		// $A130FE: Reserved.
			uint8_t SSF2_bank7;		// $A130FF: SSF2 Bank 7. ($380000 - $3FFFFF)
		};
	};
} Zomg_MD_TimeReg_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_MD_TIME_REG_H__ */
