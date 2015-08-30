/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * PausedEffectTest.hpp: Paused Effect test. (Common header)               *
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

#ifndef __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP
#define __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP

// Google Test
#include "gtest/gtest.h"

// C includes.
#include <stdint.h>

// LibGens, LibZomg
#include "Util/MdFb.hpp"
#include "libzomg/img_data.h"

namespace LibGens { namespace Tests {

struct PausedEffectTest_flags {
	uint32_t cpuFlags;
	uint32_t cpuFlags_slow;

	PausedEffectTest_flags(uint32_t cpuFlags, uint32_t cpuFlags_slow)
	{
		this->cpuFlags = cpuFlags;
		this->cpuFlags_slow = cpuFlags_slow;
	}
};

class PausedEffectTest : public ::testing::TestWithParam<PausedEffectTest_flags>
{
	protected:
		PausedEffectTest()
			: ::testing::TestWithParam<PausedEffectTest_flags>()
			, fb_normal(nullptr)
			, fb_paused(nullptr)
			, fb_test1(nullptr)
			, fb_test2(nullptr) { }
		virtual ~PausedEffectTest() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

		/**
		 * Initialize image data.
		 * @param bpp Color depth.
		 */
		void init(MdFb::ColorDepth bpp);

		/**
		 * Copy a loaded image to an MdFb in 15-bit color.
		 * Source image must be 32-bit color.
		 * @param fb	[out] Destination MdFb.
		 * @param src	[in] Source img_data.
		 */
		void copyToFb15(MdFb *fb, const Zomg_Img_Data_t *src);

		/**
		 * Copy a loaded image to an MdFb in 16-bit color.
		 * Source image must be 32-bit color.
		 * @param fb	[out] Destination MdFb.
		 * @param src	[in] Source img_data.
		 */
		void copyToFb16(MdFb *fb, const Zomg_Img_Data_t *src);

		/**
		 * Copy a loaded image to an MdFb in 32-bit color.
		 * Source image must be 32-bit color.
		 * @param fb	[out] Destination MdFb.
		 * @param src	[in] Source img_data.
		 */
		void copyToFb32(MdFb *fb, const Zomg_Img_Data_t *src);

		/**
		 * Compare two framebuffers.
		 * Both framebuffers must have the same color depth.
		 * @param fb_expected	[in] Expected image.
		 * @param fb_actual	[in] Actual image.
		 */
		void compareFb(const MdFb *fb_expected, const MdFb *fb_actual);

	protected:
		Zomg_Img_Data_t img_normal;
		Zomg_Img_Data_t img_paused;

		// Reference framebuffers.
		MdFb *fb_normal;
		MdFb *fb_paused;

		// Test framebuffers.
		// Paused Effect is applied here.
		MdFb *fb_test1;	// 1-FB
		MdFb *fb_test2;	// 2-FB

		// Previous CPU flags.
		uint32_t cpuFlags_old;
};

} }

#endif /* __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP */
