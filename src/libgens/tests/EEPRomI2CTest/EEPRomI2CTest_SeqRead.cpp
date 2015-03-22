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

class EEPRomI2CTest_SeqRead : public EEPRomI2CTest
{
	protected:
		/**
		 * Mode 1 EEPROM: Test sequential reading of an EEPROM.
		 * Starts at the specified address.
		 * @param useTestData If true, use test data; if false, use a blank EEPROM.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode1_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize);

		/**
		 * Mode 2 EEPROM: Test sequential reading of an EEPROM.
		 * Starts at the specified address.
		 * @param useTestData If true, use test data; if false, use a blank EEPROM.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode2_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize);

		/**
		 * Mode 3 EEPROM: Test sequential reading of an EEPROM.
		 * Starts at the specified address.
		 * @param useTestData If true, use test data; if false, use a blank EEPROM.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode3_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize);
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
 * Mode 1 EEPROM: Test sequential reading of an EEPROM.
 * Starts at the specified address.
 * @param useTestData If true, use test data; if false, use a blank EEPROM.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_SeqRead::eprMode1_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize)
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

	// START an I2C transfer.
	// We'll request a READ from the specified address.
	// Mode 1 word address: [A6 A5 A4 A3 A2 A1 A0 RW]
	m_eeprom->dbg_setSDA(0);	// START
	cmd = (addr_start << 1) | 1;	// RW=1
	response = sendData(cmd);

	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	ASSERT_EQ(0, response) << "NACK received; expected ACK.";

	// Read up to two times the size of the EEPROM.
	unsigned int end_addr = addr_start + (eepromSize * 2) - 1;
	for (unsigned int addr = addr_start; addr <= end_addr; addr++) {
		// Data word should only be acknowledged if this is not
		// the last byte being read.
		bool isLastByte = (addr != end_addr);
		uint8_t data_actual = recvData(isLastByte);
		if (!useTestData) {
			// Empty EEPROM.
			EXPECT_EQ(0xFF, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0xFF (empty ROM).";
		} else {
			// Full EEPROM.
			uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
			EXPECT_EQ(data_expected, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0x" << (int)data_expected << " (test EEPROM).";
		}
	}

	// STOP the transfer.
	doStop();
}

/**
 * X24C01: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, eprX24C01_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 1 test.
	eprMode1_seqRead(false, addr_start, eepromSize);
}

/**
 * X24C01: Test sequential reading of a full EEPROM.
 * EEPROM will contain random data from test_EEPRomI2C_data[].
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, eprX24C01_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 1 test.
	eprMode1_seqRead(true, addr_start, eepromSize);
}

/**
 * Mode 2 EEPROM: Test sequential reading of an EEPROM.
 * Starts at the specified address.
 * @param useTestData If true, use test data; if false, use a blank EEPROM.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_SeqRead::eprMode2_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize)
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

	// START an I2C transfer.
	m_eeprom->dbg_setSDA(0);	// START

	/** Dummy write cycle. **/

	// Device select.
	// Assuming device address is 0.
	cmd = (0xA0 | 0);				// RW=0
	cmd |= (((addr_start & eepromMask) >> 7) & ~1);	// A10-A8
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	ASSERT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=0: NACK received; expected ACK.";

	// Word address, low byte.
	cmd = (addr_start & 0xFF);			// A7-A0
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	ASSERT_EQ(0, response) << "EPR_MODE2_WORD_ADDRESS_LOW: NACK received; expected ACK.";

	// STOP the write; START a new transfer.
	doStop();
	m_eeprom->dbg_setSDA(0);

	/** Current address read. **/

	// Device select.
	// Assuming device address is 0.
	cmd = (0xA0 | 1);				// RW=1
	cmd |= (((addr_start & eepromMask) >> 7) & ~1);	// A10-A8
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	ASSERT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=1: NACK received; expected ACK.";

	// Read up to two times the size of the EEPROM.
	unsigned int end_addr = addr_start + (eepromSize * 2) - 1;
	for (unsigned int addr = addr_start; addr <= end_addr; addr++) {
		// Data word should only be acknowledged if this is not
		// the last byte being read.
		bool isLastByte = (addr != end_addr);
		uint8_t data_actual = recvData(isLastByte);
		if (!useTestData) {
			// Empty EEPROM.
			EXPECT_EQ(0xFF, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0xFF (empty ROM).";
		} else {
			// Full EEPROM.
			uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
			EXPECT_EQ(data_expected, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0x" << (int)data_expected << " (test EEPROM).";
		}
	}

	// STOP the transfer.
	doStop();
}

/**
 * 24C01: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C01_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C02: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C02_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C02.
	const unsigned int eepromSize = 256;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C04: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C04_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C04.
	const unsigned int eepromSize = 512;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C08: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C08_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C04.
	const unsigned int eepromSize = 1024;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C16: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C16_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C16.
	const unsigned int eepromSize = 2048;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C01: Test sequential reading of an full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C01_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(true, addr_start, eepromSize);
}

/**
 * 24C02: Test sequential reading of an full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C02_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C02.
	const unsigned int eepromSize = 256;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(true, addr_start, eepromSize);
}

/**
 * 24C04: Test sequential reading of an full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C04_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C04.
	const unsigned int eepromSize = 512;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(true, addr_start, eepromSize);
}

/**
 * 24C08: Test sequential reading of an full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C08_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C04.
	const unsigned int eepromSize = 1024;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(true, addr_start, eepromSize);
}

/**
 * 24C16: Test sequential reading of an full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C16_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C16.
	const unsigned int eepromSize = 2048;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 16;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_seqRead(true, addr_start, eepromSize);
}

/**
 * Mode 3 EEPROM: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param useTestData If true, use test data; if false, use a blank EEPROM.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_SeqRead::eprMode3_seqRead(bool useTestData, unsigned int addr_start, unsigned int eepromSize)
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

	// START an I2C transfer.
	m_eeprom->dbg_setSDA(0);	// START

	/** Dummy write cycle. **/

	// Device select.
	// Assuming device address is 0.
	cmd = (0xA0 | 0);				// RW=0
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=0: NACK received; expected ACK.";

	// Word address, high byte.
	cmd = ((addr_start >> 8) & 0xFF);		// A15-A0
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "EPR_MODE3_WORD_ADDRESS_LOW: NACK received; expected ACK.";

	// Word address, low byte.
	cmd = (addr_start & 0xFF);			// A7-A0
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "EPR_MODE2_WORD_ADDRESS_LOW: NACK received; expected ACK.";

	// STOP the write; START a new transfer.
	doStop();
	m_eeprom->dbg_setSDA(0);

	/** Current address read. **/

	// Device select.
	// Assuming device address is 0.
	cmd = (0xA0 | 1);				// RW=1
	response = sendData(cmd);
	// Check for ACK.
	m_eeprom->dbg_getSDA(&response);
	EXPECT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=1: NACK received; expected ACK.";

	// Read up to two times the size of the EEPROM.
	unsigned int end_addr = addr_start + (eepromSize * 2) - 1;
	for (unsigned int addr = addr_start; addr <= end_addr; addr++) {
		// Data word should only be acknowledged if this is not
		// the last byte being read.
		bool isLastByte = (addr != end_addr);
		uint8_t data_actual = recvData(isLastByte);
		if (!useTestData) {
			// Empty EEPROM.
			EXPECT_EQ(0xFF, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0xFF (empty ROM).";
		} else {
			// Full EEPROM.
			uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
			EXPECT_EQ(data_expected, data_actual) <<
				"EEPROM address 0x" <<
				std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
				" should be 0x" << (int)data_expected << " (test EEPROM).";
		}
	}

	// STOP the transfer.
	doStop();
}

/**
 * 24C32: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C32_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C32.
	const unsigned int eepromSize = 4096;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE3));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 32;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 3 test.
	eprMode3_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C64: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C64_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C64.
	const unsigned int eepromSize = 8192;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE3));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 32;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 3 test.
	eprMode3_seqRead(false, addr_start, eepromSize);
}

/**
 * 24C32: Test sequential reading of a full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C32_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C32.
	const unsigned int eepromSize = 4096;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE3));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 32;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 3 test.
	eprMode3_seqRead(true, addr_start, eepromSize);
}

/**
 * 24C64: Test sequential reading of a full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_SeqRead, epr24C64_seqReadFull)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C64.
	const unsigned int eepromSize = 8192;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE3));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 32;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 3 test.
	eprMode3_seqRead(true, addr_start, eepromSize);
}

// Sequential Read with various starting addresses.
INSTANTIATE_TEST_CASE_P(SeqRead_0x0000_0x03FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C01, 24C02
		0x0000, 0x0012, 0x004F, 0x0072, 0x0090, 0x00A3, 0x00C4, 0x00FF,
		// 24C04
		0x0100, 0x0112, 0x014F, 0x0172, 0x0190, 0x01A3, 0x01C4, 0x01FF,
		// 24C08
		0x0200, 0x0212, 0x024F, 0x0272, 0x0290, 0x02A3, 0x02C4, 0x02FF,
		0x0300, 0x0312, 0x034F, 0x0372, 0x0390, 0x03A3, 0x03C4, 0x03FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x0400_0x07FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C16
		0x0400, 0x0412, 0x044F, 0x0472, 0x0490, 0x04A3, 0x04C4, 0x04FF,
		0x0500, 0x0512, 0x054F, 0x0572, 0x0590, 0x05A3, 0x05C4, 0x05FF,
		0x0600, 0x0612, 0x064F, 0x0672, 0x0690, 0x06A3, 0x06C4, 0x06FF,
		0x0700, 0x0712, 0x074F, 0x0772, 0x0790, 0x07A3, 0x07C4, 0x07FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x0800_0x0BFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C32, part 1
		0x0800, 0x0812, 0x084F, 0x0872, 0x0890, 0x08A3, 0x08C4, 0x08FF,
		0x0900, 0x0912, 0x094F, 0x0972, 0x0990, 0x09A3, 0x09C4, 0x09FF,
		0x0A00, 0x0A12, 0x0A4F, 0x0A72, 0x0A90, 0x0AA3, 0x0AC4, 0x0AFF,
		0x0B00, 0x0B12, 0x0B4F, 0x0B72, 0x0B90, 0x0BA3, 0x0BC4, 0x0BFF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x0C00_0x0FFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C32, part 2
		0x0C00, 0x0C12, 0x0C4F, 0x0C72, 0x0C90, 0x0CA3, 0x0CC4, 0x0CFF,
		0x0D00, 0x0D12, 0x0D4F, 0x0D72, 0x0D90, 0x0DA3, 0x0DC4, 0x0DFF,
		0x0E00, 0x0E12, 0x0E4F, 0x0E72, 0x0E90, 0x0EA3, 0x0EC4, 0x0EFF,
		0x0F00, 0x0F12, 0x0F4F, 0x0F72, 0x0F90, 0x0FA3, 0x0FC4, 0x0FFF
	));

INSTANTIATE_TEST_CASE_P(SeqRead_0x1000_0x13FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C64, part 1
		0x1000, 0x1012, 0x104F, 0x1072, 0x1090, 0x10A3, 0x10C4, 0x10FF,
		0x1100, 0x1112, 0x114F, 0x1172, 0x1190, 0x11A3, 0x11C4, 0x11FF,
		0x1200, 0x1212, 0x124F, 0x1272, 0x1290, 0x12A3, 0x12C4, 0x12FF,
		0x1300, 0x1312, 0x134F, 0x1372, 0x1390, 0x13A3, 0x13C4, 0x13FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x1400_0x17FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C64, part 2
		0x1400, 0x1412, 0x144F, 0x1472, 0x1490, 0x14A3, 0x14C4, 0x14FF,
		0x1500, 0x1512, 0x154F, 0x1572, 0x1590, 0x15A3, 0x15C4, 0x15FF,
		0x1600, 0x1612, 0x164F, 0x1672, 0x1690, 0x16A3, 0x16C4, 0x16FF,
		0x1700, 0x1712, 0x174F, 0x1772, 0x1790, 0x17A3, 0x17C4, 0x17FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x1800_0x1BFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C64, part 3
		0x1800, 0x1812, 0x184F, 0x1872, 0x1890, 0x18A3, 0x18C4, 0x18FF,
		0x1900, 0x1912, 0x194F, 0x1972, 0x1990, 0x19A3, 0x19C4, 0x19FF,
		0x1A00, 0x1A12, 0x1A4F, 0x1A72, 0x1A90, 0x1AA3, 0x1AC4, 0x1AFF,
		0x1B00, 0x1B12, 0x1B4F, 0x1B72, 0x1B90, 0x1BA3, 0x1BC4, 0x1BFF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x1C00_0x1FFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C64, part 4
		0x1C00, 0x1C12, 0x1C4F, 0x1C72, 0x1C90, 0x1CA3, 0x1CC4, 0x1CFF,
		0x1D00, 0x1D12, 0x1D4F, 0x1D72, 0x1D90, 0x1DA3, 0x1DC4, 0x1DFF,
		0x1E00, 0x1E12, 0x1E4F, 0x1E72, 0x1E90, 0x1EA3, 0x1EC4, 0x1EFF,
		0x1F00, 0x1F12, 0x1F4F, 0x1F72, 0x1F90, 0x1FA3, 0x1FC4, 0x1FFF
	));

// NOTE: 24C128 isn't supported. The values here are designed
// to test wraparound on the 24C64.

INSTANTIATE_TEST_CASE_P(SeqRead_0x2000_0x23FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 1
		0x2000, 0x2012, 0x204F, 0x2072, 0x2090, 0x20A3, 0x20C4, 0x20FF,
		0x2100, 0x2112, 0x214F, 0x2172, 0x2190, 0x21A3, 0x21C4, 0x21FF,
		0x2200, 0x2212, 0x224F, 0x2272, 0x2290, 0x22A3, 0x22C4, 0x22FF,
		0x2300, 0x2312, 0x234F, 0x2372, 0x2390, 0x23A3, 0x23C4, 0x23FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x2400_0x27FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 2
		0x2400, 0x2412, 0x244F, 0x2472, 0x2490, 0x24A3, 0x24C4, 0x24FF,
		0x2500, 0x2512, 0x254F, 0x2572, 0x2590, 0x25A3, 0x25C4, 0x25FF,
		0x2600, 0x2612, 0x264F, 0x2672, 0x2690, 0x26A3, 0x26C4, 0x26FF,
		0x2700, 0x2712, 0x274F, 0x2772, 0x2790, 0x27A3, 0x27C4, 0x27FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x2800_0x2BFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 3
		0x2800, 0x2812, 0x284F, 0x2872, 0x2890, 0x28A3, 0x28C4, 0x28FF,
		0x2900, 0x2912, 0x294F, 0x2972, 0x2990, 0x29A3, 0x29C4, 0x29FF,
		0x2A00, 0x2A12, 0x2A4F, 0x2A72, 0x2A90, 0x2AA3, 0x2AC4, 0x2AFF,
		0x2B00, 0x2B12, 0x2B4F, 0x2B72, 0x2B90, 0x2BA3, 0x2BC4, 0x2BFF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x2C00_0x2FFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 4
		0x2C00, 0x2C12, 0x2C4F, 0x2C72, 0x2C90, 0x2CA3, 0x2CC4, 0x2CFF,
		0x2D00, 0x2D12, 0x2D4F, 0x2D72, 0x2D90, 0x2DA3, 0x2DC4, 0x2DFF,
		0x2E00, 0x2E12, 0x2E4F, 0x2E72, 0x2E90, 0x2EA3, 0x2EC4, 0x2EFF,
		0x2F00, 0x2F12, 0x2F4F, 0x2F72, 0x2F90, 0x2FA3, 0x2FC4, 0x2FFF
	));

INSTANTIATE_TEST_CASE_P(SeqRead_0x3000_0x33FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 5
		0x3000, 0x3012, 0x304F, 0x3072, 0x3090, 0x30A3, 0x30C4, 0x30FF,
		0x3100, 0x3112, 0x314F, 0x3172, 0x3190, 0x31A3, 0x31C4, 0x31FF,
		0x3200, 0x3212, 0x324F, 0x3272, 0x3290, 0x32A3, 0x32C4, 0x32FF,
		0x3300, 0x3312, 0x334F, 0x3372, 0x3390, 0x33A3, 0x33C4, 0x33FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x3400_0x37FF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 6
		0x3400, 0x3412, 0x344F, 0x3472, 0x3490, 0x34A3, 0x34C4, 0x34FF,
		0x3500, 0x3512, 0x354F, 0x3572, 0x3590, 0x35A3, 0x35C4, 0x35FF,
		0x3600, 0x3612, 0x364F, 0x3672, 0x3690, 0x36A3, 0x36C4, 0x36FF,
		0x3700, 0x3712, 0x374F, 0x3772, 0x3790, 0x37A3, 0x37C4, 0x37FF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x3800_0x3BFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 7
		0x3800, 0x3812, 0x384F, 0x3872, 0x3890, 0x38A3, 0x38C4, 0x38FF,
		0x3900, 0x3912, 0x394F, 0x3972, 0x3990, 0x39A3, 0x39C4, 0x39FF,
		0x3A00, 0x3A12, 0x3A4F, 0x3A72, 0x3A90, 0x3AA3, 0x3AC4, 0x3AFF,
		0x3B00, 0x3B12, 0x3B4F, 0x3B72, 0x3B90, 0x3BA3, 0x3BC4, 0x3BFF
	));
INSTANTIATE_TEST_CASE_P(SeqRead_0x3C00_0x3FFF, EEPRomI2CTest_SeqRead,
	::testing::Values(
		// 24C128, part 8
		0x3C00, 0x3C12, 0x3C4F, 0x3C72, 0x3C90, 0x3CA3, 0x3CC4, 0x3CFF,
		0x3D00, 0x3D12, 0x3D4F, 0x3D72, 0x3D90, 0x3DA3, 0x3DC4, 0x3DFF,
		0x3E00, 0x3E12, 0x3E4F, 0x3E72, 0x3E90, 0x3EA3, 0x3EC4, 0x3EFF,
		0x3F00, 0x3F12, 0x3F4F, 0x3F72, 0x3F90, 0x3FA3, 0x3FC4, 0x3FFF
	));

} }
