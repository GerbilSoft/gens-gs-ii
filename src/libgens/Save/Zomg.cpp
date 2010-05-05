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
#include "MD/VdpIo.hpp"

// C includes.
#include <stdint.h>
#include <string.h>

namespace LibGens
{

/**
 * Zomg::Zomg(): Open a ZOMG savestate file.
 * @param filename ZOMG filename.
 */
Zomg::Zomg(const char *filename)
{
	if (!filename)
	{
		// No filename specified.
		m_zFile = NULL;
		return;
	}
	
	// Open the ZOMG file.
	m_zFile = unzOpen(filename);
	if (!m_zFile)
	{
		// Couldn't open the ZOMG file.
		return;
	}
	
	// ZOMG file is open.
	// TODO: Process the ZOMG format file.
}


/**
 * Zomg::~Zomg(): Close the ZOMG savestate file.
 */
Zomg::~Zomg()
{
	if (!m_zFile)
		return;
	
	// Close the ZOMG file.
	unzClose(m_zFile);
}


/**
 * Zomg::close(): Close the ZOMG savestate file.
 */
void Zomg::close(void)
{
	if (!m_zFile)
		return;
	
	// Close the ZOMG file.
	unzClose(m_zFile);
	m_zFile = NULL;
}


/**
 * Zomg::loadVdpReg(): Load the VDP registers.
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadVdpReg(void)
{
	if (!m_zFile)
		return -1;
	
	// TODO: Load a certain number depending on the system.
	// For now, we'll assume 24 (MD).
	
	// Locate "common/vdp_reg.bin" in the ZOMG file.
	int ret = unzLocateFile(m_zFile, "common/vdp_reg.bin", 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return 1;
	}
	
	// Check the file information.
	unz_file_info zInfo;
	unzGetCurrentFileInfo(m_zFile, &zInfo, NULL, 0, NULL, 0, NULL, 0);
	if (zInfo.uncompressed_size != 24)
	{
		// Incorrect size.
		// Vdp_Reg size should be:
		// - 24: MD
		return 2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_zFile);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return 3;
	}
	
	// Get the VDP registers.
	uint8_t buf[24];
	ret = unzReadCurrentFile(m_zFile, buf, sizeof(buf));
	unzCloseCurrentFile(m_zFile);	// TODO: Check the return value!
	if (ret != sizeof(buf))
	{
		// Error reading the file.
		// TODO: Return an error code depending on the error.
		return 4;
	}
	
	// TODO: On MD, load the DMA information from the savestate.
	// Writing to register 23 changes the DMA status.
	for (int i = (sizeof(buf)/sizeof(buf[0]))-1; i >= 0; i--)
	{
		VdpIo::Set_Reg(i, buf[i]);
	}
	
	// VDP registers loaded.
	return 0;
}


/**
 * Zomg::loadVRam(): Load VRam.
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadVRam(void)
{
	if (!m_zFile)
		return -1;
	
	// Locate "common/VRam.bin" in the ZOMG file.
	int ret = unzLocateFile(m_zFile, "common/VRam.bin", 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return 1;
	}
	
	// Check the file information.
	unz_file_info zInfo;
	unzGetCurrentFileInfo(m_zFile, &zInfo, NULL, 0, NULL, 0, NULL, 0);
	if (zInfo.uncompressed_size != 65536)
	{
		// Incorrect size.
		// VRam size should be:
		// - 16384: SMS/GG
		// - 65536: MD
		return 2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_zFile);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return 3;
	}
	
	// Get the VRam data.
	// NOTE: Use uint16_t for MD, but uint8_t for SMS and GG.
	uint16_t buf[32768];
	ret = unzReadCurrentFile(m_zFile, buf, sizeof(buf));
	unzCloseCurrentFile(m_zFile);	// TODO: Check the return value!
	if (ret != sizeof(buf))
	{
		// Error reading the file.
		// TODO: Return an error code depending on the error.
		return 4;
	}
	
	// TODO: Don't byteswap on BE systems.
	// TODO: Port byteswapping macros from Gens/GS.
	for (int i = (sizeof(buf)/sizeof(buf[0]))-1; i >= 0; i--)
	{
		VdpIo::VRam.u16[i] = (((buf[i] >> 8) & 0xFF) | ((buf[i] & 0xFF) << 8));
	}
	
	// VRam update required.
	VdpIo::VDP_Flags.VRam = 1;
	
	// VRam loaded.
	return 0;
}


/**
 * Zomg::loadCRam(): Load CRam.
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadCRam(void)
{
	if (!m_zFile)
		return -1;
	
	// Locate "common/CRam.bin" in the ZOMG file.
	int ret = unzLocateFile(m_zFile, "common/CRam.bin", 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return 1;
	}
	
	// Check the file information.
	unz_file_info zInfo;
	unzGetCurrentFileInfo(m_zFile, &zInfo, NULL, 0, NULL, 0, NULL, 0);
	if (zInfo.uncompressed_size != 128)
	{
		// Incorrect size.
		// CRam size should be:
		// - 32: SMS
		// - 64: Game Gear
		// - 128: MD
		return 2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_zFile);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return 3;
	}
	
	// Get the CRam data.
	// NOTE: Use uint16_t for MD and GG, but uint8_t for SMS.
	uint16_t buf[64];
	ret = unzReadCurrentFile(m_zFile, buf, sizeof(buf));
	unzCloseCurrentFile(m_zFile);	// TODO: Check the return value!
	if (ret != sizeof(buf))
	{
		// Error reading the file.
		// TODO: Return an error code depending on the error.
		return 4;
	}
	
	// TODO: Don't byteswap on BE systems.
	// TODO: Port byteswapping macros from Gens/GS.
	for (int i = (sizeof(buf)/sizeof(buf[0]))-1; i >= 0; i--)
	{
		VdpIo::CRam.u16[i] = (((buf[i] >> 8) & 0xFF) | ((buf[i] & 0xFF) << 8));
	}
	
	// CRam update required.
	VdpIo::VDP_Flags.CRam = 1;
	
	// CRam loaded.
	return 0;
}


/**
 * Zomg::loadVSRam(): Load VSRam.
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadVSRam(void)
{
	if (!m_zFile)
		return -1;
	
	// Locate "MD/VSRam.bin" in the ZOMG file.
	int ret = unzLocateFile(m_zFile, "MD/VSRam.bin", 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return 1;
	}
	
	// Check the file information.
	unz_file_info zInfo;
	unzGetCurrentFileInfo(m_zFile, &zInfo, NULL, 0, NULL, 0, NULL, 0);
	if (zInfo.uncompressed_size != 80)
	{
		// Incorrect size.
		// VSRam size should be:
		// - 80: MD
		return 2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_zFile);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return 3;
	}
	
	// Get the VSRam data.
	uint8_t buf[80];
	ret = unzReadCurrentFile(m_zFile, buf, sizeof(buf));
	unzCloseCurrentFile(m_zFile);	// TODO: Check the return value!
	if (ret != sizeof(buf))
	{
		// Error reading the file.
		// TODO: Return an error code depending on the error.
		return 4;
	}
	
	// Copy VSRam to VdpIo.
	memcpy(VdpIo::VSRam.u8, buf, sizeof(buf));
	
	// VSRam loaded.
	return 0;
}

}
