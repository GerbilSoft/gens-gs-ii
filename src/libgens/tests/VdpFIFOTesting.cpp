/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * VdpFIFOTesting.cpp: Sprite Masking & Overflow Test ROM.           *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
 * Original ROM Copyright (c) 2009 by Nemesis.                             *
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

// LibGens VDP.
#include "MD/EmuMD.hpp"
#include "Rom.hpp"

#include "Vdp/Vdp.hpp"
#include "cpu/M68K.hpp"
#include "libzomg/zomg_m68k.h"
#include "MD/SysVersion.hpp"
#include "Util/byteswap.h"

// ARRAY_SIZE(x)
#include "macros/common.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
using namespace std;

// ZLib.
#define CHUNK 4096
#include <zlib.h>

namespace LibGens { namespace Tests {

struct VdpFIFOTesting_mode
{
	int testNum;
	const char *name;
	int lineStart;		// Tile position.
	int linesToCheck;	// Number of rows of tiles.

	// If true, go to the next prompt before checking this test.
	bool goToNextPrompt;

	VdpFIFOTesting_mode()
		: testNum(-1)
		, name("Unknown test")
		, lineStart(0)
		, linesToCheck(0)
		, goToNextPrompt(false) { }

	VdpFIFOTesting_mode(int testNum, const char *name, int lineStart, int linesToCheck, bool goToNextPrompt)
		: testNum(testNum)
		, name(name)
		, lineStart(lineStart)
		, linesToCheck(linesToCheck)
		, goToNextPrompt(goToNextPrompt) { }
};

/**
 * Formatting function for VdpFIFOTesting_mode.
 */
inline ::std::ostream& operator<<(::std::ostream& os, const VdpFIFOTesting_mode& mode)
{
	return os << mode.testNum << ". " << mode.name;
};

class VdpFIFOTesting : public ::testing::TestWithParam<VdpFIFOTesting_mode>
{
	protected:
		VdpFIFOTesting() { }
		virtual ~VdpFIFOTesting() { }

		virtual void SetUp(void) override;

	protected:
		static EmuMD *m_context;
		static Rom *m_rom;

	public:
		/**
		 * Channel value must be < this number
		 * to be considered "off".
		 */
		static const uint8_t PX_MIN_THRESH = 0x20;

		/**
		 * Channel value must be >= this number
		 * to be considered "on".
		 */
		static const uint8_t PX_MAX_THRESH = 0xE0;

	protected:
		/**
		 * Run the ROM until it prompts for controller input.
		 */
		void runUntilPrompt(void);
};

EmuMD *VdpFIFOTesting::m_context = nullptr;
Rom *VdpFIFOTesting::m_rom = nullptr;

/**
 * Set up the emulation context for testing.
 */
void VdpFIFOTesting::SetUp(void)
{
	// Initialize the emulation context.
	static const utf8_str filename[] = "VDPFIFOTesting.bin";

	// NOTE: A modified version of the ROM with the controller check
	// NOP'd out is required.
	// At address $000F46, write: 70 00 4E 71
	// TODO: Add internal copy of VDPFIFOTesting.bin.
	// TODO: Add function to load a ROM image from RAM.

	// TODO: Add debug functions to RomCartridgeMD.
	if (!m_context) {
		// VDP hasn't been created yet.
		m_rom = new LibGens::Rom(filename);
		EXPECT_TRUE(m_rom->isOpen());
		if (!m_rom->isOpen()) {
			delete m_rom;
			m_rom = nullptr;
			ASSERT_TRUE(false) << "Cannot continue without ROM file: " << filename;
		}
		EXPECT_FALSE(m_rom->isMultiFile());
		if (m_rom->isMultiFile()) {
			delete m_rom;
			m_rom = nullptr;
			ASSERT_TRUE(false) << filename << " is multi-file; cannot continue.";
		}

		m_context = new LibGens::EmuMD(m_rom);
		m_rom->close();	// TODO: Let EmuMD handle this...

		if (!m_context->isRomOpened()) {
			// Error loading the ROM image in EmuMD.
			// TODO: EmuMD error code constants.
			// TODO: Show an error message.
			delete m_context;
			m_context = nullptr;
			delete m_rom;
			m_rom = nullptr;
			ASSERT_TRUE(false) << "Emulation context failed to initialize; cannot continue.";
		}

		// FIXME: Move MD_Screen out of m_vdp.
		m_context->m_vdp->MD_Screen->setBpp(LibGens::MdFb::BPP_32);
	}

}

/**
 * Run the ROM until it prompts for controller input.
 */
void VdpFIFOTesting::runUntilPrompt(void)
{
	/**
	 * Relevant ROM addresses:
	 * - $000F46: DisplayProgressiveResultsScreenWaitForInput
	 * - $000F9A: DisplayProgressiveResultsScreenEntriesFinished
	 */
	static const uint32_t DisplayProgressiveResultsScreenWaitForInput = 0x000F46;
	static const uint32_t DisplayProgressiveResultsScreenEntriesFinished = 0x000F9A;

	// Run until we get to a prompt.
	// NOTE: Wait until 5 frames pass in order for the screen
	// to be updated completely, since the display might not
	// be fully updated on the first frame.
	Zomg_M68KRegSave_t reg;
	int promptCount = 0;
	while (promptCount < 5) {
		m_context->execFrame();
		// TODO: M68K should be part of m_context, not static.
		// TODO: Make register accessors instead of doing a whole context save.
		M68K::ZomgSaveReg(&reg);

		if (reg.pc >= DisplayProgressiveResultsScreenWaitForInput &&
		    reg.pc < DisplayProgressiveResultsScreenEntriesFinished)
		{
			// At a prompt.
			promptCount++;
			continue;
		}
	}

	// Go to the next screen.
	reg.pc = DisplayProgressiveResultsScreenEntriesFinished;
	M68K::ZomgRestoreReg(&reg);
}

/**
 * Run the VDP FIFO test.
 * TODO: Run all tests at first, then have one TEST_P for each test?
 */
TEST_P(VdpFIFOTesting, testFifo)
{
	VdpFIFOTesting_mode mode = GetParam();
	if (mode.goToNextPrompt)
		runUntilPrompt();

	/**
	 * Verify all pixels in the specified rows.
	 * Pixel colors should be:
	 * - Red only: Fail
	 * - Green only: Pass
	 * - Gray (all channels equal): Ignore
	 * - Other: Fail.
	 * Test passes only if all pixels are either green or gray.
	 */
	LibGens::MdFb *fb = m_context->m_vdp->MD_Screen;
	const int y_adj = ((240 - m_context->m_vdp->getVPix()) / 2);
	bool testFailed = false;
	for (int row = mode.lineStart; row < mode.lineStart+mode.linesToCheck && !testFailed; row++) {
		const int y_start = y_adj+(row*8);
		for (int y = y_start; y < (y_start+7) && !testFailed; y++) {
			const uint32_t *line = fb->lineBuf32(y) + 16;
			for (int x = 272-16; x > 0; x--, line++) {
				const uint8_t b = (*line & 0xFF);
				const uint8_t g = ((*line >> 8) & 0xFF);
				const uint8_t r = ((*line >> 16) & 0xFF);
				if (x == (272-16)) {
					// First pixel on the line cannot be blank.
					ASSERT_FALSE(r == 0 && g == 0 && b == 0) << "MD_Screen is blank. Test is broken.";
				}

				// Check the pixel color.
				if (r == 0 && g > 0 && b == 0) {
					// Green. Test is still passing so far...
					continue;
				} else if (r == g && g == b) {
					// Gray. Ignore this pixel.
					continue;
				} else if (r > 0 && g == 0 && b == 0) {
					// Red. Test failed.
					testFailed = true;
					break;
				} else {
					// Other color. Test failed.
					testFailed = false;
					break;
				}
			}
		}
	}

	ASSERT_FALSE(testFailed) << "Check the FIFO test ROM to see how this test failed.";
}

// Test cases.
INSTANTIATE_TEST_CASE_P(Page1, VdpFIFOTesting,
	::testing::Values(
		VdpFIFOTesting_mode(1, "FIFO Buffer Size", 4, 1, true),
		VdpFIFOTesting_mode(2, "Separate FIFO Read/Write Buffer", 6, 1, false),
		VdpFIFOTesting_mode(3, "DMA Transfer using FIFO", 8, 1, false),
		VdpFIFOTesting_mode(4, "DMA Fill FIFO Usage", 10, 1, false),
		VdpFIFOTesting_mode(5, "FIFO Write to invalid target", 12, 1, false),
		VdpFIFOTesting_mode(6, "8-bit VRAM Read target 01100", 14, 1, false),
		VdpFIFOTesting_mode(7, "VRAM Byteswapping", 16, 1, false),
		VdpFIFOTesting_mode(8, "CRAM Byteswapping", 18, 1, false),
		VdpFIFOTesting_mode(9, "VSRAM Byteswapping", 20, 1, false)
		));

INSTANTIATE_TEST_CASE_P(Page2, VdpFIFOTesting,
	::testing::Values(
		VdpFIFOTesting_mode(10, "Partial CP Writes", 4, 1, true),
		VdpFIFOTesting_mode(11, "Register Write Bit13 Masked", 6, 1, false),
		VdpFIFOTesting_mode(12, "Register Write Mode4 Mask", 8, 1, false),
		VdpFIFOTesting_mode(13, "Register Writes and Code Reg", 10, 1, false),
		VdpFIFOTesting_mode(14, "CP Write Pending Reset", 12, 1, false),
		VdpFIFOTesting_mode(15, "Read target switching", 14, 1, false),
		VdpFIFOTesting_mode(16, "FIFO Wait States", 16, 3, false)
		));

INSTANTIATE_TEST_CASE_P(Page3, VdpFIFOTesting,
	::testing::Values(
		VdpFIFOTesting_mode(17, "HV Counter Latch", 4, 6, true),
		VdpFIFOTesting_mode(18, "HBlank/VBlank flags", 8, 1, false),
		VdpFIFOTesting_mode(19, "DMA Transfer Bus Locking", 10, 4, false)
		));

INSTANTIATE_TEST_CASE_P(Page4, VdpFIFOTesting,
	::testing::Values(
		VdpFIFOTesting_mode(20, "DMA Transfer Source Wrapping", 4, 1, true),
		VdpFIFOTesting_mode(21, "DMA Transfer to VRAM Wrapping", 6, 1, false),
		VdpFIFOTesting_mode(22, "DMA Transfer to CRAM Wrapping", 8, 1, false),
		VdpFIFOTesting_mode(23, "DMA Transfer to VSRAM Wrapping", 10, 1, false),
		VdpFIFOTesting_mode(24, "DMA Transfer Length Reg Update", 12, 1, false),
		VdpFIFOTesting_mode(25, "DMA Fill Length Reg Update", 14, 1, false),
		VdpFIFOTesting_mode(26, "DMA Copy Length Reg Update", 16, 1, false),
		VdpFIFOTesting_mode(27, "DMA Transfer Source Reg Update", 18, 1, false),
		VdpFIFOTesting_mode(28, "DMA Fill Source Reg Update", 18, 1, false),
		VdpFIFOTesting_mode(29, "DMA Copy Source Reg Update", 20, 1, false)
		));
} }

int main(int argc, char *argv[])
{
	// NOTE: Tests cannot be run out of order.
	// TODO: Enforce that.
	fprintf(stderr, "LibGens test suite: VDP FIFO tests.\n\n");

	::testing::InitGoogleTest(&argc, argv);
	LibGens::Init();
	fprintf(stderr, "\n");
	fprintf(stderr, "LibGens: VDP FIFO Testing ROM.\n"
			"Original ROM (c) 2013 by Nemesis.\n\n");
	return RUN_ALL_TESTS();
}
