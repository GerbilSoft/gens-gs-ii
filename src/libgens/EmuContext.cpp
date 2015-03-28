/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContext.cpp: Emulation context base class.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#include "EmuContext.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

// C++ includes.
#include <string>
using std::string;

#include "lg_osd.h"

// ROM data access.
// TODO: Move ROM data to a cartridge class?
// NOTE: Currently only needed for fixChecksum() / restoreChecksum().
// Maybe fixChecksum() / restoreChecksum() should be moved to EmuMD.
#include "cpu/M68K_Mem.hpp"

namespace LibGens
{

// Reference counter.
// We're only allowing one emulation context at the moment.
int EmuContext::ms_RefCount = 0;

// Controller I/O manager.
// TODO: Make this non-static!
IoManager *EmuContext::m_ioManager;

// Static pointer. Temporarily needed for SRam/EEPRom.
EmuContext *EmuContext::m_instance = nullptr;

/**
 * Global settings.
 */
bool EmuContext::ms_AutoFixChecksum = true;
string EmuContext::ms_PathSRam;
string EmuContext::ms_TmssRomFilename;
bool EmuContext::ms_TmssEnabled = false;


/**
 * Initialize an emulation context.
 * @param rom ROM.
 * @param region System region. (not used in the base class)
 */
EmuContext::EmuContext(Rom *rom, SysVersion::RegionCode_t region)
{
	init(nullptr, rom, region);
}

/**
 * Initialize an emulation context.
 * @param fb Existing MdFb to use. (If nullptr, allocate a new MdFb.)
 * @param rom ROM.
 * @param region System region. (not used in the base class)
 */
EmuContext::EmuContext(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region)
{
	init(fb, rom, region);
}

/**
 * Initialize an emulation context.
 * @param fb Existing MdFb to use. (If nullptr, allocate a new MdFb.)
 * @param rom ROM.
 * @param region System region. (not used in the base class)
 */
void EmuContext::init(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region)
{
	// NOTE: Region code isn't used in the base class.
	// This may change later on.
	((void)region);

	ms_RefCount++;
	assert(ms_RefCount == 1);
	m_instance = this;

	// Initialize variables.
	m_rom = rom;
	m_saveDataEnable = true;	// Enabled by default. (TODO: Config setting.)

	// Create the Controller I/O manager.
	// TODO: For now, we'll treat them as static.
	if (!m_ioManager)
		m_ioManager = new IoManager();

	// Initialize the VDP.
	// TODO: Apply user-specified VDP options.
	m_vdp = new Vdp(fb);
}

EmuContext::~EmuContext()
{
	ms_RefCount--;
	assert(ms_RefCount == 0);
	m_instance = nullptr;

	// TODO: Delete the IoManager?
	//delete m_ioManager;
	//m_ioManager = nullptr;

	// Delete the VDP.
	delete m_vdp;
	m_vdp = nullptr;
}


/**
 * Set the SRam/EEPRom save path [static]
 * @param newPathSRam New SRam/EEPRom save path.
 */
void EmuContext::SetPathSRam(const std::string &newPathSRam)
{
	ms_PathSRam = newPathSRam;

	// TODO: Update SRam/EEPRom classes in active contexts.
}

}
