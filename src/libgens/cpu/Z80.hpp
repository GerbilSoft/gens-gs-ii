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

#include <libgens/config.libgens.h>

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
		static inline void HardReset(void);
		static inline void SoftReset(void);
		static inline void Exec(int cyclesSubtract);
		static inline void Interrupt(uint8_t irq);
		static inline void ClearOdometer(void);
		static inline void SetOdometer(unsigned int odo);
		/** END: mdZ80 wrapper functions. **/
	
	protected:
		static mdZ80_context *ms_Z80;
	
	private:
		Z80() { }
		~Z80() { }
};

/** BEGIN: mdZ80 wrapper functions. **/

#ifdef GENS_ENABLE_EMULATION
/**
 * Reset the Z80. (Hard Reset)
 * This function should be called when resetting emulation.
 */
inline void Z80::HardReset(void)
{
	mdZ80_hard_reset(ms_Z80);
}

/**
 * Reset the Z80. (Soft Reset)
 * This function should be called when the Z80 !RESET line is asserted.
 */
inline void Z80::SoftReset(void)
{
	mdZ80_soft_reset(ms_Z80);
}

/**
 * Run the Z80.
 * @param cyclesSubtract Cycles to subtract from the Z80 cycles counter.
 */
inline void Z80::Exec(int cyclesSubtract)
{
	int cyclesToRun = (M68K_Mem::Cycles_Z80 - cyclesSubtract);

	// Only run the Z80 if it's enabled and it has the bus.
	if (M68K_Mem::Z80_State == (Z80_STATE_ENABLED | Z80_STATE_BUSREQ)) {
		z80_Exec(ms_Z80, cyclesToRun);
	} else {
		mdZ80_set_odo(ms_Z80, cyclesToRun);
	}
}

/**
 * Assert an interrupt. (IRQ)
 * @param irq Interrupt request.
 */
inline void Z80::Interrupt(uint8_t irq)
{
	mdZ80_interrupt(ms_Z80, irq);
}

/**
 * Clear the odometer.
 */
inline void Z80::ClearOdometer(void)
{
	mdZ80_clear_odo(ms_Z80);
}

/**
 * Set the odometer.
 * @param odo New odometer value.
 */
inline void Z80::SetOdometer(unsigned int odo)
{
	mdZ80_set_odo(ms_Z80, odo);
}

#else /* !GENS_ENABLE_EMULATION */

inline void Z80::HardReset(void) { }
inline void Z80::SoftReset(void) { }
inline void Z80::Exec(int cyclesSubtract) { ((void)cyclesSubtract); }
inline void Z80::Interrupt(uint8_t irq) { ((void)irq); }
inline void Z80::ClearOdometer(void) { }
inline void Z80::SetOdometer(unsigned int odo) { ((void)odo); }

#endif /* GENS_ENABLE_EMULATION */

}

#endif /* __LIBGENS_CPU_Z80_HPP__ */
