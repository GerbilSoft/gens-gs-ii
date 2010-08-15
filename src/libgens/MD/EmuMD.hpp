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

// Controllers.
// TODO: Figure out a better place to put these!
#include "../IO/IoBase.hpp"

// ROM image class.
#include "../Rom.hpp"

namespace LibGens
{

class EmuMD
{
	public:
		static void Init(void);
		static void End(void);
		
		static int SetRom(Rom *rom);
		
		static void Do_Frame(void);
		
		// Controllers.
		// TODO: Figure out a better place to put these!
		static IoBase *m_port1;	// Player 1.
		static IoBase *m_port2;	// Player 2.
		static IoBase *m_portE;	// EXT port.
	
	protected:
		/**
		* LineType_t: Line types.
		*/
		enum LineType_t
		{
			LINETYPE_ACTIVEDISPLAY	= 0,
			LINETYPE_VBLANKLINE	= 1,
			LINETYPE_BORDER		= 2,
		};
		
		// TODO: Port FORCE_INLINE from Gens/GS.
		#define FORCE_INLINE inline
		
		template<LineType_t LineType, bool VDP>
		static FORCE_INLINE void T_Do_Line(void);
		
		template<bool VDP>
		static FORCE_INLINE void T_Do_Frame(void);
	
	private:
		EmuMD() { }
		~EmuMD() { }
};

}

#endif /* __LIBGENS_MD_EMUMD_HPP__ */
