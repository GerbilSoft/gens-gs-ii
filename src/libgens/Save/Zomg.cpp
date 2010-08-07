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

#ifdef _WIN32
#include "../../extlib/minizip/iowin32u.h"
#endif

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
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_zFile = unzOpen2_64(filename, &ffunc);
#else
	m_zFile = unzOpen(filename);
#endif
	
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
 * Zomg::loadFromZomg(): Load a file from the ZOMG file.
 * @param filename Filename to load from the ZOMG file.
 * @param buf Buffer to store the file in.
 * @param len Length of the buffer.
 * @return Length of file loaded, or negative number on error.
 */
int Zomg::loadFromZomg(const char *filename, void *buf, int len)
{
	if (!m_zFile)
		return -1;
	
	// Locate the file in the ZOMG file.
	int ret = unzLocateFile(m_zFile, filename, 2);
	if (ret != UNZ_OK)
	{
		// File not found.
		// TODO: Define return codes somewhere.
		return -2;
	}
	
	// Open the current file.
	ret = unzOpenCurrentFile(m_zFile);
	if (ret != UNZ_OK)
	{
		// Error opening the current file.
		return -3;
	}
	
	// Read the file.
	ret = unzReadCurrentFile(m_zFile, buf, len);
	unzCloseCurrentFile(m_zFile);	// TODO: Check the return value!
	
	// Return the number of bytes read.
	return ret;
}


/**
 * Zomg::load(): Load a ZOMG file.
 * @return 0 on success; non-zero on error.
 */
int Zomg::load(void)
{
	if (!m_zFile)
		return -1;
	
	// Assuming MD only.
	// TODO: Check for errors.
	
	// Load VDP registers.
	// TODO: Load a certain number depending on the system.
	// For now, we'll assume 24 (MD).
	loadFromZomg("common/vdp_reg.bin", m_common.VdpReg.md, sizeof(m_common.VdpReg.md));
	
	// Load VRam.
	loadFromZomg("common/VRam.bin", m_common.VRam.md, sizeof(m_common.VRam.md));
	
	// Load CRam.
	loadFromZomg("common/CRam.bin", m_common.CRam.md, sizeof(m_common.CRam.md));
	
	// Load VSRam.
	loadFromZomg("MD/VSRam.bin", m_md.VSRam, sizeof(m_md.VSRam));
	
	// Copy savestate data to the emulation memory buffers.
	
	// Write VDP registers.
	// TODO: On MD, load the DMA information from the savestate.
	// Writing to register 23 changes the DMA status.
	for (int i = (sizeof(m_common.VdpReg.md)/sizeof(m_common.VdpReg.md[0]))-1; i >= 0; i--)
	{
		VdpIo::Set_Reg(i, m_common.VdpReg.md[i]);
	}
	
	// Copy VRam to VdpIo.
	// TODO: Don't byteswap on BE systems.
	// TODO: Port byteswapping macros from Gens/GS.
	for (int i = (sizeof(m_common.VRam.md)/sizeof(m_common.VRam.md[0]))-1; i >= 0; i--)
	{
		VdpIo::VRam.u16[i] = (((m_common.VRam.md[i] >> 8) & 0xFF) |
				      ((m_common.VRam.md[i] & 0xFF) << 8));
	}
	
	// Copy CRam to VdpIo.
	// TODO: Don't byteswap on BE systems.
	// TODO: Port byteswapping macros from Gens/GS.
	for (int i = (sizeof(m_common.CRam.md)/sizeof(m_common.CRam.md[0]))-1; i >= 0; i--)
	{
		VdpIo::CRam.u16[i] = (((m_common.CRam.md[i] >> 8) & 0xFF) |
				      ((m_common.CRam.md[i] & 0xFF) << 8));
	}
	
	// Copy VSRam to VdpIo.
	memcpy(VdpIo::VSRam.u8, m_md.VSRam, sizeof(m_md.VSRam));
	
	// Savestate loaded.
	return 0;
}

}
