/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgLoad.cpp: Savestate handler. (Saving functions)                     *
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
 * saveToZomg(): Save a file to the ZOMG file.
 * @param filename Filename to save in the ZOMG file.
 * @param buf Buffer containing the file contents.
 * @param len Length of the buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveToZomg(const utf8_str *filename, const void *buf, int len)
{
	if (m_mode != ZOMG_SAVE || !m_zip)
		return -1;
	
	// Open the new file in the ZOMG file.
	zip_fileinfo zipfi;
	memcpy(&zipfi.tmz_date, &m_zipfi.tmz_date, sizeof(zipfi.tmz_date));
	zipfi.dosDate = 0;
	zipfi.internal_fa = 0x0000; // TODO: Set to 0x0001 for text files.
	zipfi.external_fa = 0x0000; // MS-DOS directory attribute byte.
	
	int ret = zipOpenNewFileInZip(
		m_zip,			// zipFile
		filename,		// Filename in the Zip archive
		&zipfi,			// File information (timestamp, attributes)
		NULL,			// extrafield_local
		0,			// size_extrafield_local,
		NULL,			// extrafield_global,
		0,			// size_extrafield_global,
		NULL,			// comment
		Z_DEFLATED,		// method
		Z_DEFAULT_COMPRESSION	// level
		);
	
	if (ret != UNZ_OK)
	{
		// Error opening the new file in the Zip archive.
		// TODO: Define return codes somewhere.
		return -2;
	}
	
	// Write the file.
	zipWriteInFileInZip(m_zip, buf, len);	// TODO: Check the return value!
	zipCloseFileInZip(m_zip);		// TODO: Check the return value!
	
	// TODO: What should we return?
	return 0;
}


/**
 * Save savestate functions.
 * @param siz Number of bytes to write.
 * @return 0 on success; non-zero on error.
 * TODO: Standardize error codes.
 */

// TODO: Determine siz and is16bit from the system type?
// (once FORMAT.ini is implemented)


/**
 * savePreview(): Save the preview image.
 * @param img_buf Image buffer. (Must have a PNG image.)
 * @param siz Size of the image buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::savePreview(const void *img_buf, size_t siz)
{
	// Verify the PNG "magic number".
	static const uint8_t png_magic[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	
	if (siz < sizeof(png_magic) ||
	    memcmp(img_buf, png_magic, sizeof(png_magic)) != 0)
	{
		// Invalid "magic number".
		return -2;
	}
	
	// Write the image buffer to the ZOMG file.
	return saveToZomg("preview.png", img_buf, siz);
}


/** VDP **/


/**
 * saveVdpReg(): Save VDP registers.
 * File: common/vdp_reg.bin
 * @param reg Source buffer for VDP registers.
 * @param siz Number of VDP registers to load.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpReg(const uint8_t *reg, size_t siz)
{
	return saveToZomg("common/vdp_reg.bin", reg, siz);
}


/**
 * saveVRam(): Save VRam.
 * File: common/VRam.bin
 * @param vram Destination buffer for VRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap from host-endian 16-bit.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveVRam(const void *vram, size_t siz, bool byteswap)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	if (byteswap)
	{
		// TODO: Byteswapping memcpy().
		uint16_t *bswap_buf = (uint16_t*)malloc(siz);
		memcpy(bswap_buf, vram, siz);
		cpu_to_be16_array(bswap_buf, siz);
		return saveToZomg("common/VRam.bin", bswap_buf, siz);
	}
	else
#endif
	{
		return saveToZomg("common/VRam.bin", vram, siz);
	}
}


/**
 * saveCRam(): Save CRam.
 * File: common/CRam.bin
 * @param vram Destination buffer for CRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap from host-endian 16-bit.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveCRam(const void *cram, size_t siz, bool byteswap)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	if (byteswap)
	{
		// TODO: Byteswapping memcpy().
		uint16_t *bswap_buf = (uint16_t*)malloc(siz);
		memcpy(bswap_buf, cram, siz);
		cpu_to_be16_array(bswap_buf, siz);
		return saveToZomg("common/CRam.bin", bswap_buf, siz);
	}
	else
#endif
	{
		return saveToZomg("common/CRam.bin", cram, siz);
	}
}


/**
 * saveMD_VSRam(): Save VSRam. (MD-specific)
 * File: MD/VSRam.bin
 * @param vsram Destination buffer for VSRam.
 * @param siz Number of bytes to read.
 * @param byteswap If true, byteswap from host-endian 16-bit.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_VSRam(const uint16_t *vsram, size_t siz, bool byteswap)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	if (byteswap)
	{
		// TODO: Byteswapping memcpy().
		uint16_t *bswap_buf = (uint16_t*)malloc(siz);
		memcpy(bswap_buf, vsram, siz);
		cpu_to_be16_array(bswap_buf, siz);
		return saveToZomg("MD/VSRam.bin", bswap_buf, siz);
	}
	else
#endif
	{
		return saveToZomg("MD/VSRam.bin", vsram, siz);
	}
}


/** Audio **/


/**
 * savePsgReg(): Save PSG registers.
 * File: common/psg.bin
 * 16-bit fields are always byteswapped from host-endian.
 * @param state PSG register buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::savePsgReg(const Zomg_PsgSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_BIG_ENDIAN
	Zomg_PsgSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state);
	
	// Byteswap the 16-bit fields.
	for (unsigned int i = 0; i < 4; i++)
	{
		bswap_state.tone_reg[i] = cpu_to_le16(bswap_state.tone_reg[i]);
		bswap_state.tone_ctr[i] = cpu_to_le16(bswap_state.tone_ctr[i]);
	}
	bswap_state.lfsr_state = cpu_to_le16(bswap_state.lfsr_state);
	return saveToZomg("common/psg.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("common/psg.bin", state, sizeof(*state));
#endif
}


/**
 * saveMD_YM2612_reg(): Save YM2612 registers. (MD-specific)
 * File: MD/YM2612_reg.bin
 * @param state YM2612 register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_YM2612_reg(const Zomg_Ym2612Save_t *state)
{
	return saveToZomg("MD/YM2612_reg.bin", state, sizeof(*state));
}


/** Z80 **/


/**
 * saveZ80Mem(): Save Z80 memory.
 * File: common/Z80_mem.bin
 * @param mem Z80 memory buffer.
 * @param siz Size of the Z80 memory buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveZ80Mem(const uint8_t *mem, size_t siz)
{
	return saveToZomg("common/Z80_mem.bin", mem, siz);
}


/**
 * saveZ80Reg(): Save Z80 registers.
 * File: common/Z80_reg.bin
 * 16-bit fields are always byteswapped from host-endian.
 * @param state Z80 register buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveZ80Reg(const Zomg_Z80RegSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_BIG_ENDIAN
	Zomg_Z80RegSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));
	
	// Byteswap the 16-bit fields.
	
	// Main register set.
	bswap_state.AF = cpu_to_le16(bswap_state.AF);
	bswap_state.BC = cpu_to_le16(bswap_state.BC);
	bswap_state.DE = cpu_to_le16(bswap_state.DE);
	bswap_state.HL = cpu_to_le16(bswap_state.HL);
	bswap_state.IX = cpu_to_le16(bswap_state.IX);
	bswap_state.IY = cpu_to_le16(bswap_state.IY);
	bswap_state.PC = cpu_to_le16(bswap_state.PC);
	bswap_state.SP = cpu_to_le16(bswap_state.SP);
	
	// Shadow register set.
	bswap_state.AF2 = cpu_to_le16(bswap_state.AF2);
	bswap_state.BC2 = cpu_to_le16(bswap_state.BC2);
	bswap_state.DE2 = cpu_to_le16(bswap_state.DE2);
	bswap_state.HL2 = cpu_to_le16(bswap_state.HL2);
	
	return saveToZomg("common/Z80_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("common/Z80_reg.bin", state, sizeof(*state));
#endif
}


/** M68K (MD-specific) **/


/**
 * saveM68KMem(): Save M68K memory. (MD-specific)
 * File: MD/M68K_mem.bin
 * @param mem M68K memory buffer.
 * @param siz Size of the M68K memory buffer.
 * @param byteswap If true, byteswap from host-endian 16-bit.
 * @param state Z80 register buffer.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveM68KMem(const uint16_t *mem, size_t siz, bool byteswap)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	if (byteswap)
	{
		// TODO: Byteswapping memcpy().
		uint16_t *bswap_buf = (uint16_t*)malloc(siz);
		memcpy(bswap_buf, mem, siz);
		cpu_to_be16_array(bswap_buf, siz);
		return saveToZomg("MD/M68K_mem.bin", bswap_buf, siz);
	}
	else
#endif
	{
		return saveToZomg("MD/M68K_mem.bin", mem, siz);
	}
}


/**
 * saveM68KReg(): Save M68K registers. (MD-specific)
 * File: MD/M68K_reg.bin
 * 16-bit and 32-bit fields are always byteswapped from host-endian.
 * @param state M68K register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveM68KReg(const Zomg_M68KRegSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_M68KRegSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));
	
	// Byteswap the registers.
	for (int i = 0; i < 8; i++)
		bswap_state.dreg[i] = cpu_to_be32(bswap_state.dreg[i]);
	for (int i = 0; i < 7; i++)
		bswap_state.areg[i] = cpu_to_be32(bswap_state.areg[i]);
	
	bswap_state.ssp = cpu_to_be32(bswap_state.ssp);
	bswap_state.usp = cpu_to_be32(bswap_state.usp);
	bswap_state.pc  = cpu_to_be32(bswap_state.pc);
	bswap_state.sr  = cpu_to_be16(bswap_state.sr);
	
	// Clear the reserved fields.
	bswap_state.reserved1 = 0;
	bswap_state.reserved2 = 0;
	
	return saveToZomg("MD/M68K_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	// TODO: Make sure the reserved fields are cleared.
	return saveToZomg("MD/M68K_reg.bin", state, sizeof(*state));
#endif
}


/** MD-specific registers. **/


/**
 * saveMD_IO(): Save MD I/O port registers. (MD-specific)
 * @param state MD I/O port register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_IO(const Zomg_MD_IoSave_t *state)
{
	return saveToZomg("MD/IO.bin", state, sizeof(*state));
}


/**
 * saveMD_Z80Ctrl(): Save MD Z80 control registers. (MD-specific)
 * 16-bit fields are always byteswapped from host-endian.
 * @param state MD Z80 control register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_Z80Ctrl(const Zomg_MD_Z80CtrlSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_MD_Z80CtrlSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));
	
	// Byteswap the 16-bit fields.
	bswap_state.m68k_bank = be16_to_cpu(bswap_state.m68k_bank);
	
	return saveToZomg("MD/Z80_ctrl.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("MD/Z80_ctrl.bin", state, sizeof(*state));
#endif
}


/**
 * saveMD_TimeReg(): Save MD /TIME registers. (MD-specific)
 * @param state MD /TIME register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_TimeReg(const Zomg_MD_TimeReg_t *state)
{
	return saveToZomg("MD/TIME_reg.bin", state, sizeof(*state));
}


/** Miscellaneous **/


/**
 * saveSRam(): Save SRAM.
 * @param sram Pointer to SRAM buffer.
 * @param siz Size of SRAM buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveSRam(const uint8_t *sram, size_t siz)
{
	return saveToZomg("common/SRam.bin", sram, siz);
}

}
