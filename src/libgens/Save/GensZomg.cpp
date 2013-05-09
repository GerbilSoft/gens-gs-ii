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

#include "GensZomg.hpp"
#include "lg_main.hpp"

#include "Vdp/Vdp.hpp"
#include "sound/SoundMgr.hpp"
#include "cpu/M68K_Mem.hpp"
#include "cpu/M68K.hpp"
#include "cpu/Z80_MD_Mem.hpp"
#include "cpu/Z80.hpp"
#include "MD/EmuMD.hpp"

// ZOMG save structs.
#include "libzomg/Zomg.hpp"
#include "libzomg/ZomgIni.hpp"
#include "libzomg/zomg_vdp.h"
#include "libzomg/zomg_psg.h"
#include "libzomg/zomg_ym2612.h"
#include "libzomg/zomg_m68k.h"
#include "libzomg/zomg_z80.h"
#include "libzomg/zomg_md_io.h"
#include "libzomg/zomg_md_z80_ctrl.h"

// C includes.
#include <stdint.h>
#include <unistd.h>

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

// C++ includes.
#include <string>
using std::string;

namespace LibGens
{

/**
 * Load the current state from a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @param context	[out] Emulation context.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int ZomgLoad(const utf8_str *filename, EmuContext *context)
{
	// Make sure the file exists.
	if (access(filename, F_OK))
		return -1;

	// Make sure this is a ZOMG file.
	if (!LibZomg::Zomg::DetectFormat(filename))
		return -2;

	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_LOAD);
	if (!zomg.isOpen())
		return -3;

	// TODO: This is MD only!
	// TODO: Check error codes from the ZOMG functions.
	// TODO: Load everything first, *then* copy it to LibGens.

	/** VDP **/
	context->m_vdp->zomgRestoreMD(&zomg);

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
	context->m_ioManager->zomgRestoreMD(&md_io_save);

	// TODO: Set MD version register.
	//md_io.version_reg = ((M68K_Mem::ms_Region.region() << 6) | 0x20);
	//md_io_save.version_reg = context->readVersionRegister_MD();

	// Load the Z80 control registers.
	Zomg_MD_Z80CtrlSave_t md_z80_ctrl_save;
	zomg.loadMD_Z80Ctrl(&md_z80_ctrl_save);

	M68K_Mem::Z80_State &= Z80_STATE_ENABLED;
	if (!md_z80_ctrl_save.busreq)
		M68K_Mem::Z80_State |= Z80_STATE_BUSREQ;
	if (!md_z80_ctrl_save.reset)
		M68K_Mem::Z80_State |= Z80_STATE_RESET;
	Z80_MD_Mem::Bank_Z80 = ((md_z80_ctrl_save.m68k_bank & 0x1FF) << 15);

	// Load the MD /TIME registers.
	Zomg_MD_TimeReg_t md_time_reg_save;
	int ret = zomg.loadMD_TimeReg(&md_time_reg_save);

	EEPRom *eeprom = context->getEEPRom();
	if (!eeprom->isEEPRomTypeSet()) {
		// EEPRom is disabled. Use SRam.
		// Load SRam control registers from the /TIME register bank.
		SRam *sram = context->getSRam();
		if (ret <= 0xF1) {
			// SRAM control register wasn't present.
			// If the ROM is less than 2 MB, force SRAM access on, write-enabled.
			// Otherwise, set SRAM off, write-protected.
			// TODO: Save a flag somewhere to indicate that this should be set
			// instead of checking M68K_Mem::Rom_Size.
			if (M68K_Mem::Rom_Size < 0x200000)
				sram->writeCtrl(1);
			else
				sram->writeCtrl(2);
		} else {
			// SRAM control register was present.
			// Write the value from the savestate.
			sram->writeCtrl(md_time_reg_save.SRAM_ctrl);
		}

		// Load SRAM.
		// TODO: Make this optional.
		sram->loadFromZomg(zomg);
	}

	// Load SSF2 bank registers.
	// TODO: Only if SSF2 is detected?
	M68K_Mem::ZomgRestoreSSF2BankState(&md_time_reg_save);

	// Close the savestate.
	zomg.close();

	// Savestate loaded.
	return 0;
}


/**
 * ZomgSave(): Save the current state to a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @param context	[in] Emulation context.
 * @param img_buf	[in, opt] Buffer containing PNG image for the ZOMG preview image.
 * @param img_siz	[in, opt] Size of img_buf.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int ZomgSave(const utf8_str *filename, const EmuContext *context,
	     const void *img_buf, size_t img_siz)
{
	LibZomg::Zomg zomg(filename, LibZomg::Zomg::ZOMG_SAVE);
	if (!zomg.isOpen())
		return -1;

	// Create ZOMG.ini.
	// TODO: Get System ID and Region from the emulated system information.
	LibZomg::ZomgIni zomgIni;
	zomgIni.setSystemId("MD");
	zomgIni.setCreator("Gens/GS II");

	// LibGens version.
	// TODO: Add easy "MDP version to string" function.
	char lg_version_str[16];
	snprintf(lg_version_str, sizeof(lg_version_str), "%d.%d.%d",
		(LibGens::version >> 24),
		((LibGens::version >> 16) & 0xFF),
		(LibGens::version & 0xFF));
	zomgIni.setCreatorVersion(string(lg_version_str));
	if (LibGens::version_vcs)
		zomgIni.setCreatorVcsVersion(string(LibGens::version_vcs));

	// TODO: Get username for debugging builds. Make this optional later.
	zomgIni.setAuthor("Joe User");

	// TODO: Move base path triming code to LibGensText later?
	string rom_filename(context->rom()->filename());
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
	zomgIni.setRomFilename(rom_filename);

	zomgIni.setDescription("Some description; should probably\nbe left\\blank.");
	zomgIni.setExtensions("EXT,THAT,DOESNT,EXIST,LOL");

	// Save ZOMG.ini.
	int ret = zomg.saveZomgIni(&zomgIni);
	if (ret != 0) {
		// Error saving ZOMG.ini.
		return ret;
	}

	// If a preview image was specified, save it.
	if (img_buf && img_siz > 0)
		zomg.savePreview(img_buf, img_siz);
	
	// TODO: This is MD only!
	// TODO: Check error codes from the ZOMG functions.
	// TODO: Load everything first, *then* copy it to LibGens.
	
	/** VDP **/
	context->m_vdp->zomgSaveMD(&zomg);
	
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
	context->m_ioManager->zomgSaveMD(&md_io_save);
	md_io_save.version_reg = context->readVersionRegister_MD();
	zomg.saveMD_IO(&md_io_save);

	// Save the Z80 control registers.
	Zomg_MD_Z80CtrlSave_t md_z80_ctrl_save;
	md_z80_ctrl_save.busreq    = !(M68K_Mem::Z80_State & Z80_STATE_BUSREQ);
	md_z80_ctrl_save.reset     = !(M68K_Mem::Z80_State & Z80_STATE_RESET);
	md_z80_ctrl_save.m68k_bank = ((Z80_MD_Mem::Bank_Z80 >> 15) & 0x1FF);
	zomg.saveMD_Z80Ctrl(&md_z80_ctrl_save);
	
	// Save the MD /TIME registers.
	Zomg_MD_TimeReg_t md_time_reg_save;
	memset(md_time_reg_save.reg, 0xFF, sizeof(md_time_reg_save.reg));
	
	// SRam control registers.
	const EEPRom *eeprom = context->getEEPRom();
	if (!eeprom->isEEPRomTypeSet())
	{
		// EEPRom is disabled. Use SRam.
		// Save SRam control registers to the /TIME register bank.
		const SRam *sram = context->getSRam();
		md_time_reg_save.SRAM_ctrl = sram->zomgReadCtrl();
		
		// Save SRAM.
		// TODO: Make this optional.
		sram->saveToZomg(zomg);
	}
	
	// Save SSF2 bank registers.
	// TODO: Only if SSF2 is detected?
	M68K_Mem::ZomgSaveSSF2BankState(&md_time_reg_save);
	
	// Write MD /TIME registers.
	zomg.saveMD_TimeReg(&md_time_reg_save);
	
	// Close the savestate.
	zomg.close();
	
	// Savestate saved.
	return 0;
}

}
