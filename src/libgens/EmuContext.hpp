/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContext.hpp: Emulation context base class.                           *
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

#ifndef __LIBGENS_EMUCONTEXT_HPP__
#define __LIBGENS_EMUCONTEXT_HPP__

// Controllers.
// TODO: Figure out a better place to put these!
#include "IO/IoBase.hpp"

// ROM image class.
#include "Rom.hpp"

namespace LibGens
{

class EmuContext
{
	public:
		EmuContext(Rom *rom);
		virtual ~EmuContext();
		
		/**
		 * saveData(): Save SRam/EEPRom.
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		virtual int saveData(void) = 0;
		
		/**
		 * autoSaveData(): AutoSave SRam/EEPRom.
		 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		virtual int autoSaveData(int framesElapsed) = 0;
		
		/**
		 * softReset(): Perform a soft reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int softReset(void) = 0;
		
		/**
		 * hardReset(): Perform a hard reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int hardReset(void) = 0;
		
		virtual void execFrame(void) = 0;
		virtual void execFrameFast(void) = 0;
		
		// Accessors.
		inline bool isRomOpened(void) { return (m_rom != NULL); }
		
		// Controllers.
		// TODO: Figure out a better place to put these!
		// TODO: Make these non-static!
		static IoBase *m_port1;		// Player 1.
		static IoBase *m_port2;		// Player 2.
		static IoBase *m_portE;		// EXT port.
	
	protected:
		Rom *m_rom;
	
	private:
		static int ms_RefCount;
};

}

#endif /* __LIBGENS_EMUCONTEXT_HPP__ */
