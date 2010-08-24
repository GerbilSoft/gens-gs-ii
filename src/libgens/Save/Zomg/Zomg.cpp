/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Zomg.cpp: Zipped Original Memory from Genesis savestate handler.        *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#include "Zomg.hpp"
#include "Util/byteswap.h"

#include "MD/VdpIo.hpp"
#include "sound/SoundMgr.hpp"

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
Zomg::Zomg(const utf8_str *filename)
{
	if (!filename)
		return;
	
	m_filename = string(filename);
	
	// Defer opening the file until load() or save() is requested.
}


/**
 * ~Zomg(): Close the ZOMG savestate file.
 */
Zomg::~Zomg()
{
	// TODO
}


/**
 * LoadFromZomg(): Load a file from the ZOMG file.
 * @param unzZomg ZOMG file handle.
 * @param filename Filename to load from the ZOMG file.
 * @param buf Buffer to store the file in.
 * @param len Length of the buffer.
 * @return Length of file loaded, or negative number on error.
 */
int Zomg::LoadFromZomg(unzFile unzZomg, const utf8_str *filename, void *buf, int len)
{
	if (!unzZomg)
		return -1;
	
	// Locate the file in the ZOMG file.
	int ret = unzLocateFile(unzZomg, filename, 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return -2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(unzZomg);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return -3;
	}
	
	// Read the file.
	ret = unzReadCurrentFile(unzZomg, buf, len);
	unzCloseCurrentFile(unzZomg);	// TODO: Check the return value!
	
	// Return the number of bytes read.
	return ret;
}


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
	LoadFromZomg(unzZomg, "MD/YM2612_reg.bin", &m_md.ym2612, sizeof(m_md.ym2612));
	
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
	
	// Copy VSRam to VdpIo.
	memcpy(VdpIo::VSRam.u8, m_vdp.MD_VSRam, sizeof(m_vdp.MD_VSRam));
	
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
	
	// Copy VSRam from VdpIo.
	memcpy(m_vdp.MD_VSRam, VdpIo::VSRam.u8, sizeof(m_vdp.MD_VSRam));
	
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
	
	/** Write to the ZOMG file. **/
	SaveToZomg(zipZomg, "common/vdp_reg.bin", m_vdp.vdp_reg.md, sizeof(m_vdp.vdp_reg.md));
	SaveToZomg(zipZomg, "common/VRam.bin", m_vdp.VRam.md, sizeof(m_vdp.VRam.md));
	SaveToZomg(zipZomg, "common/CRam.bin", m_vdp.CRam.md, sizeof(m_vdp.CRam.md));
	SaveToZomg(zipZomg, "MD/VSRam.bin", m_vdp.MD_VSRam, sizeof(m_vdp.MD_VSRam));
	SaveToZomg(zipZomg, "common/psg.bin", &m_psg, sizeof(m_psg));
	SaveToZomg(zipZomg, "MD/YM2612_reg.bin", &m_md.ym2612, sizeof(m_md.ym2612));
	
	// Close the ZOMG file.
	zipClose(zipZomg, NULL);
	
	// Savestate saved.
	return 0;
}

}
