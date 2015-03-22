/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest_PageWrite.cpp: EEPRomI2C test: Page Write tests.          *
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

// C includes.
#include <alloca.h>

#include "EEPRomI2CTest.hpp"
namespace LibGens { namespace Tests {

class EEPRomI2CTest_PageWrite : public EEPRomI2CTest
{
	protected:
		/**
		 * Mode 2 EEPROM: Test full pages writes in an EEPROM.
		 * Starts at the specified address.
		 * NOTE: Each byte is written exactly once.
		 * Wraparound can be tested by starting at an address
		 * within the page instead of at the start of the page.
		 * @param addr_start Starting address.
		 * @param eepromSize EEPROM size.
		 * @param pgSize Page size.
		 */
		void eprMode2_pageWrite(unsigned int addr_start, unsigned int eepromSize, unsigned int pgSize);
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
TEST_P(EEPRomI2CTest_PageWrite, eprX24C01_pageWrite)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as X24C01.
	const unsigned int eepromSize = 128;
	const unsigned int eepromMask = eepromSize - 1;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 4;
	const unsigned int pgMask = pgSize - 1;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

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
	uint8_t eeprom_expected[eepromSize];
	unsigned int pgStart = (addr_start & eepromMask & ~pgMask);
	memset(eeprom_expected, 0xFF, sizeof(eeprom_expected));
	memcpy(&eeprom_expected[pgStart], &test_EEPRomI2C_data[pgStart], pgSize);

	// Get the data from the EEPROM.
	uint8_t eeprom_actual[eepromSize];
	ASSERT_EQ(0, m_eeprom->dbg_readEEPRom(0x00, eeprom_actual, sizeof(eeprom_actual)));

	// Verify the EEPROM data.
	CompareByteArrays(eeprom_expected, eeprom_actual, sizeof(eeprom_expected));
}

/**
 * Mode 2 EEPROM: Test full pages writes in an EEPROM.
 * Starts at the specified address.
 * NOTE: Each byte is written exactly once.
 * Wraparound can be tested by starting at an address
 * within the page instead of at the start of the page.
 * @param addr_start Starting address.
 * @param eepromSize EEPROM size.
 * @param pgSize Page size.
 */
void EEPRomI2CTest_PageWrite::eprMode2_pageWrite(unsigned int addr_start, unsigned int eepromSize, unsigned int pgSize)
{
	const unsigned int eepromMask = eepromSize - 1;
	const unsigned int pgMask = pgSize - 1;

	// Make sure we're in a STOP condition at first.
	doStop();

	// EEPROM values.
	uint8_t cmd, response;

	// START an I2C transfer.
	m_eeprom->dbg_setSDA(0);	// START

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
	uint8_t *eeprom_expected = (uint8_t*)alloca(eepromSize);
	unsigned int pgStart = (addr_start & eepromMask & ~pgMask);
	memset(eeprom_expected, 0xFF, eepromSize);
	memcpy(&eeprom_expected[pgStart], &test_EEPRomI2C_data[pgStart], pgSize);

	// Get the data from the EEPROM.
	uint8_t *eeprom_actual = (uint8_t*)alloca(eepromSize);
	ASSERT_EQ(0, m_eeprom->dbg_readEEPRom(0x00, eeprom_actual, eepromSize));

	// Verify the EEPROM data.
	CompareByteArrays(eeprom_expected, eeprom_actual, eepromSize);
}

/**
 * 24C01: Test sequential writing of full pages.
 * Starts at the specified address.
 * NOTE: Each byte is written exactly once.
 * Wraparound can be tested by starting at an address
 * within the page instead of at the start of the page.
 * @param addr_start Starting address.
 */
TEST_P(EEPRomI2CTest_PageWrite, epr24C01_pageWrite)
{
	unsigned int addr_start = GetParam();

	// Set the EEPROM as 24C01.
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE2));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	const unsigned int pgSize = 8;
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(pgSize));

	// Run the Mode 2 test.
	eprMode2_pageWrite(addr_start, eepromSize, pgSize);
}

// Page Write with various starting addresses.
INSTANTIATE_TEST_CASE_P(PageWrite_0x0000_0x03FF, EEPRomI2CTest_PageWrite,
	::testing::Values(
		// 24C01, 24C02
		0x0000, 0x0012, 0x004F, 0x0072, 0x0090, 0x00A3, 0x00C4, 0x00FF,
		// 24C04
		0x0100, 0x0112, 0x014F, 0x0172, 0x0190, 0x01A3, 0x01C4, 0x01FF,
		// 24C08
		0x0200, 0x0212, 0x024F, 0x0272, 0x0290, 0x02A3, 0x02C4, 0x02FF,
		0x0300, 0x0312, 0x034F, 0x0372, 0x0390, 0x03A3, 0x03C4, 0x03FF
	));
INSTANTIATE_TEST_CASE_P(PageWrite_0x0400_0x07FF, EEPRomI2CTest_PageWrite,
	::testing::Values(
		// 24C16
		0x0400, 0x0412, 0x044F, 0x0472, 0x0490, 0x04A3, 0x04C4, 0x04FF,
		0x0500, 0x0512, 0x054F, 0x0572, 0x0590, 0x05A3, 0x05C4, 0x05FF,
		0x0600, 0x0612, 0x064F, 0x0672, 0x0690, 0x06A3, 0x06C4, 0x06FF,
		0x0700, 0x0712, 0x074F, 0x0772, 0x0790, 0x07A3, 0x07C4, 0x07FF
	));
INSTANTIATE_TEST_CASE_P(PageWrite_0x0800_0x0BFF, EEPRomI2CTest_PageWrite,
	::testing::Values(
		// 24C32, part 1
		0x0800, 0x0812, 0x084F, 0x0872, 0x0890, 0x08A3, 0x08C4, 0x08FF,
		0x0900, 0x0912, 0x094F, 0x0972, 0x0990, 0x09A3, 0x09C4, 0x09FF,
		0x0A00, 0x0A12, 0x0A4F, 0x0A72, 0x0A90, 0x0AA3, 0x0AC4, 0x0AFF,
		0x0B00, 0x0B12, 0x0B4F, 0x0B72, 0x0B90, 0x0BA3, 0x0BC4, 0x0BFF
	));
INSTANTIATE_TEST_CASE_P(PageWrite_0x0C00_0x0FFF, EEPRomI2CTest_PageWrite,
	::testing::Values(
		// 24C32, part 2
		0x0C00, 0x0C12, 0x0C4F, 0x0C72, 0x0C90, 0x0CA3, 0x0CC4, 0x0CFF,
		0x0D00, 0x0D12, 0x0D4F, 0x0D72, 0x0D90, 0x0DA3, 0x0DC4, 0x0DFF,
		0x0E00, 0x0E12, 0x0E4F, 0x0E72, 0x0E90, 0x0EA3, 0x0EC4, 0x0EFF,
		0x0F00, 0x0F12, 0x0F4F, 0x0F72, 0x0F90, 0x0FA3, 0x0FC4, 0x0FFF
	));

} }
