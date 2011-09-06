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

// NOTE: This will include config.h for the current project, not necessarily libgens!
// TODO: Make a global config.h instead?
#include <config.h>

// mdZ80: Z80 CPU emulator.
#include "../mdZ80/mdZ80.h"

// M68K_Mem is needed for Z80_State.
#include "M68K_Mem.hpp"

// ZOMG Z80 structs.
#include "libzomg/zomg_z80.h"

// C includes.
#include <stdint.h>
#include <stdio.h>

namespace LibGens
{

class Z80
{
	public:
		static void Init(void);
		static void End(void);
		
		/**
		 * ReInit(): Reinitialize the Z80.
		 * This function should be called when starting emulation.
		 */
		static void ReInit(void);
		
		/** ZOMG savestate functions. **/
		static void ZomgSaveReg(Zomg_Z80RegSave_t *state);
		static void ZomgRestoreReg(const Zomg_Z80RegSave_t *state);
		
		/** BEGIN: mdZ80 wrapper functions. **/
		
		/**
		 * Reset(): Reset the Z80.
		 * This function should be called when the Z80 RESET line is asserted.
		 */
		static inline void Reset(void)
		{
#ifdef GENS_ENABLE_EMULATION
			mdZ80_reset(ms_Z80);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * Exec(): Run the Z80.
		 * @param cyclesSubtract Cycles to subtract from the Z80 cycles counter.
		 */
		static inline void Exec(int cyclesSubtract)
		{
#ifdef GENS_ENABLE_EMULATION
			int cyclesToRun = (M68K_Mem::Cycles_Z80 - cyclesSubtract);
			
			// Only run the Z80 if it's enabled and it has the bus.
			if (M68K_Mem::Z80_State == (Z80_STATE_ENABLED | Z80_STATE_BUSREQ))
				z80_Exec(ms_Z80, cyclesToRun);
			else
				mdZ80_set_odo(ms_Z80, cyclesToRun);
#else
			((void)cyclesSubtract);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * Interrupt(): Assert an interrupt. (IRQ)
		 * @param irq Interrupt request.
		 */
		static inline void Interrupt(uint8_t irq)
		{
#ifdef GENS_ENABLE_EMULATION
			mdZ80_interrupt(ms_Z80, irq);
#else
			((void)irq);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * ClearOdometer(): Clear the odometer.
		 */
		static inline void ClearOdometer(void)
		{
#ifdef GENS_ENABLE_EMULATION
			mdZ80_clear_odo(ms_Z80);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * SetOdometer(): Set the odometer.
		 * @param odo New odometer value.
		 */
		static inline void SetOdometer(unsigned int odo)
		{
#ifdef GENS_ENABLE_EMULATION
			mdZ80_set_odo(ms_Z80, odo);
#else
			((void)odo);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/** END: mdZ80 wrapper functions. **/
	
	protected:
		static mdZ80_context *ms_Z80;
	
	private:
		Z80() { }
		~Z80() { }
};

}

#endif /* __LIBGENS_CPU_Z80_HPP__ */
