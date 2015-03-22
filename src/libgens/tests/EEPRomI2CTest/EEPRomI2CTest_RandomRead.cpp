/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest_RandomRead.cpp: EEPRomI2C test: Random Read tests.        *
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

class EEPRomI2CTest_RandomRead : public EEPRomI2CTest
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
 * X24C01: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, X24C01_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	m_eeprom->dbg_setEEPRomType(EEPRomI2C::EPR_X24C01);
	unsigned int eepromSize;
	m_eeprom->dbg_getEEPRomSize(&eepromSize);
	ASSERT_EQ(128U, eepromSize) << "X24C01 should be 128 bytes.";
	const unsigned int eepromMask = eepromSize - 1;

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// Read up to 64 random addresses.
	for (int i = 64; i > 0; i--) {
		const unsigned int addr = (rand() & eepromMask);

		// START an I2C transfer.
		// We'll request a READ from a random address
		// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]

		if (enableRepeatedStart) {
			// No STOP condition, so we have to
			// release /SDA from ACK while /SCL=0.
			m_eeprom->dbg_setSCL(0);
			m_eeprom->dbg_setSDA(1);
		}

		// Send a START.
		m_eeprom->dbg_setSCL(1);
		m_eeprom->dbg_setSDA(0);

		cmd = (addr << 1) | 1;		// RW=1
		response = sendData(cmd);

		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		EXPECT_EQ(0, response) << "NACK received; expected ACK.";

		// Read the data from the EEPROM.
		// NOTE: Do NOT acknowledge the data word; otherwise, the
		// EPROM will start sending the next word immediately, which
		// can prevent the START/STOP condition from being recognized.
		uint8_t data_actual = recvData(false);
		EXPECT_EQ(0xFF, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0xFF (empty EEPROM).";

		if (!enableRepeatedStart) {
			// STOP the transfer.
			doStop();
		}
	}

	if (enableRepeatedStart) {
		// STOP the transfer.
		doStop();
	}
}

/**
 * X24C01: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, X24C01_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	m_eeprom->dbg_setEEPRomType(EEPRomI2C::EPR_X24C01);
	unsigned int eepromSize;
	m_eeprom->dbg_getEEPRomSize(&eepromSize);
	ASSERT_EQ(128U, eepromSize) << "X24C01 should be 128 bytes.";
	const unsigned int eepromMask = eepromSize - 1;

	// Initialize the EEPROM data.
	ASSERT_EQ(0, m_eeprom->dbg_writeEEPRom(0x00, test_EEPRomI2C_data, eepromSize));

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// Read up to 64 random addresses.
	for (int i = 64; i > 0; i--) {
		const unsigned int addr = (rand() & eepromMask);

		// START an I2C transfer.
		// We'll request a READ from a random address.
		// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]

		if (enableRepeatedStart) {
			// No STOP condition, so we have to
			// release /SDA from ACK while /SCL=0.
			m_eeprom->dbg_setSCL(0);
			m_eeprom->dbg_setSDA(1);
		}

		// Send a START.
		m_eeprom->dbg_setSCL(1);
		m_eeprom->dbg_setSDA(0);

		cmd = (addr << 1) | 1;		// RW=1
		response = sendData(cmd);

		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		EXPECT_EQ(0, response) << "NACK received; expected ACK.";

		// Read the data from the EEPROM.
		// NOTE: Do NOT acknowledge the data word; otherwise, the
		// EPROM will start sending the next word immediately, which
		// can prevent the START/STOP condition from being recognized.
		uint8_t data_actual = recvData(false);
		uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
		EXPECT_EQ(data_expected, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0x" << (int)data_expected << " (test EEPROM).";

		if (!enableRepeatedStart) {
			// STOP the transfer.
			doStop();
		}
	}

	if (enableRepeatedStart) {
		// STOP the transfer.
		doStop();
	}
}

// Random Read tests.
// Value is non-zero to enable repeated start conditions.
INSTANTIATE_TEST_CASE_P(RandomRead, EEPRomI2CTest_RandomRead,
	::testing::Values(0, 1)
	);

} }
