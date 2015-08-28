/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuPico.cpp: Pico emulation code.                                       *
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

// Pico is nearly identical to MD, but with the following differences:
// - No Z80 or YM2612.
// - NEC uPD7759 ADPCM chip.
// - $A00000 I/O range is gone.
// - $800000 I/O range with Pico-specific hardware.

#include "EmuPico.hpp"

#include "Cartridge/RomCartridgeMD.hpp"
#include "Vdp/Vdp.hpp"
#include "cpu/M68K.hpp"
#include "cpu/M68K_Mem.hpp"
#include "sound/SoundMgr.hpp"

// LibGens OSD handler.
#include "lg_osd.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cmath>

// C++ namespace.
#include <memory>
using std::auto_ptr;

namespace LibGens {

/**
 * Initialize a Mega Drive context.
 * @param rom Pico ROM.
 * @param region System region.
 */
EmuPico::EmuPico(Rom *rom, SysVersion::RegionCode_t region )
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

	// Initialize the M68K.
	M68K::InitSys(M68K::SYSID_PICO);

	// Initialize the system status.
	// TODO: Move Vdp::SysStatus to EmuContext.
	m_vdp->SysStatus.data = 0;
	m_vdp->SysStatus.Genesis = 1;

	// Pico doesn't use MD-style TMSS.
	M68K_Mem::tmss_reg.clearTmssRom();

	// Reset the controllers.
	m_ioManager->reset();

	// Set the system version settings.
	// TODO: Update for Pico?
	m_sysVersion.setDisk(false);	// No MCD connected.
	setRegion_int(region, false);	// Initialize region code.

	// Finished initializing.
	return;
}

EmuPico::~EmuPico()
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
int EmuPico::softReset(void)
{
	// ROM checksum:
	// - If autofix is enabled, fix the checksum.
	// - If autofix is disabled, restore the checksum.
	if (AutoFixChecksum())
		M68K_Mem::ms_RomCartridge->fixChecksum();
	else
		M68K_Mem::ms_RomCartridge->restoreChecksum();

	// Reset the M68K.
	M68K::Reset();

	// TODO: Genesis Plus randomizes the restart line.
	// See genesis.c:176.
	return 0;
}

/**
 * Perform a hard reset.
 * @return 0 on success; non-zero on error.
 */
int EmuPico::hardReset(void)
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
	M68K::InitSys(M68K::SYSID_PICO);
	SoundMgr::ms_Psg.reset();

	// Reset the VDP.
	m_vdp->reset();
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
int EmuPico::setRegion(SysVersion::RegionCode_t region)
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
int EmuPico::setRegion_int(SysVersion::RegionCode_t region, bool preserveState)
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
	} else {
		M68K_Mem::CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
	}

	// No Z80 here...
	M68K_Mem::CPL_Z80 = 0;

	// Initialize audio.
	// NOTE: Only set the region. Sound rate is set by the UI.
	// TODO: Don't initialize YM2612?
	SoundMgr::SetRegion(m_sysVersion.isPal(), preserveState);

	// Region set successfully.
	return 0;
}

/**
 * Save SRam/EEPRom.
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int EmuPico::saveData(void)
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
int EmuPico::autoSaveData(int framesElapsed)
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
template<EmuPico::LineType_t LineType, bool VDP>
FORCE_INLINE void EmuPico::T_execLine(void)
{
	// Update the sound chips.
	int writeLen = SoundMgr::GetWriteLen(m_vdp->VDP_Lines.currentLine);
	SoundMgr::ms_Psg.addWriteLen(writeLen);

	// Notify controllers that a new scanline is being drawn.
	m_ioManager->doScanline();

	// Increment the cycles counter.
	// These values are the "last cycle to execute".
	// e.g. if Cycles_M68K is 5000, then we'll execute instructions
	// until the 68000's "odometer" reaches 5000.
	M68K_Mem::Cycles_M68K += M68K_Mem::CPL_M68K;

	if (m_vdp->DMAT_Length)
		M68K::AddCycles(m_vdp->updateDMA());

	switch (LineType) {
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			m_vdp->setStatusBit(VdpStatus::VDP_STATUS_HBLANK, true);	// HBlank = 1
			M68K::Exec(M68K_Mem::Cycles_M68K - 404);
			m_vdp->setStatusBit(VdpStatus::VDP_STATUS_HBLANK, false);	// HBlank = 0

			// Decrement the HInt counter.
			// If it goes below 0, an HBLANK interrupt will occur.
			// The counter will then be reloaded.
			m_vdp->decrementHIntCounter(true);
			break;

		case LINETYPE_VBLANKLINE: {
			// VBlank line!
			// Decrement the HInt counter.
			// If it goes below 0, an HBLANK interrupt will occur.
			m_vdp->decrementHIntCounter(false);

#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_PRECHECK();
#endif
			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			m_vdp->setStatusBit(VdpStatus::VDP_STATUS_HBLANK, true);
			m_vdp->setStatusBit(VdpStatus::VDP_STATUS_VBLANK, true);

			// If we're using NTSC V30 and this is an "even" frame,
			// don't set the VBlank flag.
			if (m_vdp->VDP_Lines.NTSC_V30.VBlank_Div != 0)
				m_vdp->setStatusBit(VdpStatus::VDP_STATUS_VBLANK, false);

			M68K::Exec(M68K_Mem::Cycles_M68K - 360);
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_POSTCHECK();
#endif

			m_vdp->setStatusBit(VdpStatus::VDP_STATUS_HBLANK, false);	// HBlank = 0
			if (m_vdp->VDP_Lines.NTSC_V30.VBlank_Div == 0) {
				m_vdp->setStatusBit(VdpStatus::VDP_STATUS_F, true);	// V Int happened
				m_vdp->updateIRQLine(0x8);
			}

			break;
		}

		case LINETYPE_BORDER:
		default:
			break;
	}

	if (VDP) {
		// VDP needs to be updated.
		m_vdp->renderLine();
	}

	M68K::Exec(M68K_Mem::Cycles_M68K);
}

/**
 * T_execFrame(): Run a frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
FORCE_INLINE void EmuPico::T_execFrame(void)
{
	// Initialize Vdp::VDP_Lines.
	// Reset the current VDP line variables for the new frame.
	m_vdp->updateVdpLines(true);

	// Check if VBlank is allowed.
	m_vdp->Check_NTSC_V30_VBlank();

	// NOTE: I/O devices must be updated by the UI.
	//m_ioManager->update();

	// Reset the sound chip buffer pointers and write length.
	SoundMgr::ResetPtrsAndLens();

	// Clear all of the cycle counters.
	M68K_Mem::Cycles_M68K = 0;
	M68K_Mem::Cycles_Z80 = 0;
	M68K_Mem::Last_BUS_REQ_Cnt = -1000;
	M68K::TripOdometer();

	// TODO: MDP . (LibGens)
#if 0
	// Raise the MDP _EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP _EVENT_PRE_FRAME, nullptr);
#endif

	// Start the frame.
	// This initializes the "Interlaced" flag as well as
	// the HINT counter, and clears the VBLANK flag.
	m_vdp->startFrame();

	/** Main execution loops. **/

	/** Visible line 0. **/
	m_vdp->VDP_Lines.currentLine = 0;

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

	// TODO: MDP . (LibGens)
#if 0
	// Raise the MDP _EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	post_frame.width = vdp_getHPix();
	post_frame.height = VDP_Lines.Visible.Total;
	post_frame.pitch = 336;
	post_frame.bpp = bppPico;

	int screen_offset = (TAB336[VDP_Lines.Visible.Border_Size] + 8);
	if (post_frame.width < 320)
		screen_offset += ((320 - post_frame.width) / 2);

	if (bppPico == 32)
		post_frame.md_screen = &Pico_Screen.u32[screen_offset];
	else
		post_frame.md_screen = &Pico_Screen.u16[screen_offset];

	EventMgr::RaiseEvent(MDP _EVENT_POST_FRAME, &post_frame);
#endif
}

void EmuPico::execFrame(void)
{
	T_execFrame<true>();
}

void EmuPico::execFrameFast(void)
{
	T_execFrame<false>();
}

}
