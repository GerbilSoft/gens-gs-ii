/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuMD.cpp: MD emulation code: ZOMG savestate handler.                   *
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

#include "EmuMD.hpp"
#include "lg_main.hpp"

#include "Vdp/Vdp.hpp"
#include "sound/SoundMgr.hpp"
#include "cpu/M68K_Mem.hpp"
#include "cpu/M68K.hpp"
#include "cpu/Z80_MD_Mem.hpp"
#include "cpu/Z80.hpp"
#include "Cartridge/RomCartridgeMD.hpp"

// ZOMG save structs.
#include "libzomg/Zomg.hpp"
#include "libzomg/Metadata.hpp"
#include "libzomg/zomg_vdp.h"
#include "libzomg/zomg_psg.h"
#include "libzomg/zomg_ym2612.h"
#include "libzomg/zomg_m68k.h"
#include "libzomg/zomg_z80.h"
#include "libzomg/zomg_md_io.h"
#include "libzomg/zomg_md_z80_ctrl.h"
#include "libzomg/zomg_md_tmss_reg.h"

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

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
#include "libcompat/W32U/W32U_mini.h"
#endif

namespace LibGens {

/**
 * Load the current state from a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @return 0 on success; negative errno on error.
 */
int EmuMD::zomgLoad(const char *filename)
{
	// Make sure the file exists.
	if (access(filename, F_OK))
		return -ENOENT;
	if (access(filename, R_OK))
		return -EACCES;

	// Make sure this is a ZOMG file.
	// TODO: More comprehensive error if the file simply
	// can't be opened instead of being the wrong format?
	// TODO: Better error description for wrong format?
	// (Maybe use MDP error codes instead of POSIX later...)
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

	/** Audio: MD-specific **/

	// Load the YM2612 register state.
	Zomg_Ym2612Save_t ym2612_save;
	zomg.loadMD_YM2612_reg(&ym2612_save);
	SoundMgr::ms_Ym2612.zomgRestore(&ym2612_save);

	/** Z80 **/

	// Load the Z80 memory.
	// TODO: Use the correct size based on system.
	zomg.loadZ80Mem(Ram_Z80, 8192);

	// Load the Z80 registers.
	Zomg_Z80RegSave_t z80_reg_save;
	zomg.loadZ80Reg(&z80_reg_save);
	Z80::ZomgRestoreReg(&z80_reg_save);

	/** MD: M68K **/

	// Load the M68K memory.
	zomg.loadM68KMem(Ram_68k.u16, sizeof(Ram_68k.u16), ZOMG_BYTEORDER_16H);

	// Load the M68K registers.
	Zomg_M68KRegSave_t m68k_reg_save;
	zomg.loadM68KReg(&m68k_reg_save);
	M68K::ZomgRestoreReg(&m68k_reg_save);

	/** MD: Other **/

	// Load the I/O registers. ($A10001-$A1001F, odd bytes)
	// TODO: Create/use the version register function in M68K_Mem.cpp.
	Zomg_MD_IoSave_t md_io_save;
	zomg.loadMD_IO(&md_io_save);
	m_ioManager->zomgRestoreMD(&md_io_save);

	// TODO: Set MD version register.
	//md_io.version_reg = ((M68K_Mem::ms_Region.region() << 6) | 0x20);
	//md_io_save.version_reg = readVersionRegister_MD();

	// Load the Z80 control registers.
	Zomg_MD_Z80CtrlSave_t md_z80_ctrl_save;
	zomg.loadMD_Z80Ctrl(&md_z80_ctrl_save);

	M68K_Mem::Z80_State &= Z80_STATE_ENABLED;
	if (!md_z80_ctrl_save.busreq)
		M68K_Mem::Z80_State |= Z80_STATE_BUSREQ;
	if (!md_z80_ctrl_save.reset)
		M68K_Mem::Z80_State |= Z80_STATE_RESET;
	Z80_MD_Mem::Bank_Z80 = ((md_z80_ctrl_save.m68k_bank & 0x1FF) << 15);

	// Load the cartridge data.
	// This includes:
	// - MD /TIME registers. (SRAM control, etc.)
	// - SRAM data.
	// - EEPROM control and data.
	// TODO: Make the 'loadSaveData' parameter user-configurable.
	M68K_Mem::ms_RomCartridge->zomgRestore(&zomg, false);

	// TODO: Does this need to be loaded before
	// M68K registers are restored?
	if (M68K_Mem::tmss_reg.isTmssEnabled()) {
		// TMSS is enabled.
		// Load the MD TMSS registers.
		Zomg_MD_TMSS_reg_t tmss;
		int ret = zomg.loadMD_TMSS_reg(&tmss);
		if (ret <= 0) {
			// This savestate doesn't have the TMSS registers.
			// Assume TMSS is set up properly.
			M68K_Mem::tmss_reg.a14000.d = 0x53454741; // 'SEGA'
			M68K_Mem::tmss_reg.n_cart_ce = 1;
		} else {
			// Loaded the TMSS registers.
			// TODO: Wordswapping.
			M68K_Mem::tmss_reg.a14000.d = tmss.a14000;
			M68K_Mem::tmss_reg.n_cart_ce = (tmss.n_cart_ce & 1);
		}
		// TODO: Only if cart_ce has changed?
		M68K_Mem::UpdateTmssMapping();
	}

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
int EmuMD::zomgSave(const char *filename) const
{
	// TODO: More comprehensive error reporting.
	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_SAVE);
	if (!zomg.isOpen())
		return -ENOENT;

	// Rom object has some useful ROM information.
	if (!m_rom)
		return -EINVAL;

	// Create ZOMG.ini.
	LibZomg::Metadata metadata;
	metadata.setSystemId("MD");
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

	// TODO: This is MD only!
	// TODO: Check error codes from the ZOMG functions.
	// TODO: Load everything first, *then* copy it to LibGens.
	
	/** VDP **/
	m_vdp->zomgSaveMD(&zomg);
	
	/** Audio **/
	
	// Save the PSG state.
	Zomg_PsgSave_t psg_save;
	SoundMgr::ms_Psg.zomgSave(&psg_save);
	zomg.savePsgReg(&psg_save);
	
	/** Audio: MD-specific **/
	
	// Save the YM2612 register state.
	Zomg_Ym2612Save_t ym2612_save;
	SoundMgr::ms_Ym2612.zomgSave(&ym2612_save);
	zomg.saveMD_YM2612_reg(&ym2612_save);
	
	/** Z80 **/
	
	// Save the Z80 memory.
	// TODO: Use the correct size based on system.
	zomg.saveZ80Mem(Ram_Z80, 8192);
	
	// Save the Z80 registers.
	Zomg_Z80RegSave_t z80_reg_save;
	Z80::ZomgSaveReg(&z80_reg_save);
	zomg.saveZ80Reg(&z80_reg_save);
	
	/** MD: M68K **/
	
	// Save the M68K memory.
	zomg.saveM68KMem(Ram_68k.u16, sizeof(Ram_68k.u16), ZOMG_BYTEORDER_16H);
	
	// Save the M68K registers.
	Zomg_M68KRegSave_t m68k_reg_save;
	M68K::ZomgSaveReg(&m68k_reg_save);
	zomg.saveM68KReg(&m68k_reg_save);
	
	/** MD: Other **/
	
	// Save the I/O registers. ($A10001-$A1001F, odd bytes)
	// TODO: Create/use the version register function in M68K_Mem.cpp.
	Zomg_MD_IoSave_t md_io_save;
	m_ioManager->zomgSaveMD(&md_io_save);
	md_io_save.version_reg = readVersionRegister_MD();
	zomg.saveMD_IO(&md_io_save);

	// Save the Z80 control registers.
	Zomg_MD_Z80CtrlSave_t md_z80_ctrl_save;
	md_z80_ctrl_save.busreq    = !(M68K_Mem::Z80_State & Z80_STATE_BUSREQ);
	md_z80_ctrl_save.reset     = !(M68K_Mem::Z80_State & Z80_STATE_RESET);
	md_z80_ctrl_save.m68k_bank = ((Z80_MD_Mem::Bank_Z80 >> 15) & 0x1FF);
	zomg.saveMD_Z80Ctrl(&md_z80_ctrl_save);
	
	// Save the cartridge data.
	// This includes:
	// - MD /TIME registers. (SRAM control, etc.)
	// - SRAM data.
	// - EEPROM control and data.
	M68K_Mem::ms_RomCartridge->zomgSave(&zomg);

	if (M68K_Mem::tmss_reg.isTmssEnabled()) {
		// TMSS is enabled.
		// Save the MD TMSS registers.
		Zomg_MD_TMSS_reg_t tmss;
		// TODO: Wordswapping.
		tmss.header = ZOMG_MD_TMSS_REG_HEADER;
		tmss.a14000 = M68K_Mem::tmss_reg.a14000.d;
		tmss.n_cart_ce = M68K_Mem::tmss_reg.n_cart_ce & 1;
		zomg.saveMD_TMSS_reg(&tmss);
	} else {
		// TODO: Delete MD/TMSS_reg.bin from the savestate?
	}

	// Close the savestate.
	zomg.close();
	
	// Savestate saved.
	return 0;
}

}
