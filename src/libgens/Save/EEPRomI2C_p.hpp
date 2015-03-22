/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRom_p.hpp: Serial EEPROM handler. (I2C) (Private class)              *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBGENS_SAVE_EEPROM_P_HPP__
#define __LIBGENS_SAVE_EEPROM_P_HPP__

// C includes. (C++ namespace)
#include <climits>

// C++ includes.
#include <string>

#include "EEPRomI2C.hpp"
namespace LibGens {

class EEPRomI2C;
class EEPRomI2CPrivate
{
	public:
		EEPRomI2CPrivate(EEPRomI2C *q);

	protected:
		friend class EEPRomI2C;
		EEPRomI2C *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		EEPRomI2CPrivate(const EEPRomI2CPrivate &);
		EEPRomI2CPrivate &operator=(const EEPRomI2CPrivate &);

	public:
		static const char fileExt[];
		std::string filename;		// EEPRom base filename.
		std::string pathname;		// EEPRom pathname.
		std::string fullPathname;	// Full pathname. (m_pathname + m_filename)

		// EEPRom. (8 KB max)
		uint8_t eeprom[0x2000];

		// /SCL and /SDA are both open-drain.
		// The line is 1 unless either the EEPROM or the
		// host device holds it low, in which case it's 0.

		// EEPRom state.
		uint8_t scl;		// /SCL: Clock.
		uint8_t sda_out;	// /SDA: Data. (output)
		uint8_t sda_in;		// /SDA: Data. (input)

		// Previous EEPRom state.
		uint8_t scl_prev;	// /SCL: Clock.
		uint8_t sda_out_prev;	// /SDA: Data. (output)
		uint8_t sda_in_prev;	// /SDA: Data. (input)

		uint16_t address;	// Word address.
		uint8_t counter;	// Cycle counter.
		uint8_t rw;		// Read/Write mode. (1 == read; 0 == write)
		uint8_t dev_addr;	// Device address. (Modes 2 and 3)

		uint8_t shift_rw;	// Shifting state. (1 == shift-out, 0 == shift-in)
		uint8_t data_buf;	// Temporary data buffer.

		// Internal EEPROM state.
		enum EEPRomState_t {
			EPR_STANDBY = 0,

			// Mode 1: Word address.
			// Format: [A6 A5 A4 A3 A2 A1 A0 RW]
			EPR_MODE1_WORD_ADDRESS,

			// Reading data.
			// Format: [D7 D6 D5 D4 D3 D2 D1 D0]
			EPR_READ_DATA,

			// Writing data.
			// Format: [D7 D6 D5 D4 D3 D2 D1 D0]
			EPR_WRITE_DATA,

			// Modes 2, 3: Device address.
			// Format: [ 1  0  1  0 A2 A1 A0 RW]
			// A2-A0 may be device select for smaller chips,
			// or address bits A11-A8 for larger chips.
			EPR_MODE2_DEVICE_ADDRESS,

			// Modes 2, 3: Word address, low byte.
			// Format: [A7 A6 A5 A4 A3 A2 A1 A0]
			EPR_MODE2_WORD_ADDRESS_LOW,

			// Mode 3: Word address, high byte.
			// Format: [A15 A14 A13 A12 A11 A10 A9 A8]
			EPR_MODE3_WORD_ADDRESS_HIGH,
		};
		EEPRomState_t state;

	private:
		// Dirty flag.
		bool dirty;
	public:
		// TODO: Accessor/mutator function?
		int framesElapsed;

	public:
		inline bool isDirty(void) const
			{ return dirty; }
		inline void setDirty(void)
			{ dirty = true; framesElapsed = 0; }
		inline void clearDirty(void)
			{ dirty = false; framesElapsed = 0; }

		/**
		 * AUTOSAVE_THRESHOLD_DEFAULT: Default autosave threshold, in milliseconds.
		 */
		static const int AUTOSAVE_THRESHOLD_DEFAULT = 1000;

		/**
		 * Determine the current /SDA line state.
		 * @return 1 for high; 0 for low.
		 */
		inline uint8_t getSDA(void) const
			{ return (sda_out & sda_in); }

		/**
		 * Determine the previous /SDA line state.
		 * @return 1 for high; 0 for low.
		 */
		inline uint8_t getSDA_prev(void) const
			{ return (sda_out_prev & sda_in_prev); }

		// SCL/SDL transition checks.
		inline bool checkSCL_LtoH(void) const
			{ return (!scl_prev && scl); }
		inline bool checkSCL_HtoL(void) const
			{ return (scl_prev && !scl); }
		inline bool checkSCL_H(void) const
			{ return (scl_prev && scl); }
		inline bool checkSCL_L(void) const
			{ return (!scl_prev && !scl); }

		inline bool checkSDA_LtoH(void) const
			{ return (!getSDA_prev() && getSDA()); }
		inline bool checkSDA_HtoL(void) const
			{ return (getSDA_prev() && !getSDA()); }
		inline bool checkSDA_H(void) const
			{ return (getSDA_prev() && getSDA()); }
		inline bool checkSDA_L(void) const
			{ return (!getSDA_prev() && !getSDA()); }

		// Check for START.
		inline bool checkStart(void) const
			{ return (checkSCL_H() && checkSDA_HtoL()); }
		// Check for STOP.
		inline bool checkStop(void) const
			{ return (checkSCL_H() && checkSDA_LtoH()); }

	public:
		/** EEPROM specifications. **/
		struct EEPRomChip_t {
			uint8_t epr_mode;	// EEPROM mode.
			uint8_t dev_addr;	// Device address.
			uint16_t sz_mask;	// Size mask.
			uint8_t pg_mask;	// Page mask.
		};

		/** ROM database. **/
		struct GameEEPRomInfo_t {
			char game_id[16];
			uint16_t checksum;
			EEPRomI2C::EEPRomMap_t mapper;
			EEPRomChip_t epr_chip;		// EEPROM chip specification.
		};
		static const GameEEPRomInfo_t rom_db[30];

		// Current EEPRom type.
		EEPRomChip_t eprChip;

	public:
		/**
		 * Clear the EEPRom and initialize settings.
		 * This function does NOT reset the EEPRom type!
		 */
		void reset(void);

		/**
		 * Go into standby mode.
		 */
		void doStandby(void);

		/**
		 * Process a shifted-in data word.
		 */
		void processI2CShiftIn(void);

		/**
		 * Determine what to do after shifting out a data word.
		 */
		void processI2CShiftOut(void);

		/**
		 * Process an I2C bit.
		 */
		void processI2Cbit(void);

	public:
		// Find the next highest power of two. (unsigned integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2u(T k) {
			if (k == 0)
				return 1;
			k--;
			for (unsigned int i = 1; i < sizeof(T)*CHAR_BIT; i <<= 1)
				k = k | k >> i;
			return k + 1;
		}
};

}

#endif /* __LIBGENS_SAVE_EEPROM_P_HPP__ */
