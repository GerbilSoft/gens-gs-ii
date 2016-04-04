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

#include "../cz80/cz80.h"
#include "libzomg/zomg_z80.h"

// M68K_Mem is needed for Z80_State.
#include "M68K_Mem.hpp"

// C includes.
#include <stdint.h>
#include <stdio.h>

namespace LibGens {

class Z80
{
	public:
		static void Init(void);
		static void End(void);

	private:
		Z80();
		~Z80();

	public:
		/**
		 * Reinitialize the Z80.
		 * This function should be called when starting emulation.
		 */
		static void ReInit(void);

		/** ZOMG savestate functions. **/
		static void ZomgSaveReg(Zomg_Z80RegSave_t *state);
		static void ZomgRestoreReg(const Zomg_Z80RegSave_t *state);

		/** BEGIN: Cz80 wrapper functions. **/
		static inline void HardReset(void);
		static inline void SoftReset(void);
		static inline void Exec(int cyclesSubtract);
		static inline void Interrupt(uint8_t irq);
		static inline void ClearOdometer(void);
		static inline void SetOdometer(unsigned int odo);
		/** END: Cz80 wrapper functions. **/

	protected:
		static cz80_struc *ms_Z80;

		// Cz80 uses "run xxx cycles" instead of an odometer.
		static int ms_cycleCnt;		// Cycles currently run.
};

/** BEGIN: mdZ80 wrapper functions. **/

/**
 * Reset the Z80. (Hard Reset)
 * This function should be called when resetting emulation.
 */
inline void Z80::HardReset(void)
{
	Cz80_Reset(ms_Z80);
}

/**
 * Reset the Z80. (Soft Reset)
 * This function should be called when the Z80 !RESET line is asserted.
 */
inline void Z80::SoftReset(void)
{
	Cz80_Soft_Reset(ms_Z80);
}

/**
 * Run the Z80.
 * @param cyclesSubtract Cycles to subtract from the Z80 cycles counter.
 */
inline void Z80::Exec(int cyclesSubtract)
{
	// M68K_Mem::Cycles_Z80 has the total number of cycles that should be run up to this point.
	// cyclesSubtract is the number of cycles to save.
	// cyclesTarget is the destination cycle count.
	int cyclesTarget = (M68K_Mem::Cycles_Z80 - cyclesSubtract);
	// cyclesToRun is the number of cycles to run right now.
	int cyclesToRun = cyclesTarget - ms_cycleCnt;
	if (cyclesToRun <= 0)
		return;

	// Only run the Z80 if it's enabled and it has the bus.
	if (M68K_Mem::Z80_State == (Z80_STATE_ENABLED | Z80_STATE_BUSREQ)) {
		int ret = Cz80_Exec(ms_Z80, cyclesToRun);
		if (ret >= 0) {
			// ret == number of cycles run.
			ms_cycleCnt += ret;
		}
	} else {
		// CPU isn't enabled, so add the cycles without doing anything.
		ms_cycleCnt += cyclesToRun;
	}
}

/**
 * Assert an interrupt. (IRQ)
 * @param irq Interrupt request.
 */
inline void Z80::Interrupt(uint8_t irq)
{
	Cz80_Set_IRQ(ms_Z80, irq);
}

/**
 * Clear the odometer.
 */
inline void Z80::ClearOdometer(void)
{
	ms_cycleCnt = 0;
}

/**
 * Set the odometer.
 * @param odo New odometer value.
 */
inline void Z80::SetOdometer(unsigned int odo)
{
	ms_cycleCnt = odo;
}

}

#endif /* __LIBGENS_CPU_Z80_HPP__ */
