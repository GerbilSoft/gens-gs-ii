/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * Z80Test.cpp: ZEXDOC/ZEXALL using a minimal CP/M emulator.               *
 *                                                                         *
 * Copyright (c) 2014-2015 by David Korth.                                 *
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

#include "lg_main.hpp"

#include "cz80/cz80.h"

// C includes.
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <io.h>
#else /* !_WIN32 */
#include <unistd.h>
#endif /* _WIN32 */

// NOTE: This test suite uses CZ80 directly.
// The Z80 class is currently hard-coded for MD only.

namespace LibGens { namespace Tests {

class Z80Test : public ::testing::Test
{
	protected:
		Z80Test()
			: use_color(false)
			, is_red(false)
			, has_err_occurred(false)
#ifdef _WIN32
			, old_attr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#endif /* _WIN32 */
		{ }

		virtual ~Z80Test() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		cz80_struc m_Z80;

		// Z80 RAM.
		uint8_t Ram_Z80[65536];

		// System state.
		bool halt;

		static uint8_t CZ80CALL Z80_ReadB_static(void *ctx, uint16_t address) {
			return ((Z80Test*)ctx)->Z80_ReadB(address);
		}
		static void CZ80CALL Z80_WriteB_static(void *ctx, uint16_t address, uint8_t data) {
			((Z80Test*)ctx)->Z80_WriteB(address, data);
		}

		uint8_t Z80_ReadB(uint16_t address);
		void Z80_WriteB(uint16_t address, uint8_t data);

		static uint8_t CZ80CALL Z80_INPort_static(void *ctx, uint16_t address) {
			return ((Z80Test*)ctx)->Z80_INPort(address);
		}
		static void CZ80CALL Z80_OUTPort_static(void *ctx, uint16_t address, uint8_t data) {
			((Z80Test*)ctx)->Z80_OUTPort(address, data);
		}

		uint8_t Z80_INPort(uint16_t address);
		void Z80_OUTPort(uint16_t address, uint8_t data);

		// Colorized output
		static bool ShouldUseColor(bool stdout_is_tty);
		void setTextColor(bool do_red);
		bool use_color;         // True if we should use color.
		bool is_red;            // True if currently printing red text.
		bool has_err_occurred;  // True if any red text has been printed.
#ifdef _WIN32
		WORD old_attr;          // Previous character attributes. (Win32 only)
#endif /* _WIN32 */
};

/**
 * Should we use color in terminal output?
 * Based on the internal function from gtest-1.7.0.
 * @param stdout_is_tty True if stdout is a TTY.
 * @return True if we should use color; false if not.
 */
bool Z80Test::ShouldUseColor(bool stdout_is_tty)
{
	const char* const gtest_color = testing::GTEST_FLAG(color).c_str();

	if (!strcasecmp(gtest_color, "auto")) {
#ifdef _WIN32
		// On Windows the TERM variable is usually not set, but the
		// console there does support colors.
		return stdout_is_tty;
#else /* !_WIN32 */
		// On non-Windows platforms, we rely on the TERM variable.
		const char* const term = getenv("TERM");
		const bool term_supports_color =
			!strcmp(term, "xterm") ||
			!strcmp(term, "xterm-color") ||
			!strcmp(term, "xterm-256color") ||
			!strcmp(term, "screen") ||
			!strcmp(term, "screen-256color") ||
			!strcmp(term, "linux") ||
			!strcmp(term, "cygwin");
		return stdout_is_tty && term_supports_color;
#endif /* WIN32 */
	}

	return !strcasecmp(gtest_color, "yes") ||
		!strcasecmp(gtest_color, "true") ||
		!strcasecmp(gtest_color, "t") ||
		!strcmp(gtest_color, "1");
	// We take "yes", "true", "t", and "1" as meaning "yes".  If the
	// value is neither one of these nor "auto", we treat it as "no" to
	// be conservative.
}

/**
 * Set the text color.
 * @param do_red If true, make the text red; otherwise, revert to standard colors.
 */
void Z80Test::setTextColor(bool do_red)
{
	if (is_red == do_red)
		return;
	is_red = do_red;
	if (!use_color) {
		// Color is disabled.
		if (do_red) {
			// Make sure we still mark that an error occurred.
			has_err_occurred = true;
		}
		return;
	}

#ifdef _WIN32
	const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (do_red) {
		// Make the text red.
		has_err_occurred = true;

		// Gets the current text color.
		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
		old_attr = buffer_info.wAttributes;

		// We need to flush the stream buffers into the console before each
		// SetConsoleTextAttribute call lest it affect the text that is already
		// printed but has not yet reached the console.
		fflush(stdout);
		SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
	} else {
		// Restore the previous text color.
		SetConsoleTextAttribute(stdout_handle, old_attr);
	}
#else /* !_WIN32 */
	if (do_red) {
		has_err_occurred = true;
		printf("\033[0;31m");
	} else {
		printf("\033[m");  // Resets the terminal to default.
	}
#endif /* _WIN32 */
}

void Z80Test::SetUp()
{
	// Determine if ZEXALL output should be colorized.
	use_color = ShouldUseColor(!!isatty(fileno(stdout)));
	is_red = false;
	has_err_occurred = false;
#ifdef _WIN32
	old_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#endif /* _WIN32 */

	// Not halted initially.
	halt = false;

	// Initialize Z80 memory.
	memset(Ram_Z80, 0, sizeof(Ram_Z80));
	// 0: Warm boot code - JP $F000
	Ram_Z80[0] = 0xC3;
	Ram_Z80[1] = 0x00;
	Ram_Z80[2] = 0xF0;
	// 5: SYSCALL - JP $F400
	Ram_Z80[5] = 0xC3;
	Ram_Z80[6] = 0x00;
	Ram_Z80[7] = 0xF0;

	// Load BIOS.
	FILE *f = fopen("bios.bin", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0xF000], 1, 1068, f);
	fclose(f);

	// Initialize the Z80 emulator for CP/M.
	Cz80_Init(&m_Z80);
	Cz80_Set_Ctx(&m_Z80, this);

	// Set fetch, memory, and I/O handlers.
	Cz80_Set_Fetch(&m_Z80, 0x0000, 0xFFFF, Ram_Z80);
	Cz80_Set_ReadB(&m_Z80, Z80_ReadB_static);
	Cz80_Set_WriteB(&m_Z80, Z80_WriteB_static);
	Cz80_Set_INPort(&m_Z80, Z80_INPort_static);
	Cz80_Set_OUTPort(&m_Z80, Z80_OUTPort_static);

	Cz80_Reset(&m_Z80);
}

void Z80Test::TearDown(void)
{
	halt = false;
}

// Memory read/write functions.

uint8_t Z80Test::Z80_ReadB(uint16_t address)
{
	return Ram_Z80[address & 0xFFFF];
}

void Z80Test::Z80_WriteB(uint16_t address, uint8_t data)
{
	Ram_Z80[address & 0xFFFF] = data;
}

// I/O functions.

uint8_t Z80Test::Z80_INPort(uint16_t address)
{
	// No input ports...
	((void)address);
	return 0xFF;
}

void Z80Test::Z80_OUTPort(uint16_t address, uint8_t data)
{
	switch (address & 0xFF) {
		case 0x01:	// stdout
		case 0x02:	// stderr
			// NOTE: CP/M doesn't actually have stderr.
			if (data == 'E') {
				// Start of "ERROR" text.
				// Should be printed in red.
				setTextColor(true);
			} else if (data == '\r' || data == '\n') {
				// End of the line.
				// Revert back to normal text colors.
				setTextColor(false);
			}
			fputc(data, stdout);
			fflush(stdout);
			break;

		case 0xFF:
			// End of program.
			halt = true;
			setTextColor(false);
			fflush(stdout);
			break;
	}
}

/** Test cases. **/

/**
 * Run ZEXDOC.
 */
TEST_F(Z80Test, zexdoc)
{
	// Load ZEXDOC.
	FILE *f = fopen("zexdoc.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Reset the Z80.
	Cz80_Reset(&m_Z80);

	// Run the Z80 until it's halted.
	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !(m_Z80.Status & CZ80_HALTED)) {
		Cz80_Release_Cycle(&m_Z80);
		if (Cz80_Exec(&m_Z80, 100) < 0)
			break;
	}
	printf("\n");

	// If any errors have occurred, fail the test.
	ASSERT_FALSE(has_err_occurred);
}

/**
 * Run ZEXALL.
 */
TEST_F(Z80Test, zexall)
{
	// Load ZEXALL.
	FILE *f = fopen("zexall.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Reset the Z80.
	Cz80_Reset(&m_Z80);

	// Run the Z80 until it's halted.
	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !(m_Z80.Status & CZ80_HALTED)) {
		Cz80_Release_Cycle(&m_Z80);
		if (Cz80_Exec(&m_Z80, 100) < 0)
			break;
	}
	printf("\n");

	// If any errors have occurred, fail the test.
	ASSERT_FALSE(has_err_occurred);
}

} }


int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: Z80 tests.\n");
	fprintf(stderr, "Using the CZ80 emulator.\n\n");

	::testing::InitGoogleTest(&argc, argv);
	//LibGens::Init(); /* not needed for Z80 tests */
	fflush(nullptr);

	int ret = RUN_ALL_TESTS();
	//LibGens::End(); /* not needed for Z80 tests */
	return ret;
}
