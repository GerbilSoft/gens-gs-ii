/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest.cpp: EEPRomI2C test.                                      *
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

#include "EEPRomI2CTest.hpp"

// LibGens.
#include "lg_main.hpp"

// EEPRomI2C class.
#include "Save/EEPRomI2C.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <ctime>

// C++ includes.
#include <iostream>
#include <string>
using std::endl;
using std::string;

namespace LibGens { namespace Tests {

/**
 * Set up the EEPROM for testing.
 * TODO: Read EEPROM parameters and set it up here?
 */
void EEPRomI2CTest::SetUp(void)
{
	// Initialize the EEPROM.
	m_eeprom = new EEPRomI2C();
}

/**
 * Tear down the EEPROM.
 */
void EEPRomI2CTest::TearDown(void)
{
	delete m_eeprom;
	m_eeprom = nullptr;
}

/**
 * Ensure we're in a STOP condition.
 */
void EEPRomI2CTest::doStop(void)
{
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);
	m_eeprom->dbg_setSCL(1);
	m_eeprom->dbg_setSDA(1);
}

/**
 * Send an 8-bit data word to the EEPROM.
 * @param data 8-bit data word.
 * @return 0 if ACK; 1 if NACK.
 */
uint8_t EEPRomI2CTest::sendData(uint8_t data)
{
	for (int i = 8; i > 0; i--, data <<= 1) {
		// Data is written when SCL=0.
		m_eeprom->dbg_setSCL(0);
		m_eeprom->dbg_setSDA(!!(data & 0x80));
		m_eeprom->dbg_setSCL(1);
	}

	// Data has been written.
	// ACK or NACK is received on the 9th clock cycle.
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(1);	// Release SDA.
	m_eeprom->dbg_setSCL(1);

	// Get the ACK or NACK.
	m_eeprom->dbg_getSDA(&data);
	return data;
}

/**
 * Receive an 8-bit data word from the EEPROM.
 * @param ack If true, acknowledge the data word.
 * @return 8-bit data word.
 */
uint8_t EEPRomI2CTest::recvData(bool ack)
{
	uint8_t data = 0;
	uint8_t sda_in;

	// Release the SDA line.
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(1);

	for (int i = 8; i > 0; i--) {
		// Data is received when SCL=0.
		m_eeprom->dbg_setSCL(0);
		m_eeprom->dbg_getSDA(&sda_in);
		data <<= 1;
		data |= (sda_in & 1);
		m_eeprom->dbg_setSCL(1);
	}

	// Data has been received.
	if (ack) {
		// Send an acknowledgement.
		m_eeprom->dbg_setSCL(0);
		m_eeprom->dbg_setSDA(0);
		m_eeprom->dbg_setSCL(1);
	}
	return data;
}

/**
 * Compare two byte arrays.
 * The byte arrays are converted to hexdumps and then
 * compared using EXPECT_EQ().
 * @param expected Expected data.
 * @param actual Actual data.
 * @param size Size of both arrays.
 */
void EEPRomI2CTest::CompareByteArrays(const uint8_t *expected, const uint8_t *actual, unsigned int size)
{
	// Output format: (assume ~64 bytes per line)
	// 0000: 01 23 45 67 89 AB CD EF  01 23 45 67 89 AB CD EF
	const unsigned int bufSize = ((size / 16) + !!(size % 16)) * 64;
	char printf_buf[16];
	string s_expected, s_actual;
	s_expected.reserve(bufSize);
	s_actual.reserve(bufSize);

	// TODO: Use stringstream instead?
	const uint8_t *pE = expected, *pA = actual;
	for (size_t i = 0; i < size; i++, pE++, pA++) {
		if (i % 16 == 0) {
			// New line.
			if (i > 0) {
				// Append newlines.
				s_expected += '\n';
				s_actual += '\n';
			}

			snprintf(printf_buf, sizeof(printf_buf), "%04X: ", i);
			s_expected += printf_buf;
			s_actual += printf_buf;
		}

		// Print the byte.
		snprintf(printf_buf, sizeof(printf_buf), "%02X", *pE);
		s_expected += printf_buf;
		snprintf(printf_buf, sizeof(printf_buf), "%02X", *pA);
		s_actual += printf_buf;

		if (i % 16 == 7) {
			s_expected += "  ";
			s_actual += "  ";
		} else if (i % 16  < 15) {
			s_expected += ' ';
			s_actual += ' ';
		}
	}

	// Compare the byte arrays, and
	// print the strings on failure.
	EXPECT_EQ(0, memcmp(expected, actual, size)) <<
		"Expected EEPROM data:" << endl << s_expected << endl <<
		"Actual EEPROM data:" << endl << s_actual << endl;
}

} }

int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: EEPRomI2C tests.\n");

	// Initialize the random number seed.
	unsigned int seed = time(nullptr);
	srand(seed);
	fprintf(stderr, "Random number seed is: %08X\n\n", seed);

	::testing::InitGoogleTest(&argc, argv);
	LibGens::Init();
	return RUN_ALL_TESTS();
}
