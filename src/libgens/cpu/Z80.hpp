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
		Z80();
		~Z80();

	public:
		/**
		 * Reinitialize the Z80.
		 * This function should be called when starting emulation.
		 */
		void reinit(void);

		/** ZOMG savestate functions. **/
		void zomgSaveReg(Zomg_Z80RegSave_t *state);
		void zomgRestoreReg(const Zomg_Z80RegSave_t *state);

		/** BEGIN: Cz80 wrapper functions. **/
		inline void hardReset(void);
		inline void softReset(void);
		inline void exec(int cyclesSubtract);
		inline void interrupt(uint8_t irq);
		inline void clearOdometer(void);
		inline void setOdometer(unsigned int odo);
		/** END: Cz80 wrapper functions. **/

	protected:
		cz80_struc *m_z80;

		// Cz80 uses "run xxx cycles" instead of an odometer.
		int m_cycleCnt;		// Cycles currently run.

	public:
		// Z80 memory.
		// TODO: Add accessors and make this protected.
		uint8_t m_ramZ80[8192];

	protected:
		/** Memory access functions. **/
		// ctx == Z80*
		static uint8_t CZ80CALL Z80_MD_ReadB_static(void *ctx, uint16_t address);
		static void CZ80CALL Z80_MD_WriteB_static(void *ctx, uint16_t address, uint8_t data);
		static uint16_t CZ80CALL Z80_MD_ReadW_static(void *ctx, uint16_t address);
		static void CZ80CALL Z80_MD_WriteW_static(void *ctx, uint16_t address, uint16_t data);

	public:
		// Non-static versions.
		inline uint8_t CZ80CALL Z80_MD_ReadB(uint16_t address);
		inline void CZ80CALL Z80_MD_WriteB(uint16_t address, uint8_t data);
		inline uint16_t CZ80CALL Z80_MD_ReadW(uint16_t address);
		inline void CZ80CALL Z80_MD_WriteW(uint16_t address, uint16_t data);

	protected:
		/** Z80 memory functions: MD mode. **/

	public:
		// M68K ROM banking address.
		// TODO: Add accessors and make this protected.
		uint32_t m_bankZ80;

	protected:
		/** Read Byte functions. **/
		uint8_t Z80_MD_ReadB_YM2612(uint16_t address);
		uint8_t Z80_MD_ReadB_VDP(uint16_t address);
		uint8_t Z80_MD_ReadB_68K_Rom(uint16_t address);

		/** Write Byte functions. **/
		void Z80_MD_WriteB_Bank(uint16_t address, uint8_t data);
		void Z80_MD_WriteB_YM2612(uint16_t address, uint8_t data);
		void Z80_MD_WriteB_VDP(uint16_t address, uint8_t data);
		void Z80_MD_WriteB_68K_Rom(uint16_t address, uint8_t data);
};

/** BEGIN: mdZ80 wrapper functions. **/

/**
 * Reset the Z80. (Hard Reset)
 * This function should be called when resetting emulation.
 */
inline void Z80::hardReset(void)
{
	Cz80_Reset(m_z80);
}

/**
 * Reset the Z80. (Soft Reset)
 * This function should be called when the Z80 !RESET line is asserted.
 */
inline void Z80::softReset(void)
{
	Cz80_Soft_Reset(m_z80);
}

/**
 * Run the Z80.
 * @param cyclesSubtract Cycles to subtract from the Z80 cycles counter.
 */
inline void Z80::exec(int cyclesSubtract)
{
	// M68K_Mem::Cycles_Z80 has the total number of cycles that should be run up to this point.
	// cyclesSubtract is the number of cycles to save.
	// cyclesTarget is the destination cycle count.
	int cyclesTarget = (M68K_Mem::Cycles_Z80 - cyclesSubtract);
	// cyclesToRun is the number of cycles to run right now.
	int cyclesToRun = cyclesTarget - m_cycleCnt;
	if (cyclesToRun <= 0)
		return;

	// Only run the Z80 if it's enabled and it has the bus.
	if (M68K_Mem::Z80_State == (Z80_STATE_ENABLED | Z80_STATE_BUSREQ)) {
		int ret = Cz80_Exec(m_z80, cyclesToRun);
		if (ret >= 0) {
			// ret == number of cycles run.
			m_cycleCnt += ret;
		}
	} else {
		// CPU isn't enabled, so add the cycles without doing anything.
		m_cycleCnt += cyclesToRun;
	}
}

/**
 * Assert an interrupt. (IRQ)
 * @param irq Interrupt request.
 */
inline void Z80::interrupt(uint8_t irq)
{
	Cz80_Set_IRQ(m_z80, irq);
}

/**
 * Clear the odometer.
 */
inline void Z80::clearOdometer(void)
{
	m_cycleCnt = 0;
}

/**
 * Set the odometer.
 * @param odo New odometer value.
 */
inline void Z80::setOdometer(unsigned int odo)
{
	m_cycleCnt = odo;
}

/** Memory access functions. **/

inline uint8_t CZ80CALL Z80::Z80_MD_ReadB(uint16_t address)
{
	return Z80_MD_ReadB_static(this, address);
}

inline void CZ80CALL Z80::Z80_MD_WriteB(uint16_t address, uint8_t data)
{
	Z80_MD_WriteB_static(this, address, data);
}

inline uint16_t CZ80CALL Z80::Z80_MD_ReadW(uint16_t address)
{
	return Z80_MD_ReadW_static(this, address);
}

inline void CZ80CALL Z80::Z80_MD_WriteW(uint16_t address, uint16_t data)
{
	Z80_MD_WriteW_static(this, address, data);
}

}

#endif /* __LIBGENS_CPU_Z80_HPP__ */
