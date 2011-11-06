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

// Controllers.
// TODO: Figure out a better place to put these!
// TODO: Make these non-static!
IoBase *EmuContext::m_port1;		// Player 1.
IoBase *EmuContext::m_port2;		// Player 2.
IoBase *EmuContext::m_portE;		// EXT port.

// Static pointer. Temporarily needed for SRam/EEPRom.
EmuContext *EmuContext::m_instance = NULL;

/**
 * Global settings.
 */
bool EmuContext::ms_AutoFixChecksum = true;
string EmuContext::ms_PathSRam;


/**
 * Initialize an emulation context.
 * @param rom ROM.
 * @param region System region. (not used in the base class)
 */
EmuContext::EmuContext(Rom *rom, SysVersion::RegionCode_t region)
{
	init(NULL, rom, region);
}

/**
 * Initialize an emulation context.
 * @param fb Existing MdFb to use. (If NULL, allocate a new MdFb.)
 * @param rom ROM.
 * @param region System region. (not used in the base class)
 */
EmuContext::EmuContext(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region)
{
	init(fb, rom, region);
}

/**
 * Initialize an emulation context.
 * @param fb Existing MdFb to use. (If NULL, allocate a new MdFb.)
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
	
	// Create base I/O devices that do nothing.
	// TODO: For now, we'll treat them as static.
	if (!m_port1)
		m_port1 = new IoBase();
	if (!m_port2)
		m_port2 = new IoBase();
	if (!m_portE)
		m_portE = new IoBase();
	
	// Set the SRam and EEPRom pathnames.
	// TODO: Update them if the pathname is changed.
	m_SRam.setPathname(ms_PathSRam);
	m_EEPRom.setPathname(ms_PathSRam);
	
	// Initialize EEPRom.
	// EEPRom is only used if the ROM is in the EEPRom class's database.
	// Otherwise, SRam is used.
	int eepromSize = m_rom->initEEPRom(&m_EEPRom);
	if (eepromSize > 0)
	{
		// EEPRom was initialized.
		lg_osd(OSD_EEPROM_LOAD, eepromSize);
	}
	else
	{
		// EEPRom was not initialized.
		// Initialize SRam.
		int sramSize = m_rom->initSRam(&m_SRam);
		if (sramSize > 0)
			lg_osd(OSD_SRAM_LOAD, sramSize);
	}
	
	// Initialize the VDP.
	m_vdp = new Vdp(fb);
}

EmuContext::~EmuContext()
{
	ms_RefCount--;
	assert(ms_RefCount == 0);
	m_instance = NULL;
	
	// Delete the I/O devices.
	// TODO: Don't do this right now.
#if 0
	delete m_port1;
	m_port1 = NULL;
	delete m_port2;
	m_port2 = NULL;
	delete m_portE;
	m_portE = NULL;
#endif
	
	// Delete the VDP.
	delete m_vdp;
	m_vdp = NULL;
}


/**
 * fixChecksum(): Fix the ROM checksum.
 * This function uses the standard Sega checksum formula.
 * Results are applied to M68K_Mem::Rom_Data[] only.
 * The ROM header checksum will remain the same.
 * 
 * NOTE: This function may be overridden for e.g. SMS,
 * which uses a different checksum method.
 * 
 * @return 0 on success; non-zero on error.
 */
int EmuContext::fixChecksum(void)
{
	if (!m_rom || M68K_Mem::Rom_Size <= 0x200)
		return -1;
	
	// Calculate the ROM checksum.
	// NOTE: ROM is byteswapped. (Header data is read before calling Rom::loadRom().)
	// NOTE: If ROM is an odd number of bytes, it'll be padded by 1 byte.
	uint16_t checksum = 0;
	uint16_t *rom_ptr = &M68K_Mem::Rom_Data.u16[0x200>>1];
	uint16_t *end_ptr = rom_ptr + ((M68K_Mem::Rom_Size - 0x200) >> 1);
	if (M68K_Mem::Rom_Size & 1)
		end_ptr++;
	
	for (; rom_ptr != end_ptr; rom_ptr++)
	{
		checksum += *rom_ptr;
	}
	
	// Set the new checksum.
	M68K_Mem::Rom_Data.u16[0x18E>>1] = checksum;
	return 0;
}


/**
 * restoreChecksum(): Restore the ROM checksum.
 * This restores the ROM checksum in M68K_Mem::Rom_Data[]
 * from the previously-loaded header information.
 * 
 * NOTE: This function may be overridden for e.g. SMS,
 * which uses a different checksum method.
 * 
 * @return 0 on success; non-zero on error.
 */
int EmuContext::restoreChecksum(void)
{
	if (!m_rom || M68K_Mem::Rom_Size <= 0x200)
		return -1;
	
	// Restore the ROM checksum.
	// NOTE: ROM is byteswapped. (Header data is read before calling Rom::loadRom().)
	M68K_Mem::Rom_Data.u16[0x18E>>1] = m_rom->checksum();
	return 0;
}

	
/**
 * saveData(): Save SRam/EEPRom.
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuContext::saveData(void)
{
	// TODO: Move SRam and EEPRom to the Rom class?
	if (m_EEPRom.isEEPRomTypeSet())
	{
		// Save EEPRom.
		int eepromSize = m_EEPRom.save();
		if (eepromSize > 0)
		{
			lg_osd(OSD_EEPROM_SAVE, eepromSize);
			return 2;
		}
	}
	else
	{
		// Save SRam.
		int sramSize = m_SRam.save();
		if (sramSize > 0)
		{
			lg_osd(OSD_SRAM_SAVE, sramSize);
			return 1;
		}
	}
	
	// Nothing was saved.
	return 0;
}


/**
 * autoSaveData(): AutoSave SRam/EEPRom.
 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuContext::autoSaveData(int framesElapsed)
{
	// TODO: Move SRam and EEPRom to the Rom class?
	if (m_EEPRom.isEEPRomTypeSet())
	{
		// Save EEPRom.
		int eepromSize = m_EEPRom.autoSave(framesElapsed);
		if (eepromSize > 0)
		{
			lg_osd(OSD_EEPROM_AUTOSAVE, eepromSize);
			return 2;
		}
	}
	else
	{
		// Save SRam.
		int sramSize = m_SRam.autoSave(framesElapsed);
		if (sramSize > 0)
		{
			lg_osd(OSD_SRAM_AUTOSAVE, sramSize);
			return 1;
		}
	}
	
	// Nothing was saved.
	return 0;
}


/**
 * SetPathSRam(): Set the SRam/EEPRom save path [static]
 * @param newPathSRam New SRam/EEPRom save path.
 */
void EmuContext::SetPathSRam(const char *newPathSRam)
{
	if (!newPathSRam)
		ms_PathSRam.clear();
	else
		ms_PathSRam = string(newPathSRam);
	
	// TODO: Update SRam/EEPRom classes in active contexts.
}

}
