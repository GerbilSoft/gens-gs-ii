/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * EEPRomI2CTest_SlaveAck.cpp: EEPRomI2C test: Slave ACK tests.            *
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

class EEPRomI2CTest_SlaveAck : public EEPRomI2CTest
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
 * X24C01: Test the Slave ACK timing.
 * ACK should be triggered by the slave when /SCL is low.
 */
TEST_F(EEPRomI2CTest_SlaveAck, X24C01_slaveAck)
{
	// Set the EEPROM as X24C01.
	// TODO: Move to SetUp().
	const unsigned int eepromSize = 128;
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomMode(EEPRomI2C::EPR_MODE1));
	ASSERT_EQ(0, m_eeprom->dbg_setEEPRomSize(eepromSize));
	ASSERT_EQ(0, m_eeprom->dbg_setPageSize(4));

	// Make sure we're in a STOP condition at first.
	doStop();

	// Send a START request.
	m_eeprom->dbg_setSDA(1);
	m_eeprom->dbg_setSCL(1);
	m_eeprom->dbg_setSDA(0);	// START

	// Send a simple READ request.
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A6
	m_eeprom->dbg_setSCL(1);	// Bit 7
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A5
	m_eeprom->dbg_setSCL(1);	// Bit 6
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A4
	m_eeprom->dbg_setSCL(1);	// Bit 5
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A3
	m_eeprom->dbg_setSCL(1);	// Bit 4
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A2
	m_eeprom->dbg_setSCL(1);	// Bit 3
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A1
	m_eeprom->dbg_setSCL(1);	// Bit 2
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(0);	// A0
	m_eeprom->dbg_setSCL(1);	// Bit 1
	m_eeprom->dbg_setSCL(0);
	m_eeprom->dbg_setSDA(1);	// RW
	m_eeprom->dbg_setSCL(1);	// Bit 0

	// Pull /SCL low in preparation for the 9th clock cycle.
	m_eeprom->dbg_setSCL(0);

	// /SDA should now be 0. (ACK)
	// Technically, the datasheet says it shouldn't be checked
	// until /SCL=1, but NBA Jam checks for it when /SCL=0.
	uint8_t sda_in;
	m_eeprom->dbg_getSDA(&sda_in);
	EXPECT_EQ(0, sda_in);
}

} }
