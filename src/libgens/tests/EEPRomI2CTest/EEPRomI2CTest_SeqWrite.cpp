/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest_SeqWrite.cpp: EEPRomI2C test: Sequential Write tests.     *
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

// EEPRomI2C class.
#include "Save/EEPRomI2C.hpp"

// ARRAY_SIZE(x)
#include "macros/common.h"

// Test EEPROM data.
#include "EEPRomI2CTest_data.h"

#include "EEPRomI2CTest.hpp"
namespace LibGens { namespace Tests {

class EEPRomI2CTest_SeqWrite : public EEPRomI2CTest
{
	// TODO: Add stuff?
};

/**
 * I2C notes:
 * - START condition: SCL high, SDA high-to-low
 * - STOP condition: SCL high, SDA low-to-high
 *
 * - Other than these two, SDA should only be modified
 *   by the master when SCL is low.
 */

/**
 * X24C01: Test sequential writing of full pages.
 * Starts at the specified address.
 * NOTE: Each byte is written exactly once.
 * Wraparound can be tested by starting at an address
 * within the page instead of at the start of the page.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqWrite, X24C01_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	m_eeprom->dbg_setEEPRomType(EEPRomI2C::EPR_X24C01);

	unsigned int eepromSize;
	m_eeprom->dbg_getEEPRomSize(&eepromSize);
	ASSERT_EQ(128U, eepromSize) << "X24C01 should be 128 bytes.";
	const unsigned int eepromMask = eepromSize - 1;

	unsigned int pgSize;
	m_eeprom->dbg_getPageSize(&pgSize);
	ASSERT_EQ(4U, pgSize) << "X24C01 should have 4-byte pages.";
	const unsigned int pgMask = pgSize - 1;

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// START an I2C transfer.
	// We'll request a WRITE to the specified address.
	// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]
	m_eeprom->dbg_setSDA(0);	// START
	cmd = (addr_start << 1) | 0;	// RW=0
	response = sendData(cmd);

	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "NACK received; expected ACK.";

	// Write the page twice.
	unsigned int src_addr = addr_start & eepromMask;
	for (unsigned int i = (pgSize * 2); i > 0; i--) {
		uint8_t data = test_EEPRomI2C_data[src_addr];
		response = sendData(data);

		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		EXPECT_EQ(0, response) << "NACK received; expected ACK.";

		// Next byte.
		src_addr = ((src_addr & ~pgMask) | ((src_addr + 1) & pgMask));
	}

	// STOP the transfer.
	doStop();

	// Determine the expected EEPROM data.
	// Should be all empty except for the page being written.
	uint8_t eeprom_expected[128];
	unsigned int pgStart = (addr_start & eepromMask & ~pgMask);
	memset(eeprom_expected, 0xFF, sizeof(eeprom_expected));
	memcpy(&eeprom_expected[pgStart], &test_EEPRomI2C_data[pgStart], pgSize);

	// Get the data from the EEPROM.
	uint8_t eeprom_actual[128];
	ASSERT_EQ(0, m_eeprom->dbg_readEEPRom(0x00, eeprom_actual, sizeof(eeprom_actual)));

	// Verify the EEPROM data.
	CompareByteArrays(eeprom_expected, eeprom_actual, sizeof(eeprom_expected));
}

// Sequential Read with various starting addresses.
// TODO: Add more addresses for larger EEPROMs?
INSTANTIATE_TEST_CASE_P(SeqWrite, EEPRomI2CTest_SeqWrite,
	::testing::Values(0x00, 0x12, 0x4F, 0x72, 0x90, 0xA3, 0xC4, 0xFF)
	);

} }
