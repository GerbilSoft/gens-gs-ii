/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * PsgRegisterTest.cpp: PSG register test.                                 *
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

/**
 * Register test cases are based on the SMS Power SN76489 description:
 * http://www.smspower.org/Development/SN76489
 */

// Google Test
#include "gtest/gtest.h"

// LibGens.
#include "lg_main.hpp"

// LibGens PSG.
#include "sound/Psg.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

namespace LibGens { namespace Tests {

class PsgRegisterTest : public ::testing::Test
{
	protected:
		PsgRegisterTest()
			: ::testing::Test()
			, m_psg(nullptr) { }
		virtual ~PsgRegisterTest() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		Psg *m_psg;
};

/**
 * Set up the Vdp for testing.
 */
void PsgRegisterTest::SetUp(void)
{
	// TODO: Initialize source clock and output rate?
	m_psg = new Psg();
	// NOTE: The PSG tables will have garbage values
	// because the clock hasn't been initialized.
	m_psg->reset();
}

void PsgRegisterTest::TearDown(void)
{
	delete m_psg;
	m_psg = nullptr;
}

/**
 * Set channel 0 tone to 0xFE. (440 Hz @ 3,579,545 Hz clock)
 */
TEST_F(PsgRegisterTest, setChannel0ToneTo0xFE)
{
	uint8_t regNumLatch; uint16_t val;

	// Latch the register.
	m_psg->write(0x8E);	// %1 00 0 1110: LATCH, channel 0, tone, data %1110
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(0, regNumLatch);
	// Write the data.
	m_psg->write(0x0F);	// %0 0  001111: DATA, %001111
	// Verify that the latched register number hasn't changed.
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(0, regNumLatch);
	// Verify the tone value.
	m_psg->dbg_getReg(0, &val);
	EXPECT_EQ(0xFE, val);

	// TODO: Verify that other registers haven't been touched?
}

/**
 * Set channel 1 volume to 0xF. (silent)
 */
TEST_F(PsgRegisterTest, setChannel1VolumeTo0xF)
{
	uint8_t regNumLatch; uint16_t val;

	// NOTE: The initial PSG state ensures all volumes are 0xF.
	m_psg->write(0xBF);	// %1 01 1 1111: LATCH, channel 1, volume, data %1111
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(3, regNumLatch);
	// Verify the volume.
	m_psg->dbg_getReg(3, &val);
	EXPECT_EQ(0xF, val);

	// TODO: Verify that other registers haven't been touched?
}

/**
 * Set channel 2 volume to 0xF (silent), THEN update to 0x0 (full).
 * Only one LATCH byte is used.
 */
TEST_F(PsgRegisterTest, setChannel2VolumeTo0xF_then_0x0_NoLatch)
{
	uint8_t regNumLatch; uint16_t val;

	// NOTE: The initial PSG state ensures all volumes are 0xF.
	m_psg->write(0xDF);	// %1 10 1 1111: LATCH, channel 2, volume, data %1111
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(5, regNumLatch);
	// Verify the volume.
	m_psg->dbg_getReg(5, &val);
	EXPECT_EQ(0xF, val);

	// TODO: Verify that other registers haven't been touched?

	// Set the volume again, but this time,
	// don't write a LATCH byte.
	m_psg->write(0x00);	// %0 0  000000: DATA, %000000
	// Verify that the latched register number hasn't changed.
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(5, regNumLatch);
	// Verify the volume.
	m_psg->dbg_getReg(5, &val);
	EXPECT_EQ(0x0, val);

	// TODO: Verify that other registers haven't been touched?
}

/**
 * Set noise register to 0x5. (white noise, medium shift rate)
 */
TEST_F(PsgRegisterTest, setNoiseRegisterTo0x5)
{
	uint8_t regNumLatch; uint16_t val;

	m_psg->write(0xE5);	// %1 11 0 0101: LATCH, channel 3, noise, data %0101
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(6, regNumLatch);
	// Verify the noise value.
	m_psg->dbg_getReg(6, &val);
	EXPECT_EQ(0x5, val);

	// TODO: Verify that other registers haven't been touched?
}

/**
 * Set noise register to 0x5 (white noise, medium shift rate),
 * THEN update it to 0x4 (white noise, high shift rate).
 * Only one LATCH byte is used.
 */
TEST_F(PsgRegisterTest, setNoiseRegisterTo0x5_then_0x4_NoLatch)
{
	uint8_t regNumLatch; uint16_t val;

	m_psg->write(0xE5);	// %1 11 0 0101: LATCH, channel 3, noise, data %0101
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(6, regNumLatch);
	// Verify the noise value.
	m_psg->dbg_getReg(6, &val);
	EXPECT_EQ(0x5, val);

	// TODO: Verify that other registers haven't been touched?

	// Set the noise value again, but this time,
	// don't write a LATCH byte.
	m_psg->write(0x04);	// %0 0  000100: DATA, %000100
	m_psg->dbg_getRegNumLatch(&regNumLatch);
	EXPECT_EQ(6, regNumLatch);
	// Verify the noise value.
	m_psg->dbg_getReg(6, &val);
	EXPECT_EQ(0x4, val);

	// TODO: Verify that other registers haven't been touched?
}

} }

/**
 * Test suite main function.
 * Called by gtest_main.inc.cpp's main().
 */
static int test_main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: PSG register test.\n\n");
	LibGens::Init();
	fflush(nullptr);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#include "libcompat/tests/gtest_main.inc.cpp"
