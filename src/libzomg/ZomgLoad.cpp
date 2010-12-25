/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgLoad.cpp: Savestate handler. (Loading functions)                    *
 *                                                                         *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#include "Zomg.hpp"
#include "zomg_byteswap.h"

// C includes.
#include <stdint.h>
#include <string.h>

namespace LibZomg
{

/**
 * loadFromZomg(): Load a file from the ZOMG file.
 * @param filename Filename to load from the ZOMG file.
 * @param buf Buffer to store the file in.
 * @param len Length of the buffer.
 * @return Length of file loaded, or negative number on error.
 */
int Zomg::loadFromZomg(const utf8_str *filename, void *buf, int len)
{
	if (m_mode != ZOMG_LOAD || !m_unz)
		return -1;
	
	// Locate the file in the ZOMG file.
	int ret = unzLocateFile(m_unz, filename, 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return -2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_unz);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return -3;
	}
	
	// Read the file.
	ret = unzReadCurrentFile(m_unz, buf, len);
	unzCloseCurrentFile(m_unz);	// TODO: Check the return value!
	
	// Return the number of bytes read.
	return ret;
}


/**
 * Load savestate functions.
 * @param siz Number of bytes to read.
 * @return Bytes read on success; negative on error.
 * TODO: Standardize error codes.
 */

// TODO: Determine siz and is16bit from the system type?
// (once FORMAT.ini is implemented)


/**
 * loadPreview(): Load the preview image.
 * @param img_buf Image buffer.
 * @param siz Size of the image buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadPreview(void *img_buf, size_t siz)
{
	static const uint8_t png_magic[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	
	// Make sure a preview image is available to load.
	if (m_preview_size == 0 || siz < sizeof(png_magic) || siz < m_preview_size)
		return -4;
	
	// Load the preview image.
	int ret = loadFromZomg("preview.png", img_buf, siz);
	if (ret < 0)
		return ret;
	
	// Verify the PNG "magic number".
	if (memcmp(img_buf, png_magic, sizeof(png_magic)) != 0)
	{
		// Invalid "magic number".
		// Clear the image buffer.
		memset(img_buf, 0x00, siz);
		return -5;
	}
	
	// Preview image loaded.
	return 0;
}


/** VDP **/


/**
 * loadVdpReg(): Load VDP registers.
 * File: common/vdp_reg.bin
 * @param reg Destination buffer for VDP registers.
 * @param siz Number of VDP registers to load.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpReg(uint8_t *reg, size_t siz)
{
	return loadFromZomg("common/vdp_reg.bin", reg, siz);
}


/**
 * loadVRam(): Load VRam.
 * File: common/VRam.bin
 * @param vram Destination buffer for VRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap to host-endian 16-bit.
 * @return Number of bytes read on success; negative on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::loadVRam(void *vram, size_t siz, bool byteswap)
{
	int ret = loadFromZomg("common/VRam.bin", vram, siz);
	if (byteswap)
	{
		// TODO: Only byteswap for MD.
		be16_to_cpu_array(vram, siz);
	}
	return ret;
}


/**
 * loadCRam(): Load CRam.
 * File: common/CRam.bin
 * @param vram Destination buffer for CRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap to host-endian 16-bit.
 * @return Number of bytes read on success; negative on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::loadCRam(void *cram, size_t siz, bool byteswap)
{
	int ret = loadFromZomg("common/CRam.bin", cram, siz);
	if (byteswap)
	{
		// TODO: Only byteswap for MD.
		be16_to_cpu_array(cram, siz);
	}
	return ret;
}


/**
 * loadMD_VSRam(): Load VSRam. (MD-specific)
 * File: MD/VSRam.bin
 * @param vsram Destination buffer for VSRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap to host-endian 16-bit.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_VSRam(uint16_t *vsram, size_t siz, bool byteswap)
{
	int ret = loadFromZomg("MD/VSRam.bin", vsram, siz);
	if (byteswap)
		be16_to_cpu_array(vsram, siz);
	return ret;
}


/** Audio **/


/**
 * loadPsgReg(): Load PSG registers.
 * File: common/psg.bin
 * 16-bit fields are always byteswapped to host-endian.
 * @param state PSG register buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadPsgReg(Zomg_PsgSave_t *state)
{
	int ret = loadFromZomg("common/psg.bin", state, sizeof(*state));
	
	// Byteswap the 16-bit fields.
	for (unsigned int i = 0; i < 4; i++)
	{
		state->tone_reg[i] = le16_to_cpu(state->tone_reg[i]);
		state->tone_ctr[i] = le16_to_cpu(state->tone_ctr[i]);
	}
	state->lfsr_state = le16_to_cpu(state->lfsr_state);
	
	return ret;
}


/**
 * loadMD_YM2612_reg(): Load YM2612 registers. (MD-specific)
 * File: MD/YM2612_reg.bin
 * @param state YM2612 register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_YM2612_reg(Zomg_Ym2612Save_t *state)
{
	return loadFromZomg("MD/YM2612_reg.bin", state, sizeof(*state));
}


/** Z80 **/


/**
 * loadZ80Mem(): Load Z80 memory.
 * File: common/Z80_mem.bin
 * @param mem Z80 memory buffer.
 * @param siz Size of the Z80 memory buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadZ80Mem(uint8_t *mem, size_t siz)
{
	return loadFromZomg("common/Z80_mem.bin", mem, siz);
}


/**
 * loadZ80Reg(): Load Z80 registers.
 * File: common/Z80_reg.bin
 * 16-bit fields are always byteswapped to host-endian.
 * @param state Z80 register buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadZ80Reg(Zomg_Z80RegSave_t *state)
{
	int ret = loadFromZomg("common/Z80_reg.bin", state, sizeof(*state));
	
	// Byteswap the 16-bit fields.
	
	// Main register set.
	state->AF = le16_to_cpu(state->AF);
	state->BC = le16_to_cpu(state->BC);
	state->DE = le16_to_cpu(state->DE);
	state->HL = le16_to_cpu(state->HL);
	state->IX = le16_to_cpu(state->IX);
	state->IY = le16_to_cpu(state->IY);
	state->PC = le16_to_cpu(state->PC);
	state->SP = le16_to_cpu(state->SP);
	
	// Shadow register set.
	state->AF2 = le16_to_cpu(state->AF2);
	state->BC2 = le16_to_cpu(state->BC2);
	state->DE2 = le16_to_cpu(state->DE2);
	state->HL2 = le16_to_cpu(state->HL2);
	
	return ret;
}


/** M68K (MD-specific) **/


/**
 * loadM68KMem(): Load M68K memory. (MD-specific)
 * File: MD/M68K_mem.bin
 * @param mem M68K memory buffer.
 * @param siz Size of the M68K memory buffer.
 * @param byteswap If true, byteswap to host-endian 16-bit.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadM68KMem(uint16_t *mem, size_t siz, bool byteswap)
{
	int ret = loadFromZomg("MD/M68K_mem.bin", mem, siz);
	if (byteswap)
		be16_to_cpu_array(mem, siz);
	return ret;
}


/**
 * loadM68KReg(): Load M68K registers. (MD-specific)
 * File: MD/M68K_reg.bin
 * 16-bit and 32-bit fields are always byteswapped to host-endian.
 * @param state M68K register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadM68KReg(Zomg_M68KRegSave_t *state)
{
	int ret = loadFromZomg("MD/M68K_reg.bin", state, sizeof(*state));
	
	// Byteswap the 16-bit and 32-bit fields.
	for (unsigned int i = 0; i < 8; i++)
	{
		state->areg[i] = be32_to_cpu(state->areg[i]);
		state->dreg[i] = be32_to_cpu(state->dreg[i]);
	}
	state->asp = be32_to_cpu(state->asp);
	state->pc  = be32_to_cpu(state->pc);
	state->sr  = be16_to_cpu(state->sr);
	
	return ret;
}


/** MD-specific registers. **/


/**
 * loadMD_IO(): Load MD I/O port registers. (MD-specific)
 * @param state MD I/O port register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_IO(Zomg_MD_IoSave_t *state)
{
	return loadFromZomg("MD/IO.bin", state, sizeof(*state));
}


/**
 * loadMD_Z80Ctrl(): Load MD Z80 control registers. (MD-specific)
 * 16-bit fields are always byteswapped to host-endian.
 * @param state MD Z80 control register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state)
{
	int ret = loadFromZomg("MD/Z80_ctrl.bin", state, sizeof(*state));
	
	// Byteswap the 16-bit fields.
	state->m68k_bank = be16_to_cpu(state->m68k_bank);
	
	return ret;
}


/**
 * loadMD_TimeReg(): Load MD /TIME registers. (MD-specific)
 * @param state MD /TIME register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_TimeReg(Zomg_MD_TimeReg_t *state)
{
	memset(state, 0xFF, sizeof(state));
	int ret = loadFromZomg("MD/TIME_reg.bin", state, sizeof(*state));
	
	if (ret <= 0xF1)
	{
		// SRAM control register wasn't loaded.
		// Set it to 2 by default:
		// - ROM accessible
		// - SRAM write-protected
		state->SRAM_ctrl = 2;
	}
	
	return ret;
}


/** Miscellaneous **/


/**
 * loadSRam(): Load SRAM.
 * @param sram Pointer to SRAM buffer.
 * @param siz Size of SRAM buffer.
 * @return Number of bytes read on success; negative on error.
 * NOTE: If the loaded SRAM file is smaller than the specified SRAM buffer,
 * the remainder of the SRAM buffer is initialized to 0xFF.
 */
int Zomg::loadSRam(uint8_t *sram, size_t siz)
{
	int ret = loadFromZomg("common/SRam.bin", sram, siz);
	if (ret > 0)
	{
		// Data was loaded.
		// If the data is less than the size of the SRAM buffer,
		// set the rest of the SRAM buffer to 0xFF.
		if (ret < (int)siz)
		{
			int diff = ((int)siz - ret);
			memset(&sram[ret], 0xFF, diff);
		}
	}
	
	return ret;
}

}
