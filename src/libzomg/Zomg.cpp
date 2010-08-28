/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.hpp: Savestate handler.                                            *
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

#ifdef _WIN32
// MiniZip Win32 I/O handler.
#include "../../extlib/minizip/iowin32u.h"
#endif

// C includes.
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

namespace LibZomg
{

/**
 * Zomg(): Open a ZOMG savestate file.
 * @param filename ZOMG filename.
 */
Zomg::Zomg(const utf8_str *filename, ZomgFileMode mode)
{
	m_mode = ZOMG_CLOSED;
	m_unz = NULL;
	m_zip = NULL;
	
	if (!filename)
		return;
	
	// Open the ZOMG file.
	// TODO: Open for reading to load existing FORMAT.ini even if
	// the current mode is ZOMG_SAVE.
	
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
#endif
	
	switch (mode)
	{
		case ZOMG_LOAD:
#ifdef _WIN32
			m_unz = unzOpen2_64(filename, &ffunc);
#else
			m_unz = unzOpen(filename);
#endif
			if (!m_unz)
				return;
			break;
		
		case ZOMG_SAVE:
#ifdef _WIN32
			m_zip = zipOpen2_64(filename, APPEND_STATUS_CREATE, NULL, &ffunc);
#else
			m_zip = zipOpen(filename, APPEND_STATUS_CREATE);
#endif
			if (!m_zip)
				return;
			break;
		
		default:
			return;
	}
	
	// ZOMG file is open.
	m_mode = mode;
	
	// Save the filename.
	m_filename = string(filename);
}


/**
 * ~Zomg(): Close the ZOMG savestate file.
 */
Zomg::~Zomg()
{
	if (m_unz)
	{
		unzClose(m_unz);
		m_unz = NULL;
	}
	
	if (m_zip)
	{
		zipClose(m_zip, NULL);
		m_zip = NULL;
	}
	
	m_mode = ZOMG_CLOSED;
}


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
	// TODO: Set the Zip timestamps.
	zip_fileinfo zipfi;
	zipfi.tmz_date.tm_sec = 0;
	zipfi.tmz_date.tm_min = 0;
	zipfi.tmz_date.tm_hour = 0;
	zipfi.tmz_date.tm_mday = 0;
	zipfi.tmz_date.tm_mon = 0;
	zipfi.tmz_date.tm_year = 0;
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
	int ret;
	
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
	
	// Byteswap the 16-bit fields.
	
	// Byteswap the 16-bit and 32-bit fields.
	for (unsigned int i = 0; i < 8; i++)
	{
		bswap_state.areg[i] = cpu_to_be32(bswap_state.areg[i]);
		bswap_state.dreg[i] = cpu_to_be32(bswap_state.dreg[i]);
	}
	bswap_state.asp = cpu_to_be32(bswap_state.asp);
	bswap_state.pc  = cpu_to_be32(bswap_state.pc);
	bswap_state.sr  = cpu_to_be16(bswap_state.sr);
	
	return saveToZomg("MD/M68K_reg.bin", &bswap_state, sizeof(bswap_state));
#else
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

}
