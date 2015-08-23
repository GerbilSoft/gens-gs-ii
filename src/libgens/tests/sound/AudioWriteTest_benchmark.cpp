/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * AudioWriteTest_benchmark.cpp: AudioWriteTest benchmarks.                *
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

// Google Test
#include "gtest/gtest.h"

// LibGens.
#include "lg_main.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Sound Manager
#include "sound/SoundMgr.hpp"

// Test data.
#include "AudioWriteTest_data.h"

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

// TODO: More sample data:
// 48,000 Hz @ 50 Hz (960 samples)
// 44,100 Hz @ 60 Hz (735 samples)
// 44,100 Hz @ 50 Hz (882 samples)
// The latter two are important because they test data blocks
// that aren't multiples of 8 or 16 bytes. (MMX/SSE2)

namespace LibGens { namespace Tests {

class AudioWriteTest_benchmark : public ::testing::Test
{
	protected:
		AudioWriteTest_benchmark()
			: ::testing::Test() { }
		virtual ~AudioWriteTest_benchmark() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		static const int rate;
		static const int samples;

		// Aligned destination buffer.
		int16_t *buf;
};

const int AudioWriteTest_benchmark::rate = 48000;
const int AudioWriteTest_benchmark::samples = 800;

/**
 * Set up the SoundMgr for testing.
 */
void AudioWriteTest_benchmark::SetUp(void)
{
	// Initialize SoundMgr.
	SoundMgr::ReInit(rate, false);

	// Allocate an aligned destination buffer.
	buf = (int16_t*)aligned_malloc(16, samples * 2 * sizeof(*buf));
}

/**
 * Tear down the test.
 */
void AudioWriteTest_benchmark::TearDown(void)
{
	aligned_free(buf);
}

/**
 * Benchmark SoundMgr::writeStereo().
 */
TEST_F(AudioWriteTest_benchmark, writeStereo)
{
	// Run this test 1,000,000 times.
	for (int i = 1000000; i > 0; i--) {
		// Copy the test data into SoundMgr.
		// Note that this has to be done here instead of in SetUp(),
		// since the segment buffer is erased after every iteration.
		memcpy(SoundMgr::ms_SegBufL, AudioWriteTest_Input_L, sizeof(AudioWriteTest_Input_L));
		memcpy(SoundMgr::ms_SegBufR, AudioWriteTest_Input_R, sizeof(AudioWriteTest_Input_R));

		int ret = SoundMgr::writeStereo(buf, samples);
		ASSERT_EQ(samples, ret);
	}
}

/**
 * Benchmark SoundMgr::writeMono().
 */
TEST_F(AudioWriteTest_benchmark, writeMono)
{
	// Run this test 1,000,000 times.
	for (int i = 1000000; i > 0; i--) {
		// Copy the test data into SoundMgr.
		// Note that this has to be done here instead of in SetUp(),
		// since the segment buffer is erased after every iteration.
		memcpy(SoundMgr::ms_SegBufL, AudioWriteTest_Input_L, sizeof(AudioWriteTest_Input_L));
		memcpy(SoundMgr::ms_SegBufR, AudioWriteTest_Input_R, sizeof(AudioWriteTest_Input_R));

		int ret = SoundMgr::writeMono(buf, samples);
		ASSERT_EQ(samples, ret);
	}
}

} }
