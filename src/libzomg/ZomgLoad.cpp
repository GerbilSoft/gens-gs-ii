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
 * loadVdpCtrl_8(): Load VDP control registers. (8-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Destination buffer for VDP control registers.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpCtrl_8(Zomg_VdpCtrl_8_t *ctrl)
{
	int ret = loadFromZomg("common/vdp_ctrl.bin", ctrl, sizeof(*ctrl));
	
	// Verify that the control registers are valid.
	if (ret != (int)(sizeof(*ctrl)))
		return -1;
	
	// Verify the header.
	ctrl->header = be32_to_cpu(ctrl->header);
	if (ctrl->header != ZOMG_VDPCTRL_8_HEADER)
		return -2;
	
	// Byteswap the fields.
	ctrl->address = be16_to_cpu(ctrl->address);
	
	// Clear the reserved fields.
	ctrl->reserved1 = 0;
	ctrl->reserved2 = 0;
	
	// Return the number of bytes read.
	return ret;
}


/**
 * loadVdpCtrl_16(): Load VDP control registers. (16-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Destination buffer for VDP control registers.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpCtrl_16(Zomg_VdpCtrl_16_t *ctrl)
{
	int ret = loadFromZomg("common/vdp_ctrl.bin", ctrl, sizeof(*ctrl));
	
	// Verify that the control registers are valid.
	if (ret != (int)(sizeof(*ctrl)))
		return -1;
	
	// Verify the header.
	ctrl->header = be32_to_cpu(ctrl->header);
	if (ctrl->header != ZOMG_VDPCTRL_16_HEADER)
		return -2;
	
	// Byteswap the fields.
	ctrl->ctrl_word[0]	= be16_to_cpu(ctrl->ctrl_word[0]);
	ctrl->ctrl_word[1]	= be16_to_cpu(ctrl->ctrl_word[1]);
	ctrl->address		= be16_to_cpu(ctrl->address);
	ctrl->status		= be16_to_cpu(ctrl->status);
	
	// FIFO
	for (int i = 0; i < 4; i++)
		ctrl->data_fifo[i] = be16_to_cpu(ctrl->data_fifo[i]);
	
	// DMA
	ctrl->dma_src_address = be32_to_cpu(ctrl->dma_src_address);
	ctrl->dma_length = be16_to_cpu(ctrl->dma_length);
	
	// Clear the reserved fields.
	ctrl->reserved2 = 0;
	
	// Return the number of bytes read.
	return ret;
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
 * @param cram Destination buffer for CRam.
 * @return Number of bytes read on success; negative on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::loadCRam(Zomg_CRam_t *cram)
{
	int ret = loadFromZomg("common/CRam.bin", cram->md, sizeof(cram->md));
	
	// TODO: Only byteswap for MD.
	be16_to_cpu_array(cram->md, sizeof(cram->md));
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
	uint8_t data[sizeof(Zomg_M68KRegSave_t)];
	int ret = loadFromZomg("MD/M68K_reg.bin", &data, sizeof(data));
	
	if (ret == (int)sizeof(Zomg_M68KRegSave_Old_t))
	{
		// OLD VERSION. (pre-c43510fe)
		// DEPRECATED: Remove this once ZOMG is completed.
		fprintf(stderr, "Zomg::loadM68KReg(): %s: WARNING: Deprecated (pre-c43510fe) 74-byte MD/M68K_reg.bin found.\n",
			m_filename.c_str());
		Zomg_M68KRegSave_Old_t old_state;
		memcpy(&old_state, &data, sizeof(old_state));
		
		// Byteswap the 32-bit fields.
		for (int i = 0; i < 8; i++)
			state->dreg[i] = be32_to_cpu(old_state.dreg[i]);
		for (int i = 0; i < 7; i++)
			state->areg[i] = be32_to_cpu(old_state.areg[i]);
		
		// Byteswap the program counter.
		state->pc = be32_to_cpu(old_state.pc);
		
		// Byteswap the status register.
		state->sr = be16_to_cpu(old_state.sr);
		
		// Byteswap the stack pointers.
		if (state->sr & 0x2000)
		{
			// Supervisor mode.
			// old_state->areg[7] == ssp
			// old_state->asp     == usp
			state->ssp = be32_to_cpu(old_state.areg[7]);
			state->usp = be32_to_cpu(old_state.asp);
		}
		else
		{
			// User mode.
			// old_state->areg[7] == usp
			// old_state->asp     == ssp
			state->ssp = be32_to_cpu(old_state.asp);
			state->usp = be32_to_cpu(old_state.areg[7]);
		}
	}
	else if (ret >= (int)sizeof(Zomg_M68KRegSave_t))
	{
		// New version. (c43510fe or later)
		memcpy(state, &data, sizeof(*state));
		
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
		// Byteswapping is required.
		
		// Byteswap the main registers.
		for (int i = 0; i < 8; i++)
			state->dreg[i] = cpu_to_be32(state->dreg[i]);
		for (int i = 0; i < 7; i++)
			state->areg[i] = cpu_to_be32(state->areg[i]);
		
		// Byteswap the other registers.
		state->ssp = cpu_to_be32(state->ssp);
		state->usp = cpu_to_be32(state->usp);
		state->pc  = cpu_to_be32(state->pc);
		state->sr  = cpu_to_be16(state->sr);
#endif	
	}
	else
	{
		// Invalid size.
		fprintf(stderr, "Zomg::loadM68KReg(): %s: ERROR: Invalid MD/M68K_reg.bin found. (%d bytes)\n",
			m_filename.c_str(), ret);
		return -1;
	}
	
	// Clear the reserved fields.
	state->reserved1 = 0;
	state->reserved2 = 0;
	
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
	memset(state, 0xFF, sizeof(*state));
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
