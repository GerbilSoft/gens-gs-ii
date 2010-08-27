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

namespace LibGens
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
		zipClose(m_zip);
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
	int ret = loadFromZomg("common/vdp_reg.bin", reg, siz);
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
int Zomg::loadVSRam(void *vsram, size_t siz, bool byteswap)
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
 * @param state PSG register buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadPsgReg(Zomg_PsgSave_t *state)
{
	return loadFromZomg("common/psg.bin", state, sizeof(*state));
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
 * @param mem Z80 memory buffer.
 * @param siz Size of the Z80 memory buffer.
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
 * 16-bit and 32-bit fields are always byteswapped to host-endian.
 * @param state M68K register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadM68KReg(Zomg_M68KRegSave_t *state)
{
	int ret = loadFromZomg("common/Z80_reg.bin", state, sizeof(*state));
	
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
}}


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
 * loadMD_IO(): Load MD Z80 control registers. (MD-specific)
 * 16-bit fields are always byteswapped to host-endian.
 * @param state MD Z80 control register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state)
{
	int ret = loadFromZomg("MD/Z80_ctrl.bin", state, sizeof(*state));
	
	// Byteswap the 16-bit fields.
	state->m68k_bank = be16_to_cpu(state->m68k_bank);
	
	return ret;
}


#if 0
/**
 * Zomg::load(): Load a ZOMG file.
 * @return 0 on success; non-zero on error.
 */
int Zomg::load(void)
{
	// TODO: Error code constants.
	
	// Open the ZOMG file.
	unzFile unzZomg;
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	unzZomg = unzOpen2_64(m_filename.c_str(), &ffunc);
#else
	unzZomg = unzOpen(m_filename.c_str());
#endif
	
	if (!unzZomg)
	{
		// Couldn't open the ZOMG file.
		return -1;
	}
	
	// ZOMG file is open.
	// TODO: Process the ZOMG format file.
	
	// Assuming MD only.
	// TODO: Check for errors.
	
	/** Load from the ZOMG file. **/
	LoadFromZomg(unzZomg, "common/vdp_reg.bin", m_vdp.vdp_reg.md, sizeof(m_vdp.vdp_reg.md));
	LoadFromZomg(unzZomg, "common/VRam.bin", m_vdp.VRam.md, sizeof(m_vdp.VRam.md));
	LoadFromZomg(unzZomg, "common/CRam.bin", m_vdp.CRam.md, sizeof(m_vdp.CRam.md));
	LoadFromZomg(unzZomg, "MD/VSRam.bin", m_vdp.MD_VSRam, sizeof(m_vdp.MD_VSRam));
	LoadFromZomg(unzZomg, "common/psg.bin", &m_psg, sizeof(m_psg));
	LoadFromZomg(unzZomg, "common/Z80_mem.bin", &m_z80_mem.mem_mk3, sizeof(m_z80_mem.mem_mk3));
	LoadFromZomg(unzZomg, "common/Z80_reg.bin", &m_z80_reg, sizeof(m_z80_reg));
	// MD-specific.
	LoadFromZomg(unzZomg, "MD/YM2612_reg.bin", &m_md.ym2612, sizeof(m_md.ym2612));
	LoadFromZomg(unzZomg, "MD/M68K_mem.bin", &m_md.m68k_mem, sizeof(m_md.m68k_mem));
	LoadFromZomg(unzZomg, "MD/M68K_reg.bin", &m_md.m68k_reg, sizeof(m_md.m68k_reg));
	LoadFromZomg(unzZomg, "MD/IO.bin", &m_md.md_io, sizeof(m_md.md_io));
	LoadFromZomg(unzZomg, "MD/Z80_ctrl.bin", &m_md.md_z80_ctrl, sizeof(m_md.md_z80_ctrl));
	
	// Close the ZOMG file.
	unzClose(unzZomg);
	
	// Copy savestate data to the emulation memory buffers.
	
	/** VDP **/
	
	// Write VDP registers.
	// TODO: On MD, load the DMA information from the savestate.
	// Writing to register 23 changes the DMA status.
	for (int i = (sizeof(m_vdp.vdp_reg.md)/sizeof(m_vdp.vdp_reg.md[0]))-1; i >= 0; i--)
	{
		VdpIo::Set_Reg(i, m_vdp.vdp_reg.md[i]);
	}
	
	// Copy VRam to VdpIo.
	// TODO: Create a byteswapping memcpy().
	memcpy(VdpIo::VRam.u16, m_vdp.VRam.md, sizeof(m_vdp.VRam.md));
	be16_to_cpu_array(VdpIo::VRam.u16, sizeof(m_vdp.VRam.md));
	
	// Copy CRam to VdpIo.
	// TODO: Create a byteswapping memcpy().
	memcpy(VdpIo::CRam.u16, m_vdp.CRam.md, sizeof(m_vdp.CRam.md));
	be16_to_cpu_array(VdpIo::CRam.u16, sizeof(m_vdp.CRam.md));
	
	/** VDP: MD specific **/
	
	// Load VSRam.
	// TODO: Create a byteswapping memcpy().
	for (unsigned int i = 0; i < (sizeof(m_vdp.MD_VSRam)/sizeof(m_vdp.MD_VSRam[0])); i++)
	{
		m_vdp.MD_VSRam[i] = be16_to_cpu(m_vdp.MD_VSRam[i]);
	}
	memcpy(VdpIo::VSRam.u16, m_vdp.MD_VSRam, sizeof(m_vdp.MD_VSRam));
	
	/** Audio **/
	
	// Byteswap PSG values.
	// TODO: LE16 or BE16 for PSG?
	for (unsigned int i = 0; i < 4; i++)
	{
		m_psg.tone_reg[i] = le16_to_cpu(m_psg.tone_reg[i]);
		m_psg.tone_ctr[i] = le16_to_cpu(m_psg.tone_ctr[i]);
	}
	m_psg.lfsr_state = le16_to_cpu(m_psg.lfsr_state);
	// Load the PSG state.
	SoundMgr::ms_Psg.zomgRestore(&m_psg);
	
	/** Audio: MD specific **/
	
	// Load the YM2612 state.
	SoundMgr::ms_Ym2612.zomgRestore(&m_md.ym2612);
	
	/** Z80 **/
	
	// Load the Z80 memory.
	// TODO: Use the correct size based on system.
	memcpy(Ram_Z80, m_z80_mem.mem_mk3, sizeof(m_z80_mem.mem_mk3));
	
	// Load the Z80 registers.
	Z80::ZomgRestoreReg(&m_z80_reg);
	
	/** MD: M68K **/
	
	// Load the M68K memory.
	// TODO: Create a byteswapping memcpy().
	memcpy(&Ram_68k.u16[0], m_md.m68k_mem.mem, sizeof(m_md.m68k_mem.mem));
	be16_to_cpu_array(&Ram_68k.u16[0], sizeof(m_md.m68k_mem.mem));
	
	// Load the M68K registers.
	M68K::ZomgRestoreReg(&m_md.m68k_reg);
	
	/** MD: Other **/
	
	// Load the I/O registers. ($A10001-$A1001F, odd bytes)
	// TODO: Create/use the version register function in M68K_Mem.cpp.
	IoBase::Zomg_MD_IoSave_int_t io_int;
	// TODO: Set MD version register.
	//m_md.md_io.version_reg = ((M68K_Mem::ms_Region.region() << 6) | 0x20);
	io_int.data     = m_md.md_io.port1_data;
	io_int.ctrl     = m_md.md_io.port1_ctrl;
	io_int.ser_tx   = m_md.md_io.port1_ser_tx;
	io_int.ser_rx   = m_md.md_io.port1_ser_rx;
	io_int.ser_ctrl = m_md.md_io.port1_ser_ctrl;
	EmuMD::m_port1->zomgRestoreMD(&io_int);
	io_int.data     = m_md.md_io.port2_data;
	io_int.ctrl     = m_md.md_io.port2_ctrl;
	io_int.ser_tx   = m_md.md_io.port2_ser_tx;
	io_int.ser_rx   = m_md.md_io.port2_ser_rx;
	io_int.ser_ctrl = m_md.md_io.port2_ser_ctrl;
	EmuMD::m_port2->zomgRestoreMD(&io_int);
	io_int.data     = m_md.md_io.port3_data;
	io_int.ctrl     = m_md.md_io.port3_ctrl;
	io_int.ser_tx   = m_md.md_io.port3_ser_tx;
	io_int.ser_rx   = m_md.md_io.port3_ser_rx;
	io_int.ser_ctrl = m_md.md_io.port3_ser_ctrl;
	EmuMD::m_portE->zomgRestoreMD(&io_int);
	
	// Load the Z80 control registers.
	M68K_Mem::Z80_State &= Z80_STATE_ENABLED;
	if (!m_md.md_z80_ctrl.busreq)
		M68K_Mem::Z80_State |= Z80_STATE_BUSREQ;
	if (!m_md.md_z80_ctrl.reset)
		M68K_Mem::Z80_State |= Z80_STATE_RESET;
	Z80_MD_Mem::Bank_Z80 = ((be16_to_cpu(m_md.md_z80_ctrl.m68k_bank) & 0x1FF) << 15);
	
	// Savestate loaded.
	return 0;
}


/**
 * SaveToZomg(): Save a file to the ZOMG file.
 * @param zipZomg ZOMG file handle.
 * @param filename Filename to save in the ZOMG file.
 * @param buf Buffer containing the file contents.
 * @param len Length of the buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::SaveToZomg(zipFile zipZomg, const utf8_str *filename, void *buf, int len)
{
	if (!zipZomg)
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
		zipZomg,		// zipFile
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
	zipWriteInFileInZip(zipZomg, buf, len); // TODO: Check the return value!
	zipCloseFileInZip(zipZomg);		// TODO: Check the return value!
	
	// TODO: What should we return?
	return 0;
}


/**
 * save(): Save a ZOMG file.
 * @return 0 on success; non-zero on error.
 */
int Zomg::save(void)
{
	// TODO: Open the ZOMG file and load its format information first!
	// TODO: Add a global Zip comment?
	// TODO: Error code constants.
	
	// Open the ZOMG file.
	zipFile zipZomg;
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	zipZomg = zipOpen2_64(m_filename.c_str(), APPEND_STATUS_CREATE, NULL, &ffunc);
#else
	zipZomg = zipOpen(m_filename.c_str(), APPEND_STATUS_CREATE);
#endif
	
	if (!zipZomg)
	{
		// Couldn't open the ZOMG file.
		return -1;
	}
	
	// ZOMG file is open.
	// TODO: Create the ZOMG format file.
	
	// Assuming MD only.
	// TODO: Check for errors.
	
	// Copy emulation memory buffers to Zomg buffers.
	
	/** VDP **/
	
	// Save VDP registers.
	for (int i = (sizeof(m_vdp.vdp_reg.md)/sizeof(m_vdp.vdp_reg.md[0]))-1; i >= 0; i--)
	{
		m_vdp.vdp_reg.md[i] = VdpIo::VDP_Reg.reg[i];
	}
	
	// Copy VRam from VdpIo.
	// TODO: Create a byteswapping memcpy().
	memcpy(m_vdp.VRam.md, VdpIo::VRam.u16, sizeof(m_vdp.VRam.md));
	be16_to_cpu_array(m_vdp.VRam.md, sizeof(m_vdp.VRam.md));
	
	// Copy CRam from VdpIo.
	// TODO: Create a byteswapping memcpy().
	memcpy(m_vdp.CRam.md, VdpIo::CRam.u16, sizeof(m_vdp.CRam.md));
	be16_to_cpu_array(m_vdp.CRam.md, sizeof(m_vdp.CRam.md));
	
	/** VDP: MD specific **/
	
	// Save VSRam.
	// TODO: Create a byteswapping memcpy().
	memcpy(m_vdp.MD_VSRam, VdpIo::VSRam.u16, sizeof(m_vdp.MD_VSRam));
	for (unsigned int i = 0; i < (sizeof(m_vdp.MD_VSRam)/sizeof(m_vdp.MD_VSRam[0])); i++)
	{
		m_vdp.MD_VSRam[i] = cpu_to_be16(m_vdp.MD_VSRam[i]);
	}
	
	/** Audio **/
	
	// Save the PSG state.
	SoundMgr::ms_Psg.zomgSave(&m_psg);
	// Byteswap PSG values.
	// TODO: LE16 or BE16 for PSG?
	for (unsigned int i = 0; i < 4; i++)
	{
		m_psg.tone_reg[i] = cpu_to_le16(m_psg.tone_reg[i]);
		m_psg.tone_ctr[i] = cpu_to_le16(m_psg.tone_ctr[i]);
	}
	m_psg.lfsr_state = cpu_to_le16(m_psg.lfsr_state);
	
	/** Audio: MD specific **/
	
	// Save the YM2612 state.
	SoundMgr::ms_Ym2612.zomgSave(&m_md.ym2612);
	
	/** Z80 **/
	
	// Load the Z80 memory.
	// TODO: Use the correct size based on system.
	memcpy(m_z80_mem.mem_mk3, Ram_Z80, sizeof(m_z80_mem.mem_mk3));
	
	// Save the Z80 registers.
	Z80::ZomgSaveReg(&m_z80_reg);
	
	/** MD: M68K **/
	
	// Save the M68K memory.
	// TODO: Create a byteswapping memcpy().
	memcpy(m_md.m68k_mem.mem, &Ram_68k.u16[0], sizeof(m_md.m68k_mem));
	be16_to_cpu_array(m_md.m68k_mem.mem, sizeof(m_md.m68k_mem));
	
	// Save the M68K registers.
	M68K::ZomgSaveReg(&m_md.m68k_reg);
	
	/** MD: Other **/
	
	// Save the I/O registers. ($A10001-$A1001F, odd bytes)
	// TODO: Create/use the version register function in M68K_Mem.cpp.
	IoBase::Zomg_MD_IoSave_int_t io_int;
	m_md.md_io.version_reg = ((M68K_Mem::ms_Region.region() << 6) | 0x20);
	EmuMD::m_port1->zomgSaveMD(&io_int);
	m_md.md_io.port1_data     = io_int.data;
	m_md.md_io.port1_ctrl     = io_int.ctrl;
	m_md.md_io.port1_ser_tx   = io_int.ser_tx;
	m_md.md_io.port1_ser_rx   = io_int.ser_rx;
	m_md.md_io.port1_ser_ctrl = io_int.ser_ctrl;
	EmuMD::m_port2->zomgSaveMD(&io_int);
	m_md.md_io.port2_data     = io_int.data;
	m_md.md_io.port2_ctrl     = io_int.ctrl;
	m_md.md_io.port2_ser_tx   = io_int.ser_tx;
	m_md.md_io.port2_ser_rx   = io_int.ser_rx;
	m_md.md_io.port2_ser_ctrl = io_int.ser_ctrl;
	EmuMD::m_portE->zomgSaveMD(&io_int);
	m_md.md_io.port3_data     = io_int.data;
	m_md.md_io.port3_ctrl     = io_int.ctrl;
	m_md.md_io.port3_ser_tx   = io_int.ser_tx;
	m_md.md_io.port3_ser_rx   = io_int.ser_rx;
	m_md.md_io.port3_ser_ctrl = io_int.ser_ctrl;
	
	// Save the Z80 control registers.
	m_md.md_z80_ctrl.busreq    = !(M68K_Mem::Z80_State & Z80_STATE_BUSREQ);
	m_md.md_z80_ctrl.reset     = !(M68K_Mem::Z80_State & Z80_STATE_RESET);
	m_md.md_z80_ctrl.m68k_bank = cpu_to_be16((Z80_MD_Mem::Bank_Z80 >> 15) & 0x1FF);
	
	/** Write to the ZOMG file. **/
	SaveToZomg(zipZomg, "common/vdp_reg.bin", m_vdp.vdp_reg.md, sizeof(m_vdp.vdp_reg.md));
	SaveToZomg(zipZomg, "common/VRam.bin", m_vdp.VRam.md, sizeof(m_vdp.VRam.md));
	SaveToZomg(zipZomg, "common/CRam.bin", m_vdp.CRam.md, sizeof(m_vdp.CRam.md));
	SaveToZomg(zipZomg, "MD/VSRam.bin", m_vdp.MD_VSRam, sizeof(m_vdp.MD_VSRam));
	SaveToZomg(zipZomg, "common/psg.bin", &m_psg, sizeof(m_psg));
	SaveToZomg(zipZomg, "common/Z80_mem.bin", &m_z80_mem.mem_mk3, sizeof(m_z80_mem.mem_mk3));
	SaveToZomg(zipZomg, "common/Z80_reg.bin", &m_z80_reg, sizeof(m_z80_reg));
	// MD-specific.
	SaveToZomg(zipZomg, "MD/YM2612_reg.bin", &m_md.ym2612, sizeof(m_md.ym2612));
	SaveToZomg(zipZomg, "MD/M68K_mem.bin", &m_md.m68k_mem, sizeof(m_md.m68k_mem));
	SaveToZomg(zipZomg, "MD/M68K_reg.bin", &m_md.m68k_reg, sizeof(m_md.m68k_reg));
	SaveToZomg(zipZomg, "MD/IO.bin", &m_md.md_io, sizeof(m_md.md_io));
	SaveToZomg(zipZomg, "MD/Z80_ctrl.bin", &m_md.md_z80_ctrl, sizeof(m_md.md_z80_ctrl));
	
	// Close the ZOMG file.
	zipClose(zipZomg, NULL);
	
	// Savestate saved.
	return 0;
}
#endif

}
