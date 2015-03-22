/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest_SeqRead.cpp: EEPRomI2C test: Sequential Read tests.       *
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

class EEPRomI2CTest_SeqRead : public EEPRomI2CTest
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
 * X24C01: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, X24C01_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// START an I2C transfer.
	// We'll request a READ from the specified address.
	// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]
	m_eeprom->dbg_setSDA(0);	// START
	cmd = (addr_start << 1) | 1;	// RW=1
	response = sendData(cmd);

	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "NACK received; expected ACK.";

	// Read up to two times the size of the EEPROM.
	unsigned int end_addr = addr_start + (eepromSize * 2) - 1;
	for (unsigned int addr = addr_start; addr <= end_addr; addr++) {
		// Data word should only be acknowledged if this is not
		// the last byte being read.
		bool isLastByte = (addr != end_addr);
		uint8_t data_actual = recvData(isLastByte);
		EXPECT_EQ(0xFF, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0xFF (empty ROM).";
	}

	// STOP the transfer.
	doStop();
}

/**
 * X24C01: Test sequential reading of a full EEPROM.
 * EEPROM will contain random data from test_EEPRomI2C_data[].
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, X24C01_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	const unsigned int eepromSize = 128;
	const unsigned int eepromMask = eepromSize - 1;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Initialize the EEPROM data.
	ASSERT_EQ(0, m_eeprom->dbg_writeEEPRom(0x00, test_EEPRomI2C_data, eepromSize));

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// START an I2C transfer.
	// We'll request a READ from the specified address.
	// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]
	m_eeprom->dbg_setSDA(0);	// START
	cmd = (addr_start << 1) | 1;	// RW=1
	response = sendData(cmd);

	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "NACK received; expected ACK.";

	// Read up to two times the size of the EEPROM.
	unsigned int end_addr = addr_start + (eepromSize * 2) - 1;
	for (unsigned int addr = addr_start; addr <= end_addr; addr++) {
		// Data word should only be acknowledged if this is not
		// the last byte being read.
		bool isLastByte = (addr != end_addr);
		uint8_t data_actual = recvData(isLastByte);
		uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
		EXPECT_EQ(data_expected, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0x" << (int)data_expected << " (test EEPROM).";
	}

	// STOP the transfer.
	doStop();
}

// Sequential Read with various starting addresses.
// TODO: Add more addresses for larger EEPROMs?
INSTANTIATE_TEST_CASE_P(SeqRead, EEPRomI2CTest_SeqRead,
	::testing::Values(0x00, 0x12, 0x4F, 0x72, 0x90, 0xA3, 0xC4, 0xFF)
	);

} }
