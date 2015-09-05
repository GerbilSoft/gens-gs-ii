/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * TestSuite.cpp: Test Suite base class.                                   *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

// DEPRECATED: All tests deriving from TestSuite should be
// rewritten to use Google Test.

#include "TestSuite.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// C++ includes.
#include <sstream>
#include <string>
#include <iomanip>
using std::string;
using std::ostringstream;

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty(fileno) _isatty(fileno)
#define fileno(stream) _fileno(stream)
#else
#include <unistd.h>
#endif

namespace LibGens {
namespace Tests {

TestSuite::TestSuite()
	: m_tests_total(0)
	, m_tests_failed(0)
	, m_section_total(0)
	, m_section_failed(0)
	, m_is_color(false)
	, m_f_out(NULL)
{
	// Initialize the output stream.
	m_f_out = stderr;

	// Check if the output stream supports color.
	// (Based on Google Test.)
	if (isatty(fileno(m_f_out))) {
		// Output stream is a terminal.
#ifdef _WIN32
		// Always use color on Windows.
		m_is_color = true;
#else /* !_WIN32 */
		// On Unix/Linux, check the terminal
		// to see if it actually supports color.
		const char *const term = getenv("TERM");
		if (term) {
			if (!strcmp(term, "xterm") ||
			    !strcmp(term, "xterm-color") ||
			    !strcmp(term, "xterm-256color") ||
			    !strcmp(term, "screen") ||
			    !strcmp(term, "screen-256color") ||
			    !strcmp(term, "linux") ||
			    !strcmp(term, "cygwin"))
			{
				// Terminal supports color.
				m_is_color = true;
			}
		}
#endif /* _WIN32 */
	}
}

/**
 * Start a new test section.
 */
void TestSuite::newSection(void)
{
	if (m_section_total > 0) {
		fprintf(m_f_out, "Section complete. %d/%d tests passed.\n\n",
			(m_section_total - m_section_failed), m_section_total);
	}

	// Reset the section counters.
	m_section_failed = 0;
	m_section_total = 0;
}

/**
 * Print the test results and indicate the tests are completed.
 */
void TestSuite::testsCompleted(void)
{
	newSection();
	fprintf(m_f_out, "Tests complete. %d/%d tests passed.\n",
		(m_tests_total - m_tests_failed), m_tests_total);
}


// Colorization functions.
// TODO: Switch to fputs()?
#ifndef _WIN32
// ANSI escape sequences.
#define PrintMsg(fn, str, color) \
void TestSuite::Print##fn(void) \
{ \
	if (m_is_color) { \
		fprintf(m_f_out, "[\x1B[01;3" #color "m" str "\x1B[00m" "] "); \
	} else { \
		fprintf(m_f_out, "[" str "] "); \
	} \
}
PrintMsg(Fail, "FAIL", 1)
PrintMsg(Warn, "WARN", 3)
PrintMsg(Pass, "PASS", 2)
PrintMsg(Info, "INFO", 5)
PrintMsg(Unknown, "UNKNOWN", 3)
#else /* _WIN32 */
#define PrintMsg(fn, str, color) \
void TestSuite::Print##fn(void) \
{ \
	if (m_is_color) { \
		fprintf(m_f_out, "["); \
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE); \
		/* Get the current text color. */ \
		CONSOLE_SCREEN_BUFFER_INFO buffer_info; \
		GetConsoleScreenBufferInfo(stdout_handle, &buffer_info); \
		const WORD old_color_attrs = buffer_info.wAttributes; \
		/* Flush the stream buffers to prevent issues when changing attributes. */ \
		fflush(stdout); \
		fflush(stderr); \
		SetConsoleTextAttribute(stdout_handle, color | FOREGROUND_INTENSITY); \
		fprintf(m_f_out, "%s", str); \
		fflush(m_f_out); \
		SetConsoleTextAttribute(stdout_handle, old_color_attrs); \
		fprintf(m_f_out, "] "); \
	} else { \
		fprintf(m_f_out, "[" str "] "); \
	} \
}
// NOTE: RGB bits are inverted compared to ANSI terminals.
PrintMsg(Fail, "FAIL", FOREGROUND_RED)
PrintMsg(Warn, "WARN", FOREGROUND_RED | FOREGROUND_GREEN)
PrintMsg(Pass, "PASS", FOREGROUND_GREEN)
PrintMsg(Info, "INFO", FOREGROUND_RED | FOREGROUND_BLUE)
PrintMsg(Unknown, "UNKNOWN", FOREGROUND_RED | FOREGROUND_GREEN)
#endif /* _WIN32 */

/**
 * Internal function to indicate a test passed.
 * @param expr Stringified expression, or NULL if nothing should be printed.
 */
void TestSuite::assertPass(const char *expr)
{
	m_tests_total++;
	m_section_total++;

	if (expr) {
		PrintPass();
		fprintf(m_f_out, "Test `%s' passed.\n", expr);
	}
}

/**
 * Internal function to indicate a test failed.
 * @param expr Stringified expression, or NULL if nothing should be printed.
 */
void TestSuite::assertFail(const char *expr)
{
	m_tests_total++;
	m_tests_failed++;
	m_section_total++;
	m_section_failed++;

	if (expr) {
		PrintFail();
		fprintf(m_f_out, "Test `%s' failed.\n", expr);
	}
}

/**
 * Check two hex values for equality.
 * @param test Test name.
 * @param expected Expected value.
 * @param actual Actual value.
 * @return True if the values match.
 */
template<typename T>
bool TestSuite::assertEquals_hex(const char *test, T expected, T actual)
{
	m_tests_total++;
	m_section_total++;

	if (expected == actual) {
		PrintPass();
	} else {
		m_tests_failed++;
		m_section_failed++;
		PrintFail();
	}

	// Convert the expected value to a string.
	ostringstream oss;
	oss << std::hex << std::uppercase << std::setfill('0') << std::setw(sizeof(T)*2);
	oss << expected;
	string exp_str = oss.str();

	// Convert the actual value to a string.
	oss.str("");
	oss << std::hex << std::uppercase << std::setfill('0') << std::setw(sizeof(T)*2);
	oss << actual;
	string act_str = oss.str();

	fprintf(m_f_out, "%s: expected %s, got %s\n", test, exp_str.c_str(), act_str.c_str());
	return (expected == actual);
}

// Ensure variants of assertEquals_hex() exist for most types.
template bool TestSuite::assertEquals_hex<uint8_t>(const char *test, uint8_t expected, uint8_t actual);
template bool TestSuite::assertEquals_hex<uint16_t>(const char *test, uint16_t expected, uint16_t actual);
template bool TestSuite::assertEquals_hex<uint32_t>(const char *test, uint32_t expected, uint32_t actual);
template bool TestSuite::assertEquals_hex<uint64_t>(const char *test, uint64_t expected, uint64_t actual);

template bool TestSuite::assertEquals_hex<int8_t>(const char *test, int8_t expected, int8_t actual);
template bool TestSuite::assertEquals_hex<int16_t>(const char *test, int16_t expected, int16_t actual);
template bool TestSuite::assertEquals_hex<int32_t>(const char *test, int32_t expected, int32_t actual);
template bool TestSuite::assertEquals_hex<int64_t>(const char *test, int64_t expected, int64_t actual);


/**
 * Check two values for equality.
 * @param test Test name.
 * @param expected Expected value.
 * @param actual Actual value.
 * @return True if the values match.
 */
template<typename T>
bool TestSuite::assertEquals(const char *test, T expected, T actual)
{
	m_tests_total++;
	m_section_total++;

	if (expected == actual) {
		PrintPass();
	} else {
		m_tests_failed++;
		m_section_failed++;
		PrintFail();
	}

	// Convert the expected value to a string.
	ostringstream oss;
	oss << expected;
	string exp_str = oss.str();

	// Convert the actual value to a string.
	oss.str("");
	oss << actual;
	string act_str = oss.str();

	fprintf(m_f_out, "%s: expected %s, got %s\n", test, exp_str.c_str(), act_str.c_str());
	return (expected == actual);
}

// Ensure variants of assertEquals_hex() exist for most types.
template bool TestSuite::assertEquals<uint8_t>(const char *test, uint8_t expected, uint8_t actual);
template bool TestSuite::assertEquals<uint16_t>(const char *test, uint16_t expected, uint16_t actual);
template bool TestSuite::assertEquals<uint32_t>(const char *test, uint32_t expected, uint32_t actual);
template bool TestSuite::assertEquals<uint64_t>(const char *test, uint64_t expected, uint64_t actual);

template bool TestSuite::assertEquals<int8_t>(const char *test, int8_t expected, int8_t actual);
template bool TestSuite::assertEquals<int16_t>(const char *test, int16_t expected, int16_t actual);
template bool TestSuite::assertEquals<int32_t>(const char *test, int32_t expected, int32_t actual);
template bool TestSuite::assertEquals<int64_t>(const char *test, int64_t expected, int64_t actual);

} }
