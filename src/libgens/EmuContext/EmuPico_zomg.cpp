/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuPico_zomg.cpp: Pico emulation code: ZOMG savestate handler.          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "EmuPico.hpp"
#include "lg_main.hpp"

#include "Vdp/Vdp.hpp"
#include "sound/SoundMgr.hpp"
#include "cpu/M68K_Mem.hpp"
#include "cpu/M68K.hpp"
#include "Cartridge/RomCartridgeMD.hpp"

// ZOMG save structs.
#include "libzomg/Zomg.hpp"
#include "libzomg/Metadata.hpp"
#include "libzomg/zomg_vdp.h"
#include "libzomg/zomg_psg.h"
#include "libzomg/zomg_m68k.h"

// Screenshots.
#include "Util/Screenshot.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <cerrno>

// OS-specific includes.
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

// C++ includes.
#include <string>
using std::string;

// Win32 Unicode Translation Layer.
#ifdef _WIN32
#include "Win32/W32U_mini.h"
#endif

#include "Util/byteswap.h"

namespace LibGens {

/**
 * Load the current state from a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @return 0 on success; negative errno on error.
 */
int EmuPico::zomgLoad(const utf8_str *filename)
{
	// Make sure the file exists.
	if (access(filename, F_OK))
		return -ENOENT;

	// Make sure this is a ZOMG file.
	if (!LibZomg::Zomg::DetectFormat(filename))
		return -EINVAL;

	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_LOAD);
	if (!zomg.isOpen())
		return -EIO;

	// TODO: Check error codes from the ZOMG functions.
	// TODO: Load everything first, *then* copy it to LibGens.

	/** VDP **/
	m_vdp->zomgRestoreMD(&zomg);

	/** Audio **/

	// Load the PSG state.
	Zomg_PsgSave_t psg_save;
	zomg.loadPsgReg(&psg_save);
	SoundMgr::ms_Psg.zomgRestore(&psg_save);

	/** MD: M68K **/

	// Load the M68K memory.
	zomg.loadM68KMem(Ram_68k.u16, sizeof(Ram_68k.u16), ZOMG_BYTEORDER_16H);

	// Load the M68K registers.
	Zomg_M68KRegSave_t m68k_reg_save;
	zomg.loadM68KReg(&m68k_reg_save);
	M68K::ZomgRestoreReg(&m68k_reg_save);

	/* TODO: Pico-specific registers. ($800000) */

	// Load the cartridge data.
	// This includes:
	// - MD /TIME registers. (SRAM control, etc.)
	// - SRAM data.
	// - EEPROM control and data.
	// TODO: Make the 'loadSaveData' parameter user-configurable.
	M68K_Mem::ms_RomCartridge->zomgRestore(&zomg, false);

	// TODO: Load TMSS.
	// Pico TMSS only has one register, the 'SEGA' register.

	// Close the savestate.
	zomg.close();

	// Savestate loaded.
	return 0;
}


/**
 * Save the current state to a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @return 0 on success; negative errno on error.
 */
int EmuPico::zomgSave(const utf8_str *filename) const
{
	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_SAVE);
	if (!zomg.isOpen())
		return -ENOENT;

	// Rom object has some useful ROM information.
	if (!m_rom)
		return -EINVAL;

	// Create ZOMG.ini.
	// TODO: More information...
	LibZomg::Metadata metadata;
	metadata.setSystemId("Pico");
	// TODO: System metadata flags, e.g. save author name.

	// ROM information.
	metadata.setRomFilename(m_rom->filename_base());
	metadata.setRomCrc32(m_rom->rom_crc32());

	// Additional metadata.
	metadata.setDescription("Some description; should probably\nbe left\\blank.");
	// TODO: Remove these fake extensions before release.
	metadata.setExtensions("EXT,THAT,DOESNT,EXIST,LOL");

	// Save ZOMG.ini.
	int ret = zomg.saveZomgIni(&metadata);
	if (ret != 0) {
		// Error saving ZOMG.ini.
		return ret;
	}

	// Create the preview image.
	// TODO: Use the existing metadata?
	// TODO: Check the return value?
	MdFb *fb = m_vdp->MD_Screen->ref();
	Screenshot::toZomg(&zomg, fb, m_rom);
	fb->unref();

	// TODO: Check error codes from the ZOMG functions.
	// TODO: Load everything first, *then* copy it to LibGens.

	/** VDP **/
	m_vdp->zomgSaveMD(&zomg);

	/** Audio **/

	// Save the PSG state.
	Zomg_PsgSave_t psg_save;
	SoundMgr::ms_Psg.zomgSave(&psg_save);
	zomg.savePsgReg(&psg_save);

	/** MD: M68K **/

	// Save the M68K memory.
	zomg.saveM68KMem(Ram_68k.u16, sizeof(Ram_68k.u16), ZOMG_BYTEORDER_16H);

	// Save the M68K registers.
	Zomg_M68KRegSave_t m68k_reg_save;
	M68K::ZomgSaveReg(&m68k_reg_save);
	zomg.saveM68KReg(&m68k_reg_save);

	/* TODO: Pico-specific registers. ($800000) */

	// Save the cartridge data.
	// This includes:
	// - MD /TIME registers. (SRAM control, etc.)
	// - SRAM data.
	// - EEPROM control and data.
	M68K_Mem::ms_RomCartridge->zomgSave(&zomg);

	// TODO: Save TMSS.
	// Pico TMSS only has one register, the 'SEGA' register.

	// Close the savestate.
	zomg.close();
	
	// Savestate saved.
	return 0;
}

}
