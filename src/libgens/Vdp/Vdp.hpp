/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Vdp.hpp: VDP emulation class.                                           *
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

#ifndef __LIBGENS_MD_VDP_HPP__
#define __LIBGENS_MD_VDP_HPP__

#include <stdint.h>

// VdpRend includes.
#include "../Util/MdFb.hpp"

// Needed for FORCE_INLINE.
#include "../macros/common.h"

// VDP types.
#include "VdpTypes.hpp"
#include "VdpStatus.hpp"
#include "../Util/MdFb.hpp"
#include "VdpPalette.hpp"

namespace LibZomg {
	class Zomg;
}

namespace LibGens {

class VdpPrivate;
class Vdp
{
	public:
		// TODO: Remove MdFb.
		Vdp(MdFb *fb = nullptr);
		~Vdp();

	protected:
		friend class VdpPrivate;
		VdpPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Vdp(const Vdp &);
		Vdp &operator=(const Vdp &);

	public:
		/**
		 * Reset the VDP.
		 */
		void reset(void);

	public:
		/** VDP settings. **/

		// Video mode. (PAL/NTSC)
		bool isPal(void) const;
		bool isNtsc(void) const;
		void setPal(void);
		void setNtsc(void);
		void setVideoMode(bool videoMode);

		// Display resolution.
		// Required by the frontend for proper rendering.
		// Also used by VdpRend_Err.
		int getHPix(void) const;
		int getHPixBegin(void) const;
		int getVPix(void) const;

		// VDP emulation options.
		// TODO: Make private, and add accessor/mutator?
		VdpTypes::VdpEmuOptions_t options;

	public:
		/** Main VDP functions. **/
		uint8_t Int_Ack(void);

		/**
		 * Update the IRQ line.
		 * @param interrupt Interrupt that just occurred. (If 0, no interrupt occurred.)
		 */
		void updateIRQLine(int interrupt);

		// NOTE: Replace updateVdpLines() with internal function
		// called on init, video mode change, and graphics mode change.
		void updateVdpLines(bool resetCurrent);
		// TODO: Move this to VdpPrivate?
		void Check_NTSC_V30_VBlank(void);

		/**
		 * Start of frame.
		 * This updates the "Interlaced" flag and HINT counter,
		 * and clears the VBLANK flag.
		 *
		 * "Interlaced" flag behavior: (VDP_STATUS_ODD)
		 * - If interlaced mode is not set, the flag is cleared.
		 * - If interlaced mode is set, the flag is toggled.
		 */
		void startFrame(void);

		/**
		 * Initialize the HINT counter.
		 * This also clears the VBLANK flag.
		 */
		void initHIntCounter(void);

		/**
		 * Decrement the HINT counter.
		 * If it goes below 0, an HBLANK interrupt will occur.
		 * @param reload If true, reload the HINT counter afterwards.
		 */
		void decrementHIntCounter(bool reload);

		/**
		 * Set a bit in the status register.
		 * Wrapper for VdpStatus::setBit().
		 * Needed because the emulation loops toggle HBLANK/VBLANK.
		 * @param bit Status bit to set.
		 * @param value New value.
		 */
		void setStatusBit(VdpStatus::StatusBits bit, bool value);

		// TODO: Functions for VDP lines? (inc, check in vblank, etc)

		/**
		 * Update the DMA state.
		 * @return Number of cycles taken from the 68000 for DMA.
		 */
		unsigned int updateDMA(void);

		/**
		 * Render the current line to the framebuffer.
		 */
		void renderLine(void);

	public:
		/** MD-side interface. **/
		// NOTE: Byte-wide MD ctrl/data functions are
		// merely wrappers around the 16-bit functions.

		// Data port: $C00000 [mirror: 06]
		uint16_t readDataMD(void);
		void writeDataMD(uint16_t data);
		void writeDataMD_8(uint8_t data);

		// Control port; $C00004 [mirror: 06]
		uint16_t readCtrlMD(void);	// status register
		void writeCtrlMD(uint16_t ctrl);
		void writeCtrlMD_8(uint8_t ctrl);

		// H/V counter: $C00008 [mirrors: 0A, 0C, 0E]
		uint16_t readHVCounterMD(void);	// convenience function
		uint8_t readHCounter(void);
		uint8_t readVCounter(void);

		// PSG: $C00011 [mirrors: 13, 15, 17]
		// Not implemented here.

		/**
		 * Test register: $C0001C [mirrors: 1E]
		 * TODO
		 * From http://md.squee.co/wiki/VDP#Debug_Register
		 * [   ? SHDE2 SHDE1 SHDE0 CLICK CLICK PSGEN BGHDE] (15..8)
		 * [SDBG  DISP CORR1     ? KILL1     ?     ?     ?] (7..0)
		 * - SHDE: Hides sprites on the screen.
		 * - CLICK: When toggled, a click can be heard in the audio.
		 * - PSGEN: When set, PSG is disabled.
		 * - BGHDE: When BG layer is hidden, dots appear and sprites are hidden.
		 * - SDBG: When set, the VDP draws a black box around sprites.
		 * - DISP: When set, display can be disabled. (Overrides Reg.#1)
		 * - CORR1: When set, corrupt data is displayed, and may corrupt internal VDP state.
		 * - KILL1: When set, causes streaks down the screen as if there's continuous 68k -> CRAM DMA.
		 */
		uint16_t readTestRegMD(void) const;
		void writeTestRegMD(uint16_t data);
		void writeTestRegMD_8(uint8_t data);

	public:
		/** ZOMG savestate functions. **/

		/**
		 * Save the VDP state. (MD mode)
		 * @param zomg ZOMG savestate object to save to.
		 */
		void zomgSaveMD(LibZomg::Zomg *zomg) const;

		/**
		 * Restore the VDP state. (MD mode)
		 * @param zomg ZOMG savestate object to restore from.
		 */
		void zomgRestoreMD(LibZomg::Zomg *zomg);

	public:
		// TODO: Move to private class.
		int DMAT_Length;

		// MD framebuffer.
		// TODO: Remove this; instead, have a parameter
		// passed to renderLine().
		MdFb *MD_Screen;

		// FIXME: Bpp should be a property of the framebuffer.
		// Currently needed by GensQt4::EmuManager::closeRom()
		// in order to show the "intro effect".
		// Also for updating the VBackend.
		VdpPalette::ColorDepth bpp(void) const;
		void setBpp(VdpPalette::ColorDepth bpp);

		// VDP line counters.
		// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
		VdpTypes::VdpLines_t VDP_Lines;

		// System status.
		// TODO: Move this to a more relevant file.
		union {
			struct {
				unsigned int Genesis	:1;
				unsigned int SegaCD	:1;
				unsigned int _32X	:1;
			};
			unsigned int data;
		} SysStatus;
};

}

#endif /* __LIBGENS_MD_VDP_HPP__ */
