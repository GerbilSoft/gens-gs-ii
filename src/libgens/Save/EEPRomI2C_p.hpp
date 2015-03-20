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

		// Internal EEPROM state.
		enum EEPRomState_t {
			EPR_STANDBY = 0,

			// Mode 1: Receiving word address.
			// Format: [A6 A5 A4 A3 A2 A1 A0 RW]
			EPR_MODE1_WORD_ADDRESS,
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
		/** EEPROM types. **/
		enum EEPRomType_t {
			EPR_NONE = 0,

			// Mode 1: 7-bit addressing.
			EPR_X24C01,

			EPR_MAX
		};

		/** EEPROM specifications. **/
		struct EEPRomSpec_t {
			uint16_t sz_mask;	// Size mask.
			uint8_t pg_mask;	// Page mask.
			uint8_t reserved;
		};
		static const EEPRomSpec_t eeprom_spec[EPR_MAX];

		/** EEPROM map. **/
		struct EEPRomMap_t {
			uint32_t sda_in_adr;		// 68000 memory address mapped to SDA_IN.
			uint32_t sda_out_adr;		// 68000 memory address mapped to SDA_OUT.
			uint32_t scl_adr;		// 68000 memory address mapped to SCL.
			uint8_t sda_in_bit;		// Bit offset for SDA_IN.
			uint8_t sda_out_bit;		// Bit offset for SDA_OUT.
			uint8_t scl_bit;		// Bit offset for SCL.
			uint8_t epr_type;		// EEPROM type.
		};

		/** ROM database. **/
		struct GameEEPRomInfo_t {
			char game_id[16];
			uint16_t checksum;
			EEPRomMap_t type;
		};
		static const GameEEPRomInfo_t rom_db[1];

		// Current EEPRom type.
		EEPRomSpec_t eprSpec;
		GameEEPRomInfo_t eprType;

	public:
		/**
		 * Clear the EEPRom and initialize settings.
		 * This function does NOT reset the EEPRom type!
		 */
		void reset(void);

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
