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
		 * Mode 2 EEPROM: Test sequential reading of an empty EEPROM.
		 * Starts at the specified address.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode2_seqReadEmpty(unsigned int addr_start, unsigned int eepromSize);

		/**
		 * Mode 2 EEPROM: Test sequential reading of a full EEPROM.
		 * Reads 64 bytes at random addresses.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 */
		void eprMode2_seqReadFull(unsigned int addr_start, unsigned int eepromSize);
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
TEST_P(EEPRomI2CTest_SeqRead, eprX24C01_seqReadEmpty)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
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
TEST_P(EEPRomI2CTest_SeqRead, eprX24C01_seqReadFull)
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

/**
 * Mode 2 EEPROM: Test sequential reading of an empty EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_SeqRead::eprMode2_seqReadEmpty(unsigned int addr_start, unsigned int eepromSize)
{
	const unsigned int eepromMask = eepromSize - 1;

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
	EXPECT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=0: NACK received; expected ACK.";

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
	cmd |= (((addr_start & eepromMask) >> 7) & ~1);	// A10-A8
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
		EXPECT_EQ(0xFF, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0xFF (empty ROM).";
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
	eprMode2_seqReadEmpty(addr_start, eepromSize);
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
	eprMode2_seqReadEmpty(addr_start, eepromSize);
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
	eprMode2_seqReadEmpty(addr_start, eepromSize);
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
	eprMode2_seqReadEmpty(addr_start, eepromSize);
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
	eprMode2_seqReadEmpty(addr_start, eepromSize);
}

/**
 * Mode 2 EEPROM: Test sequential reading of a full EEPROM.
 * Starts at the specified address.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 */
void EEPRomI2CTest_SeqRead::eprMode2_seqReadFull(unsigned int addr_start, unsigned int eepromSize)
{
	const unsigned int eepromMask = eepromSize - 1;

	// Initialize the EEPROM data.
	ASSERT_EQ(0, m_eeprom->dbg_writeEEPRom(0x00, test_EEPRomI2C_data, eepromSize));

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
	EXPECT_EQ(0, response) << "EPR_MODE2_DEVICE_ADDRESS, RW=0: NACK received; expected ACK.";

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
	cmd |= (((addr_start & eepromMask) >> 7) & ~1);	// A10-A8
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
		uint8_t data_expected = test_EEPRomI2C_data[addr & eepromMask];
		EXPECT_EQ(data_expected, data_actual) <<
			"EEPROM address 0x" <<
			std::hex << std::setw(2) << std::setfill('0') << std::uppercase << addr <<
			" should be 0x" << (int)data_expected << " (test EEPROM).";
	}

	// STOP the transfer.
	doStop();
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
	eprMode2_seqReadFull(addr_start, eepromSize);
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
	eprMode2_seqReadFull(addr_start, eepromSize);
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
	eprMode2_seqReadFull(addr_start, eepromSize);
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
	eprMode2_seqReadFull(addr_start, eepromSize);
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
	eprMode2_seqReadFull(addr_start, eepromSize);
}

// Sequential Read with various starting addresses.
// TODO: Add more addresses for larger EEPROMs?
INSTANTIATE_TEST_CASE_P(SeqRead, EEPRomI2CTest_SeqRead,
	::testing::Values(0x00, 0x12, 0x4F, 0x72, 0x90, 0xA3, 0xC4, 0xFF)
	);

} }
