/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Z80.hpp: Z80 CPU wrapper class.                                         *
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

#ifndef __LIBGENS_CPU_Z80_HPP__
#define __LIBGENS_CPU_Z80_HPP__

// mdZ80: Z80 CPU emulator.
#include "mdZ80/mdZ80.h"

// M68K_Mem is needed for Z80_State.
#include "M68K_Mem.hpp"

// C includes.
#include <stdint.h>

namespace LibGens
{

class Z80
{
	public:
		static void Init(void);
		static void End(void);
		
		static void Reset(void);
		
		/**
		 * Exec(): Run the Z80.
		 * @param cyclesSubtract Cycles to subtract from the Z80 cycles counter.
		 */
		static inline void Exec(int cyclesSubtract)
		{
			int cyclesToRun = (M68K_Mem::Cycles_Z80 - cyclesSubtract);
			
			// Only run the Z80 if it's enabled and it has the bus.
			if (M68K_Mem::Z80_State == (Z80_STATE_ENABLED | Z80_STATE_BUSREQ))
				z80_Exec(&ms_Z80, cyclesToRun);
			else
				mdZ80_set_odo(&ms_Z80, cyclesToRun);
		}
		
		/**
		 * Interrupt(): Assert an interrupt. (IRQ)
		 * @param irq Interrupt request.
		 */
		static inline void Interrupt(uint8_t irq)
		{
			mdZ80_interrupt(&ms_Z80, irq);
		}
		
		/**
		 * ClearOdometer(): Clear the odometer.
		 */
		static inline void ClearOdometer(void)
		{
			mdZ80_clear_odo(&ms_Z80);
		}
		
		/**
		 * SetOdometer(): Set the odometer.
		 * @param odo New odometer value.
		 */
		static inline void SetOdometer(unsigned int odo)
		{
			mdZ80_set_odo(&ms_Z80, odo);
		}
	
	protected:
		static Z80_CONTEXT ms_Z80;
	
	private:
		Z80() { }
		~Z80() { }
};

}

#endif /* __LIBGENS_CPU_Z80_HPP__ */
