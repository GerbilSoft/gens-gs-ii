/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuMD.cpp: MD emulation code.                                           *
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

#include "EmuMD.hpp"

// VDP I/O and renderer.
#include "VdpIo.hpp"
#include "VdpRend.hpp"
#include "VdpPalette.hpp"

// CPU emulators.
#include "cpu/M68K.cpp"
#include "cpu/Z80.hpp"

// Byteswapping macros and functions.
#include "Util/byteswap.h"

// I/O devices.
#include "IO/IoBase.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// ZOMG
#include "Save/Zomg.hpp"

// LibGens OSD handler.
#include "lg_osd.h"

// C includes.
#include <stdio.h>
#include <math.h>

namespace LibGens
{

// Controllers.
// TODO: Figure out a better place to put these!
IoBase *EmuMD::m_port1 = NULL;	// Player 1.
IoBase *EmuMD::m_port2 = NULL;	// Player 2.
IoBase *EmuMD::m_portE = NULL;	// EXT port.


/**
 * EmuMD::Init(): Initialize EmuMD.
 */
void EmuMD::Init(void)
{
	// Create base I/O devices that do nothing.
	m_port1 = new IoBase();
	m_port2 = new IoBase();
	m_portE = new IoBase();
}


/**
 * EmuMD::End(): Shut down EmuMD.
 */
void EmuMD::End(void)
{
	delete m_port1;
	m_port1 = NULL;
	delete m_port2;
	m_port2 = NULL;
	delete m_portE;
	m_portE = NULL;
}


/**
 * EmuMD::SetRom(): Set the ROM image for MD emulation.
 * NOTE: This function resets the emulated system!
 * @param rom Rom class with the ROM image.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::SetRom(Rom *rom)
{
	// Check the ROM size.
	if ((rom->romSize() == 0) || (rom->romSize() > sizeof(M68K_Mem::Rom_Data)))
	{
		// ROM is either empty or too big.
		// TODO: Error code constants.
		return -1;
	}
	
	// Load the ROM into memory.
	M68K_Mem::Rom_Size = rom->romSize();
	size_t siz_loaded = rom->loadRom(&M68K_Mem::Rom_Data.u8[0], M68K_Mem::Rom_Size);
	if (siz_loaded != M68K_Mem::Rom_Size)
	{
		// Error loading the ROM.
		// TODO: Error code constants.
		return -2;
	}
	
	// Enable SRam/EEPRom by default.
	// TODO: Make accessor/mutator functions.
	M68K_Mem::SaveDataEnable = true;
	
	// Initialize EEPRom.
	// EEPRom is only used if the ROM is in the EEPRom class's database.
	// Otherwise, SRam is used.
	int eepromSize = rom->initEEPRom(&M68K_Mem::m_EEPRom);
	if (eepromSize > 0)
	{
		// EEPRom was initialized.
		lg_osd(OSD_EEPROM_LOAD, eepromSize);
	}
	else
	{
		// EEPRom was not initialized.
		// Initialize SRam.
		int sramSize = rom->initSRam(&M68K_Mem::m_SRam);
		if (sramSize > 0)
			lg_osd(OSD_SRAM_LOAD, sramSize);
	}
	
	// TODO: Byteswapping flags.
	// Until they're implemented, byteswap the ROM *after* initializing SRam/EEPRom.
	be16_to_cpu_array(&M68K_Mem::Rom_Data.u8[0], M68K_Mem::Rom_Size);
	
	// Initialize the VDP.
	VdpIo::Reset();
	
	// Initialize the M68K.
	M68K::InitSys(M68K::SYSID_MD);
	
	// Reinitialize the Z80.
	M68K_Mem::Z80_State = Z80_STATE_ENABLED;	// TODO: "Sound, Z80" setting.
	Z80::ReInit();
	
	// Reset the controller ports.
	m_port1->reset();
	m_port2->reset();
	m_portE->reset();
	
	// TODO: Set these elsewhere.
	M68K_Mem::ms_Region.setRegion(SysRegion::REGION_US_NTSC);
	M68K_Mem::Gen_Mode = 0;		// TODO: This isn't actually used anywhere right now...
	
	// TODO: VdpIo::VDP_Lines.Display.Total isn't being set properly...
	VdpIo::VDP_Lines.Display.Total = (M68K_Mem::ms_Region.isPal() ? 312 : 262);
	VdpIo::Set_Visible_Lines();
	
	// Initialize CPL.
	// TODO: Initialize this somewhere else.
	if (M68K_Mem::ms_Region.isPal())
	{
		M68K_Mem::CPL_M68K = (int)rint((((double)CLOCK_NTSC / 7.0) / 50.0) / 312.0);
		M68K_Mem::CPL_Z80 = (int)rint((((double)CLOCK_NTSC / 15.0) / 50.0) / 312.0);
	}
	else
	{
		M68K_Mem::CPL_M68K = (int)rint((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		M68K_Mem::CPL_Z80 = (int)rint((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
	}
	
	// Initialize audio.
	// NOTE: Only set the region. Sound rate is set by the UI.
	SoundMgr::SetRegion(M68K_Mem::ms_Region.isPal(), false);
	
	return 0;
}


/**
 * EmuMD::SaveData(): Save SRam/EEPRom.
 * @param rom Rom class with the ROM image.
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuMD::SaveData(Rom *rom)
{
	// TODO: Move SRam and EEPRom to the Rom class?
	if (M68K_Mem::m_EEPRom.isEEPRomTypeSet())
	{
		// Save EEPRom.
		int eepromSize = M68K_Mem::m_EEPRom.save();
		if (eepromSize > 0)
		{
			lg_osd(OSD_EEPROM_SAVE, eepromSize);
			return 2;
		}
	}
	else
	{
		// Save SRam.
		int sramSize = M68K_Mem::m_SRam.save();
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
 * EmuMD::AutoSaveData(): AutoSave SRam/EEPRom.
 * @param rom Rom class with the ROM image.
 * @param frames Number of frames elapsed, or -1 for paused (force autosave).
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuMD::AutoSaveData(Rom *rom, int framesElapsed)
{
	// TODO: Move SRam and EEPRom to the Rom class?
	if (M68K_Mem::m_EEPRom.isEEPRomTypeSet())
	{
		// Save EEPRom.
		int eepromSize = M68K_Mem::m_EEPRom.autoSave(framesElapsed);
		if (eepromSize > 0)
		{
			lg_osd(OSD_EEPROM_AUTOSAVE, eepromSize);
			return 2;
		}
	}
	else
	{
		// Save SRam.
		int sramSize = M68K_Mem::m_SRam.autoSave(framesElapsed);
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
 * T_Do_Line(): Run a scanline.
 * @param LineType Line type.
 * @param VDP If true, VDP is updated.
 */
template<EmuMD::LineType_t LineType, bool VDP>
FORCE_INLINE void EmuMD::T_Do_Line(void)
{
	int writePos = SoundMgr::GetWritePos(VdpIo::VDP_Lines.Display.Current);
	int32_t *bufL = &SoundMgr::ms_SegBufL[writePos];
	int32_t *bufR = &SoundMgr::ms_SegBufR[writePos];
	
	// Update the sound chips.
	int writeLen = SoundMgr::GetWriteLen(VdpIo::VDP_Lines.Display.Current);
	SoundMgr::ms_Ym2612.updateDacAndTimers(bufL, bufR, writeLen);
	SoundMgr::ms_Ym2612.addWriteLen(writeLen);
	SoundMgr::ms_Psg.addWriteLen(writeLen);
	
	// Notify controllers that a new scanline is being drawn.
	m_port1->doScanline();
	m_port2->doScanline();
	m_portE->doScanline();
	
	// Increment the cycles counter.
	// These values are the "last cycle to execute".
	// e.g. if Cycles_M68K is 5000, then we'll execute instructions
	// until the 68000's "odometer" reaches 5000.
	M68K_Mem::Cycles_M68K += M68K_Mem::CPL_M68K;
	M68K_Mem::Cycles_Z80 += M68K_Mem::CPL_Z80;
	
	if (VdpIo::DMAT_Length)
		main68k_addCycles(VdpIo::Update_DMA());
	
	switch (LineType)
	{
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			VdpIo::VDP_Status |=  0x0004;	// HBlank = 1
			main68k_exec(M68K_Mem::Cycles_M68K - 404);
			VdpIo::VDP_Status &= ~0x0004;	// HBlank = 0
			
			if (--VdpIo::HInt_Counter < 0)
			{
				VdpIo::VDP_Int |= 0x4;
				VdpIo::Update_IRQ_Line();
				VdpIo::HInt_Counter = VdpIo::VDP_Reg.m5.H_Int;
			}
			
			break;
		
		case LINETYPE_VBLANKLINE:
		{
			// VBlank line!
			if (--VdpIo::HInt_Counter < 0)
			{
				VdpIo::VDP_Int |= 0x4;
				VdpIo::Update_IRQ_Line();
			}
			
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_PRECHECK();
#endif
			VdpIo::VDP_Status |= 0x000C;	// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			if (VdpIo::VDP_Lines.NTSC_V30.VBlank_Div != 0)
				VdpIo::VDP_Status &= ~0x0008;
			
			main68k_exec(M68K_Mem::Cycles_M68K - 360);
			Z80::Exec(168);
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_POSTCHECK();
#endif
			
			VdpIo::VDP_Status &= ~0x0004;	// HBlank = 0
			if (VdpIo::VDP_Lines.NTSC_V30.VBlank_Div == 0)
			{
				VdpIo::VDP_Status |=  0x0080;		// V Int happened
				
				VdpIo::VDP_Int |= 0x8;
				VdpIo::Update_IRQ_Line();
				
				// Z80 interrupt.
				// TODO: Does this trigger on all VBlanks,
				// or only if VINTs are enabled in the VDP?
				Z80::Interrupt(0xFF);
			}
			
			break;
		}
		
		case LINETYPE_BORDER:
		default:
			break;
	}
	
	if (VDP)
	{
		// VDP needs to be updated.
		VdpRend::Render_Line();
	}
	
	main68k_exec(M68K_Mem::Cycles_M68K);
	Z80::Exec(0);
}


/**
 * T_Do_Frame(): Run a frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
FORCE_INLINE void EmuMD::T_Do_Frame(void)
{
	// Initialize VDP_Lines.Display.
	VdpIo::Set_Visible_Lines();
	
	// Check if VBlank is allowed.
	VdpIo::Check_NTSC_V30_VBlank();
	
	// Update I/O devices.
	// TODO: Determine the best place for the I/O devices to be updated:
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	DevManager::Update();	// Update the Device Manager first.
	m_port1->update();
	m_port2->update();
	m_portE->update();
	
	// Reset the sound chip buffer pointers and write length.
	SoundMgr::ResetPtrsAndLens();
	
	// Clear all of the cycle counters.
	M68K_Mem::Cycles_M68K = 0;
	M68K_Mem::Cycles_Z80 = 0;
	M68K_Mem::Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	Z80::ClearOdometer();
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
#endif
	
	// Set the VRam flag to force a VRam update.
	VdpIo::VDP_Flags.VRam = 1;
	
	// Interlaced frame status.
	// Both Interlaced Modes 1 and 2 set this bit on odd frames.
	// This bit is cleared on even frames and if not running in interlaced mode.
	if (VdpIo::VDP_Reg.m5.Set4 & 0x06)
		VdpIo::VDP_Status ^= 0x0010;
	else
		VdpIo::VDP_Status &= ~0x0010;
	
	/** Main execution loops. **/
	
	/** Loop 0: Top border. **/
	/** NOTE: VdpIo::VDP_Lines.Visible.Current may initially be 0! (NTSC V30) **/
	VdpIo::VDP_Lines.Display.Current = 0;
	while (VdpIo::VDP_Lines.Visible.Current < 0)
	{
		T_Do_Line<LINETYPE_BORDER, VDP>();
		
		// Next line.
		VdpIo::VDP_Lines.Display.Current++;
		VdpIo::VDP_Lines.Visible.Current++;
	}
	
	/** Visible line 0. **/
	VdpIo::HInt_Counter = VdpIo::VDP_Reg.m5.H_Int;	// Initialize HInt_Counter.
	VdpIo::VDP_Status &= ~0x0008;			// Clear VBlank status.
	
	/** Loop 1: Active display. **/
	do
	{
		T_Do_Line<LINETYPE_ACTIVEDISPLAY, VDP>();
		
		// Next line.
		VdpIo::VDP_Lines.Display.Current++;
		VdpIo::VDP_Lines.Visible.Current++;
	} while (VdpIo::VDP_Lines.Visible.Current < VdpIo::VDP_Lines.Visible.Total);
	
	/** Loop 2: VBlank line. **/
	T_Do_Line<LINETYPE_VBLANKLINE, VDP>();
	VdpIo::VDP_Lines.Display.Current++;
	VdpIo::VDP_Lines.Visible.Current++;
	
	/** Loop 3: Bottom border. **/
	do
	{
		T_Do_Line<LINETYPE_BORDER, VDP>();
		
		// Next line.
		VdpIo::VDP_Lines.Display.Current++;
		VdpIo::VDP_Lines.Visible.Current++;
	} while (VdpIo::VDP_Lines.Display.Current < VdpIo::VDP_Lines.Display.Total);
	
	// Update the PSG and YM2612 output.
	SoundMgr::SpecialUpdate();
	
#if 0
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
#endif
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	post_frame.width = vdp_getHPix();
	post_frame.height = VDP_Lines.Visible.Total;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	int screen_offset = (TAB336[VDP_Lines.Visible.Border_Size] + 8);
	if (post_frame.width < 320)
		screen_offset += ((320 - post_frame.width) / 2);
	
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen.u32[screen_offset];
	else
		post_frame.md_screen = &MD_Screen.u16[screen_offset];
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
#endif
}

void EmuMD::Do_Frame(void)
{
	T_Do_Frame<true>();
}

void EmuMD::Do_Frame_Fast(void)
{
	T_Do_Frame<false>();
}

}
