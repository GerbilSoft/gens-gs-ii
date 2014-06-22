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

// VDP.
#include "Vdp/Vdp.hpp"

// CPU emulators.
#include "cpu/M68K.hpp"
#include "cpu/Z80.hpp"

// Byteswapping macros and functions.
#include "Util/byteswap.h"

// Device Manager.
#include "GensInput/DevManager.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// LibGens OSD handler.
#include "lg_osd.h"

// ROM cartridge.
#include "Cartridge/RomCartridgeMD.hpp"

// C includes.
#include <stdio.h>
#include <math.h>

namespace LibGens
{

/**
 * Initialize a Mega Drive context.
 * @param rom MD ROM.
 * @param region System region.
 */
EmuMD::EmuMD(Rom *rom, SysVersion::RegionCode_t region )
	: EmuContext(rom, region)
{
	// Load the ROM image.
	m_rom = rom;	// NOTE: This is already done in EmuContext::EmuContext()...
	if (!m_rom) {
		// No ROM specified.
		// TODO: Set an error code.
		return;
	}

	// Check the ROM size.
	if ((rom->romSize() == 0) || (rom->romSize() > RomCartridgeMD::MaxRomSize())) {
		// ROM is either empty or too big.
		// TODO: Set an error code.
		m_rom = nullptr;
		return;
	}

	// Load the ROM into memory.
	M68K_Mem::ms_RomCartridge = new RomCartridgeMD(rom);
	M68K_Mem::ms_RomCartridge->loadRom();
	if (!M68K_Mem::ms_RomCartridge->isRomLoaded()) {
		// Error loading the ROM.
		// TODO: Set an error code.
		delete M68K_Mem::ms_RomCartridge;
		M68K_Mem::ms_RomCartridge = nullptr;
		m_rom = nullptr;
		return;
	}

	// Autofix the ROM checksum, if enabled.
	if (AutoFixChecksum())
		M68K_Mem::ms_RomCartridge->fixChecksum();

	// Disable TMSS for now.
	// TODO: Implement proper TMSS loading.
	M68K_Mem::tmss_reg.tmss_en = false;
	M68K_Mem::tmss_reg.tmss_rom_valid = false;

	// Initialize the M68K.
	M68K::InitSys(M68K::SYSID_MD);

	// Reinitialize the Z80.
	// Z80's initial state is RESET.
	M68K_Mem::Z80_State = (Z80_STATE_ENABLED | Z80_STATE_RESET);	// TODO: "Sound, Z80" setting.
	Z80::ReInit();

	// Initialize the system status.
	// TODO: Move Vdp::SysStatus to EmuContext.
	m_vdp->SysStatus.data = 0;
	m_vdp->SysStatus.Genesis = 1;

	// Reset the controllers.
	m_ioManager->reset();

	// Set the system version settings.
	m_sysVersion.setDisk(false);	// No MCD connected.
	setRegion_int(region, false);	// Initialize region code.

	// Finished initializing.
	return;
}


EmuMD::~EmuMD()
{
	// TODO: Other stuff?
	M68K::EndSys();

	// Delete the RomCartridgeMD.
	delete M68K_Mem::ms_RomCartridge;
	M68K_Mem::ms_RomCartridge = nullptr;
}


/**
 * Perform a soft reset.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::softReset(void)
{
	// ROM checksum:
	// - If autofix is enabled, fix the checksum.
	// - If autofix is disabled, restore the checksum.
	if (AutoFixChecksum())
		M68K_Mem::ms_RomCartridge->fixChecksum();
	else
		M68K_Mem::ms_RomCartridge->restoreChecksum();
	
	// Reset the M68K, Z80, and YM2612.
	M68K::Reset();
	Z80::SoftReset();
	SoundMgr::ms_Ym2612.reset();
	
	// Z80 state should be reset to the default value.
	// Z80's initial state is RESET.
	M68K_Mem::Z80_State = (Z80_STATE_ENABLED | Z80_STATE_RESET);	// TODO: "Sound, Z80" setting.
	
	// TODO: Genesis Plus randomizes the restart line.
	// See genesis.c:176.
	return 0;
}


/**
 * Perform a hard reset.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::hardReset(void)
{
	// Reset the controllers.
	m_ioManager->reset();

	// ROM checksum:
	// - If autofix is enabled, fix the checksum.
	// - If autofix is disabled, restore the checksum.
	if (AutoFixChecksum())
		M68K_Mem::ms_RomCartridge->fixChecksum();
	else
		M68K_Mem::ms_RomCartridge->restoreChecksum();

	// Hard-Reset the M68K, Z80, VDP, PSG, and YM2612.
	// This includes clearing RAM.
	M68K::InitSys(M68K::SYSID_MD);
	Z80::ReInit();
	m_vdp->reset();
	SoundMgr::ms_Psg.reset();
	SoundMgr::ms_Ym2612.reset();

	// Make sure the VDP's video mode bit is set properly.
	m_vdp->setVideoMode(m_sysVersion.isPal());

	// Reset successful.
	return 0;
}


/**
 * Set the region code.
 * @param region Region code.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::setRegion(SysVersion::RegionCode_t region)
	{ return setRegion_int(region, true); }

/**
 * Gens rounding function.
 * The implementation doesn't match rint(), so we're defining this here.
 * @param val Value to round.
 * @return Rounded value.
 */
static inline int Round_Double(double val)
{
	if ((val - (double)(int)val) > 0.5)
		return (int)val + 1;
	else
		return (int)val;
}

/**
 * Set the region code. (INTERNAL VERSION)
 * @param region Region code.
 * @param preserveState If true, preserve the audio IC state.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::setRegion_int(SysVersion::RegionCode_t region, bool preserveState)
{
	SysVersion newRegion(region);
	if (preserveState && (m_sysVersion.isPal() == newRegion.isPal())) {
		// preserveState was specified, and the current NTSC/PAL setting
		// matches the new NTSC/PAL setting. Don't reset anything.
		m_sysVersion.setRegion(region);
		return 0;
	}

	// Set the region.
	m_sysVersion.setRegion(region);

	// Initialize Vdp::VDP_Lines.
	// Don't reset the VDP current line variables here,
	// since this might not be the beginning of the frame.
	m_vdp->updateVdpLines(false);
	m_vdp->setVideoMode(m_sysVersion.isPal());

	// Initialize CPL.
	/* NOTE: Game_Music_Emu uses floor() here, but it seems that using floor()
	 * causes audio distortion on the title screen of "Beavis and Butt-head" (U).
	 * Use the old "Round_Double" implementation like in old Gens.
	 * [rint() uses banker's rounding, which rounds 0.5 to 0 and 1.5 to 2.]
	 * [Round_Double() rounds 0.5 to 0 and 1.5 to 1.] */
	// TODO: Jorge says CPL is always 3420 master clock cycles...
	if (m_sysVersion.isPal()) {
		M68K_Mem::CPL_M68K = Round_Double((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		M68K_Mem::CPL_Z80 = Round_Double((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
	} else {
		M68K_Mem::CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		M68K_Mem::CPL_Z80 = Round_Double((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
	}

	// Initialize audio.
	// NOTE: Only set the region. Sound rate is set by the UI.
	SoundMgr::SetRegion(m_sysVersion.isPal(), preserveState);

	// Region set successfully.
	return 0;
}


/**
 * Save SRam/EEPRom.
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuMD::saveData(void)
{
	// TODO: Call lg_osd here instead of in RomCartridgeMD().
	if (M68K_Mem::ms_RomCartridge)
		return M68K_Mem::ms_RomCartridge->saveData();

	// Nothing was saved.
	return 0;
}

/**
 * AutoSave SRam/EEPRom.
 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuMD::autoSaveData(int framesElapsed)
{
	// TODO: Call lg_osd here instead of in RomCartridgeMD().
	if (M68K_Mem::ms_RomCartridge)
		return M68K_Mem::ms_RomCartridge->autoSaveData(framesElapsed);

	// Nothing was saved.
	return 0;
}

/**
 * Run a scanline.
 * @param LineType Line type.
 * @param VDP If true, VDP is updated.
 */
template<EmuMD::LineType_t LineType, bool VDP>
FORCE_INLINE void EmuMD::T_execLine(void)
{
	int writePos = SoundMgr::GetWritePos(m_vdp->VDP_Lines.currentLine);
	int32_t *bufL = &SoundMgr::ms_SegBufL[writePos];
	int32_t *bufR = &SoundMgr::ms_SegBufR[writePos];

	// Update the sound chips.
	int writeLen = SoundMgr::GetWriteLen(m_vdp->VDP_Lines.currentLine);
	SoundMgr::ms_Ym2612.updateDacAndTimers(bufL, bufR, writeLen);
	SoundMgr::ms_Ym2612.addWriteLen(writeLen);
	SoundMgr::ms_Psg.addWriteLen(writeLen);

	// Notify controllers that a new scanline is being drawn.
	m_ioManager->doScanline();

	// Increment the cycles counter.
	// These values are the "last cycle to execute".
	// e.g. if Cycles_M68K is 5000, then we'll execute instructions
	// until the 68000's "odometer" reaches 5000.
	M68K_Mem::Cycles_M68K += M68K_Mem::CPL_M68K;
	M68K_Mem::Cycles_Z80 += M68K_Mem::CPL_Z80;

	if (m_vdp->DMAT_Length)
		M68K::AddCycles(m_vdp->Update_DMA());

	switch (LineType) {
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_HBLANK, true);	// HBlank = 1
			M68K::Exec(M68K_Mem::Cycles_M68K - 404);
			m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_HBLANK, false);	// HBlank = 0

			if (--m_vdp->HInt_Counter < 0) {
				m_vdp->VDP_Int |= 0x4;
				m_vdp->Update_IRQ_Line();
				m_vdp->HInt_Counter = m_vdp->VDP_Reg.m5.H_Int;
			}

			break;

		case LINETYPE_VBLANKLINE: {
			// VBlank line!
			if (--m_vdp->HInt_Counter < 0) {
				m_vdp->VDP_Int |= 0x4;
				m_vdp->Update_IRQ_Line();
			}

#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_PRECHECK();
#endif
			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_HBLANK, true);
			m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_VBLANK, true);

			// If we're using NTSC V30 and this is an "even" frame,
			// don't set the VBlank flag.
			if (m_vdp->VDP_Lines.NTSC_V30.VBlank_Div != 0)
				m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_VBLANK, false);

			M68K::Exec(M68K_Mem::Cycles_M68K - 360);
			Z80::Exec(168);
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_POSTCHECK();
#endif

			m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_HBLANK, false);	// HBlank = 0
			if (m_vdp->VDP_Lines.NTSC_V30.VBlank_Div == 0) {
				m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_F, true);	// V Int happened

				m_vdp->VDP_Int |= 0x8;
				m_vdp->Update_IRQ_Line();

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

	if (VDP) {
		// VDP needs to be updated.
		m_vdp->Render_Line();
	}

	M68K::Exec(M68K_Mem::Cycles_M68K);
	Z80::Exec(0);
}

/**
 * T_execFrame(): Run a frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
FORCE_INLINE void EmuMD::T_execFrame(void)
{
	// Initialize Vdp::VDP_Lines.
	// Reset the current VDP line variables for the new frame.
	m_vdp->updateVdpLines(true);

	// Check if VBlank is allowed.
	m_vdp->Check_NTSC_V30_VBlank();

	// Update I/O devices.
	// TODO: Determine the best place for the I/O devices to be updated:
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	DevManager::Update();	// Update the Device Manager first.
	m_ioManager->update();

	// Reset the sound chip buffer pointers and write length.
	SoundMgr::ResetPtrsAndLens();

	// Clear all of the cycle counters.
	M68K_Mem::Cycles_M68K = 0;
	M68K_Mem::Cycles_Z80 = 0;
	M68K_Mem::Last_BUS_REQ_Cnt = -1000;
	M68K::TripOdometer();
	Z80::ClearOdometer();

	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, nullptr);
#endif

	// Set the VRam flag to force a VRam update.
	m_vdp->MarkVRamDirty();

	// Interlaced frame status.
	// Both Interlaced Modes 1 and 2 set this bit on odd frames.
	// This bit is cleared on even frames and if not running in interlaced mode.
	if (m_vdp->VDP_Reg.m5.Set4 & 0x06)
		m_vdp->Reg_Status.toggleBit(VdpStatus::VDP_STATUS_ODD);
	else
		m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_ODD, false);

	/** Main execution loops. **/

	/** Visible line 0. **/
	m_vdp->VDP_Lines.currentLine = 0;
	m_vdp->HInt_Counter = m_vdp->VDP_Reg.m5.H_Int;			// Initialize HInt_Counter.
	m_vdp->Reg_Status.setBit(VdpStatus::VDP_STATUS_VBLANK, false);	// Clear VBlank flag.

	/** Loop 1: Active display. **/
	do {
		T_execLine<LINETYPE_ACTIVEDISPLAY, VDP>();

		// Next line.
		m_vdp->VDP_Lines.currentLine++;
	} while (m_vdp->VDP_Lines.currentLine < m_vdp->VDP_Lines.totalVisibleLines);

	/** Loop 2: VBlank line. **/
	T_execLine<LINETYPE_VBLANKLINE, VDP>();
	m_vdp->VDP_Lines.currentLine++;

	/** Loop 3: Borders. **/
	do {
		T_execLine<LINETYPE_BORDER, VDP>();

		// Next line.
		m_vdp->VDP_Lines.currentLine++;
	} while (m_vdp->VDP_Lines.currentLine < m_vdp->VDP_Lines.totalDisplayLines);

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

void EmuMD::execFrame(void)
{
	T_execFrame<true>();
}

void EmuMD::execFrameFast(void)
{
	T_execFrame<false>();
}

}
