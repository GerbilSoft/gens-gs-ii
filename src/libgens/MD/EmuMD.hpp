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

#ifndef __LIBGENS_MD_EMUMD_HPP__
#define __LIBGENS_MD_EMUMD_HPP__

#include "../EmuContext.hpp"

// Needed for FORCE_INLINE.
#include "../macros/common.h"

namespace LibGens
{

class EmuMD : public EmuContext
{
	public:
		EmuMD(Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
		~EmuMD();

		/**
		 * Perform a soft reset.
		 * @return 0 on success; non-zero on error.
		 */
		int softReset(void);

		/**
		 * Perform a hard reset.
		 * @return 0 on success; non-zero on error.
		 */
		int hardReset(void);

		/**
		 * Set the region code.
		 * @param region Region code.
		 * @return 0 on success; non-zero on error.
		 */
		int setRegion(SysVersion::RegionCode_t region);

		/**
		 * Save SRam/EEPRom.
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int saveData(void) final;

		/**
		 * AutoSave SRam/EEPRom.
		 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int autoSaveData(int framesElapsed) final;

		/** Frame execution functions. **/
		void execFrame(void);
		void execFrameFast(void);

	protected:
		/**
		 * Line types.
		 */
		enum LineType_t {
			LINETYPE_ACTIVEDISPLAY	= 0,
			LINETYPE_VBLANKLINE	= 1,
			LINETYPE_BORDER		= 2,
		};

		template<LineType_t LineType, bool VDP>
		FORCE_INLINE void T_execLine(void);

		template<bool VDP>
		FORCE_INLINE void T_execFrame(void);

		/**
		 * Set the region code. (INTERNAL VERSION)
		 * @param region Region code.
		 * @param preserveState If true, preserve the audio IC state.
		 * @return 0 on success; non-zero on error.
		 */
		int setRegion_int(SysVersion::RegionCode_t region, bool preserveState);

		/**
		 * Initialize TMSS.
		 * If TMSS is enabled by the user, this
		 * causes the TMSS ROM to be activated.
		 */
		void initTmss(void);
};

}

#endif /* __LIBGENS_MD_EMUMD_HPP__ */
