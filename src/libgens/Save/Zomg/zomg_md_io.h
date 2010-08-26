/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * zomg_md_io.h: ZOMG save definitions for the MD I/O region.              *
 * MD I/O region: $A10001-$A1001F (odd bytes)                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_MD_IO_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_MD_IO_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// MD I/O save struct.
#pragma pack(1)
typedef struct PACKED _Zomg_MD_IoSave_t
{
	uint8_t version_reg;	// $A10001: Version register.
	uint8_t port1_data;	// $A10003: Control Port 1: Data.
	uint8_t port2_data;	// $A10005: Control Port 2: Data.
	uint8_t port3_data;	// $A10007: Control Port 3: Data.
	uint8_t port1_ctrl;	// $A10009: Control Port 1: Ctrl.
	uint8_t port2_ctrl;	// $A1000B: Control Port 2: Ctrl.
	uint8_t port3_ctrl;	// $A1000D: Control Port 3: Ctrl.
	uint8_t port1_ser_tx;	// $A1000F: Control Port 1: Serial TxData.
	uint8_t port1_ser_rx;	// $A10011: Control Port 1: Serial RxData.
	uint8_t port1_ser_ctrl;	// $A10013: Control Port 1: Serial Control.
	uint8_t port2_ser_tx;	// $A10015: Control Port 2: Serial TxData.
	uint8_t port2_ser_rx;	// $A10017: Control Port 2: Serial RxData.
	uint8_t port2_ser_ctrl;	// $A10019: Control Port 2: Serial Control.
	uint8_t port3_ser_tx;	// $A1001B: Control Port 3: Serial TxData.
	uint8_t port3_ser_rx;	// $A1001D: Control Port 3: Serial RxData.
	uint8_t port3_ser_ctrl;	// $A1001F: Control Port 3: Serial Control.
} Zomg_MD_IoSave_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_PSG_H__ */
