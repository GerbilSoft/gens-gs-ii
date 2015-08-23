/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * AudioWriteTest.cpp: SoundMgr audio write test.                          *
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

#include "AudioWriteTest.hpp"

// Google Test
#include "gtest/gtest.h"

// LibGens.
#include "lg_main.hpp"
#include "Util/cpuflags.h"

// Sound Manager
#include "sound/SoundMgr.hpp"

// Test data.
#include "AudioWriteTest_data.h"

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cstring>

// TODO: More sample data:
// 48,000 Hz @ 50 Hz (960 samples)
// 44,100 Hz @ 60 Hz (735 samples)
// 44,100 Hz @ 50 Hz (882 samples)
// The latter two are important because they test data blocks
// that aren't multiples of 8 or 16 bytes. (MMX/SSE2)

namespace LibGens { namespace Tests {

class AudioWriteTest : public ::testing::TestWithParam<AudioWriteTest_flags>
{
	protected:
		AudioWriteTest()
			: ::testing::TestWithParam<AudioWriteTest_flags>()
			, buf(nullptr) { }
		virtual ~AudioWriteTest() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		static const int rate;
		static const int samples;

		// Aligned destination buffer.
		int16_t *buf;

		// Previous CPU flags.
		uint32_t cpuFlags_old;
};

const int AudioWriteTest::rate = 48000;
const int AudioWriteTest::samples = 800;

/**
 * Set up the SoundMgr for testing.
 */
void AudioWriteTest::SetUp(void)
{
	// Verify CPU flags.
	AudioWriteTest_flags flags = GetParam();
	uint32_t totalFlags = (flags.cpuFlags | flags.cpuFlags_slow);
	if (flags.cpuFlags != 0) {
		ASSERT_NE(0U, CPU_Flags & totalFlags) <<
			"CPU does not support the required flags for this test.";
	}
	// NOTE: We're not going to show a slow CPU warning,
	// since this isn't a benchmark test.
	cpuFlags_old = CPU_Flags;
	CPU_Flags = flags.cpuFlags;

	// Initialize SoundMgr.
	SoundMgr::ReInit(rate, false);

	// Allocate an aligned destination buffer.
	buf = (int16_t*)aligned_malloc(16, samples * 2 * sizeof(*buf));

	// Copy the test data into SoundMgr.
	memcpy(SoundMgr::ms_SegBufL, AudioWriteTest_Input_L, sizeof(AudioWriteTest_Input_L));
	memcpy(SoundMgr::ms_SegBufR, AudioWriteTest_Input_R, sizeof(AudioWriteTest_Input_R));
}

/**
 * Tear down the test.
 */
void AudioWriteTest::TearDown(void)
{
	CPU_Flags = cpuFlags_old;
	aligned_free(buf);
}

/**
 * Test SoundMgr::writeStereo().
 */
TEST_P(AudioWriteTest, writeStereo)
{
	int ret = SoundMgr::writeStereo(buf, samples);
	ASSERT_EQ(samples, ret);

	// Verify the data.
	const int16_t *expected = AudioWriteTest_Output_Stereo;
	for (int i = 0; i < samples*2; i++) {
		EXPECT_EQ(expected[i], buf[i]) <<
			"Output sample " << i << " should be " <<
			std::hex << std::uppercase <<
			std::setfill('0') << std::setw(4) <<
			expected[i] << ", but was " << buf[i];
	}
}

/**
 * Test SoundMgr::writeMono().
 */
TEST_P(AudioWriteTest, writeMono)
{
	int ret = SoundMgr::writeMono(buf, samples);
	ASSERT_EQ(samples, ret);

	// Verify the data.
	const int16_t *expected = AudioWriteTest_Output_Mono_fast;
	for (int i = 0; i < samples; i++) {
		EXPECT_EQ(expected[i], buf[i]) <<
			"Output sample " << i << " should be " <<
			std::hex << std::uppercase <<
			std::setfill('0') << std::setw(4) <<
			expected[i] << ", but was " << buf[i];
	}
}

// Test cases.

INSTANTIATE_TEST_CASE_P(AudioWriteTest_NoFlags, AudioWriteTest,
	::testing::Values(AudioWriteTest_flags(0, 0)
));

// NOTE: SoundMgr only implements MMX/SSE2 using GNU assembler.
#if defined(__i386__) || defined(__amd64__)
INSTANTIATE_TEST_CASE_P(AudioWriteTest_MMX, AudioWriteTest,
	::testing::Values(AudioWriteTest_flags(MDP_CPUFLAG_X86_MMX, 0)
));
INSTANTIATE_TEST_CASE_P(AudioWriteTest_SSE2, AudioWriteTest,
	::testing::Values(AudioWriteTest_flags(MDP_CPUFLAG_X86_SSE2, MDP_CPUFLAG_X86_SSE2SLOW)
));
#endif

} }

int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: SoundMgr audio write test.\n\n");
	LibGens::Init();
	fflush(nullptr);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
