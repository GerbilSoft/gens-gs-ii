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

// ZOMG image data.
#include "libzomg/img_data.h"

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
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
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
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int EmuPico::zomgSave(const utf8_str *filename) const
{
	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_SAVE);
	if (!zomg.isOpen())
		return -ENOENT;

	// Rom object has some useful ROM information.
	if (!m_rom)
		return -2;

	// Create ZOMG.ini.
	// TODO: More information...
	LibZomg::Metadata metadata;
	metadata.setSystemId("Pico");
	metadata.setCreator("Gens/GS II");

	// LibGens version.
	// TODO: Add easy "MDP version to string" function.
	char lg_version_str[16];
	snprintf(lg_version_str, sizeof(lg_version_str), "%d.%d.%d",
		(LibGens::version >> 24),
		((LibGens::version >> 16) & 0xFF),
		(LibGens::version & 0xFF));
	metadata.setCreatorVersion(string(lg_version_str));
	if (LibGens::version_vcs)
		metadata.setCreatorVcsVersion(string(LibGens::version_vcs));

	// TODO: Get username for debugging builds. Make this optional later.
	metadata.setAuthor("Joe User");

	// TODO: Move base path triming code to LibGensText later?
	string rom_filename(m_rom->filename());
#ifdef _WIN32
	const char chr_slash = '\\';
#else
	const char chr_slash = '/';
#endif
	size_t slash_pos = rom_filename.find_last_of(chr_slash);
	if (slash_pos != string::npos) {
		if ((slash_pos + 1) <= rom_filename.size() && slash_pos > 0) {
			// Check for another slash.
			slash_pos = rom_filename.find_last_of(chr_slash, slash_pos - 1);
			if (slash_pos != string::npos) {
				// Trim the filename.
				rom_filename = rom_filename.substr(slash_pos + 1);
			}
		} else {
			// Trim the filename.
			rom_filename = rom_filename.substr(slash_pos + 1);
		}
	}
	metadata.setRomFilename(rom_filename);

	// ROM CRC32.
	metadata.setRomCrc32(m_rom->rom_crc32());

	metadata.setDescription("Some description; should probably\nbe left\\blank.");
	metadata.setExtensions("EXT,THAT,DOESNT,EXIST,LOL");

	// Save ZOMG.ini.
	int ret = zomg.saveZomgIni(&metadata);
	if (ret != 0) {
		// Error saving ZOMG.ini.
		return ret;
	}

	// Create the preview image.
	// TODO: Separate function to create an img_data from an MdFb.
	// NOTE: LibZomg doesn't depend on LibGens, so it can't use MdFb directly.
	// TODO: Store VPix and HPixBegin in the MdFb.
	MdFb *fb = m_vdp->MD_Screen->ref();
	const int startY = ((240 - m_vdp->getVPix()) / 2);
	const int startX = (m_vdp->getHPixBegin());

	// TODO: Option to save the full framebuffer, not just active display?
	Zomg_Img_Data_t img_data;
	img_data.w = m_vdp->getHPix();
	img_data.h = m_vdp->getVPix();

	const MdFb::ColorDepth bpp = fb->bpp();
	if (bpp == MdFb::BPP_32) {
		img_data.data = (void*)(fb->lineBuf32(startY) + startX);
		img_data.pitch = (fb->pxPitch() * sizeof(uint32_t));
		img_data.bpp = 32;
	} else {
		img_data.data = (void*)(fb->lineBuf16(startY) + startX);
		img_data.pitch = (fb->pxPitch() * sizeof(uint16_t));
		img_data.bpp = (bpp == MdFb::BPP_16 ? 16 : 15);
	}

	// TODO: Save more metadata.
	zomg.savePreview(&img_data);
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
