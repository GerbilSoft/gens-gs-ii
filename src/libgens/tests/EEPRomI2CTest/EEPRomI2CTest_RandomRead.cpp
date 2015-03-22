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

// NOTE: Page sizes used for MODE2 EEPROMs are taken from the
// Atmel AT24C01A/02/04/08A/16A datasheet.

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
	protected:
		/**
		 * Mode 1 EEPROM: Test random reading of an EEPROM.
		 * Reads 64 bytes at random addresses.
		 * @param useTestData If true, use test data; if false, use a blank EEPROM.
		 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode1_randomRead(bool useTestData, unsigned int enableRepeatedStart, unsigned int eepromSize);

		/**
		 * Mode 2 EEPROM: Test random reading of an EEPROM.
		 * Reads 64 bytes at random addresses.
		 * @param useTestData If true, use test data; if false, use a blank EEPROM.
		 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode2_randomRead(bool useTestData, unsigned int enableRepeatedStart, unsigned int eepromSize);
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
 * Mode 1 EEPROM: Test random reading of an EEPROM.
 * Reads 64 bytes at random addresses.
 * @param useTestData If true, use test data; if false, use a blank EEPROM.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_RandomRead::eprMode1_randomRead(bool useTestData, unsigned int enableRepeatedStart, unsigned int eepromSize)
{
	const unsigned int eepromMask = eepromSize - 1;

	if (useTestData) {
		// Initialize the EEPROM data.
		ASSERT_EQ(0, m_eeprom->dbg_writeEEPRom(0x00, test_EEPRomI2C_data, eepromSize));
	}

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
		ASSERT_EQ(0, response) << "EPR_MODE1_WORD_ADDRESS: NACK received; expected ACK.";

		// Read the data from the EEPROM.
		// NOTE: Do NOT acknowledge the data word; otherwise, the
		// EPROM will start sending the next word immediately, which
		// can prevent the START/STOP condition from being recognized.
		uint8_t data_actual = recvData(false);
		if (!useTestData) {
			// Empty EEPROM.
			EXPECT_EQ(0xFF, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0xFF (empty EEPROM).";
		} else {
			// Full EEPROM.
			uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
			EXPECT_EQ(data_expected, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0x" << (int)data_expected << " (test EEPROM).";
		}

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
 * X24C01: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, eprX24C01_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as X24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 1 test.
	eprMode1_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * X24C01: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, eprX24C01_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as X24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 1 test.
	eprMode1_randomRead(true, enableRepeatedStart, eepromSize);
}

/**
 * Mode 2 EEPROM: Test random reading of an EEPROM.
 * Reads 64 bytes at random addresses.
 * @param useTestData If true, use test data; if false, use a blank EEPROM.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_RandomRead::eprMode2_randomRead(bool useTestData, unsigned int enableRepeatedStart, unsigned int eepromSize)
{
	const unsigned int eepromMask = eepromSize - 1;

	if (useTestData) {
		// Initialize the EEPROM data.
		ASSERT_EQ(0, m_eeprom->dbg_writeEEPRom(0x00, test_EEPRomI2C_data, eepromSize));
	}

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

		/** Dummy write cycle. **/

		// Device select.
		// Assuming device address is 0.
		cmd = (0xA0 | 0);				// RW=0
		cmd |= (((addr & eepromMask) >> 7) & ~1);	// A10-A8
		response = sendData(cmd);
		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		ASSERT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=0: NACK received; expected ACK.";

		// Word address, low byte.
		cmd = (addr & 0xFF);				// A7-A0
		response = sendData(cmd);
		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		ASSERT_EQ(0, response) << "EPR_MODE2_WORD_ADDRESS_LOW: NACK received; expected ACK.";

		if (!enableRepeatedStart) {
			doStop();
			m_eeprom->dbg_setSDA(0);
		} else {
			m_eeprom->dbg_setSCL(0);
			m_eeprom->dbg_setSDA(1);
			// Send a START.
			m_eeprom->dbg_setSCL(1);
			m_eeprom->dbg_setSDA(0);
		}

		/** Current address read. **/

		// Device select.
		// Assuming device address is 0.
		cmd = (0xA0 | 1);				// RW=1
		cmd |= (((addr & eepromMask) >> 7) & ~1);	// A10-A8
		response = sendData(cmd);
		// Check for ACK.
		m_eeprom->dbg_getSDA(&response);
		ASSERT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=1: NACK received; expected ACK.";

		// Read the data from the EEPROM.
		// NOTE: Do NOT acknowledge the data word; otherwise, the
		// EPROM will start sending the next word immediately, which
		// can prevent the START/STOP condition from being recognized.
		uint8_t data_actual = recvData(false);
		if (!useTestData) {
			// Empty EEPROM.
			EXPECT_EQ(0xFF, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0xFF (empty EEPROM).";
		} else {
			// Full EEPROM.
			uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
			EXPECT_EQ(data_expected, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0x" << (int)data_expected << " (test EEPROM).";
		}

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
 * 24C01: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C01_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * 24C02: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C02_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C02.
	const unsigned int eepromSize = 256;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * 24C04: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C04_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 512;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * 24C08: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C08_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 1024;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * 24C16: Test random reading of an empty EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C16_randomReadEmpty)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 2048;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(false, enableRepeatedStart, eepromSize);
}

/**
 * 24C01: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C01_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(true, enableRepeatedStart, eepromSize);
}

/**
 * 24C02: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C02_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C02.
	const unsigned int eepromSize = 256;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(true, enableRepeatedStart, eepromSize);
}

/**
 * 24C04: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C04_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C04.
	const unsigned int eepromSize = 512;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(true, enableRepeatedStart, eepromSize);
}

/**
 * 24C08: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C08_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C08.
	const unsigned int eepromSize = 1024;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(true, enableRepeatedStart, eepromSize);
}

/**
 * 24C16: Test random reading of a full EEPROM.
 * Reads 64 bytes at random addresses.
 * @param enableRepeatedStart If zero, a STOP condition will be emitted after every read.
 */
TEST_P(EEPRomI2CTest_RandomRead, epr24C16_randomReadFull)
{
	unsigned int enableRepeatedStart = GetParam();

	// Set the EEPROM as 24C02.
	const unsigned int eepromSize = 2048;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));
	ASSERT_EQ(0, m_eeprom->dbg_setDevAddr(0));

	// Run the Mode 2 test.
	eprMode2_randomRead(true, enableRepeatedStart, eepromSize);
}

// Random Read tests.
// Value is non-zero to enable repeated start conditions.
INSTANTIATE_TEST_CASE_P(RandomRead, EEPRomI2CTest_RandomRead,
	::testing::Values(0, 1)
	);

} }
