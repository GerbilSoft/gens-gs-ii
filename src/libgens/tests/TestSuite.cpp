/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * TestSuite.cpp: Test Suite base class.                                   *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#include "TestSuite.hpp"

// C includes. (C++ namespace)
#include <cstdio>
using namespace std;

namespace LibGens {
namespace Tests {

TestSuite::TestSuite()
	: m_tests_total(0)
	, m_tests_failed(0)
	, m_section_total(0)
	, m_section_failed(0)
{ }


/**
 * Start a new test section.
 */
void TestSuite::newSection(void)
{
	if (m_section_total > 0)
	{
		fprintf(stderr, "Section complete. %d/%d tests passed.\n\n",
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
	fprintf(stderr, "Tests complete. %d/%d tests passed.\n",
		(m_tests_total - m_tests_failed), m_tests_total);
}


// Colorization functions.
#ifndef _WIN32
// ANSI escape sequences.
#define ANSI_ESC_COLOR(x)	"\x1B[01;3" #x "m"
#define ANSI_ESC_END		"\x1B[00m"
void TestSuite::PrintFail(FILE *f)
	{ fprintf(f, "[" ANSI_ESC_COLOR(1) "FAIL" ANSI_ESC_END "] "); }
void TestSuite::PrintWarn(FILE *f)
	{ fprintf(f, "[" ANSI_ESC_COLOR(2) "WARN" ANSI_ESC_END "] "); }
void TestSuite::PrintPass(FILE *f)
	{ fprintf(f, "[" ANSI_ESC_COLOR(5) "PASS" ANSI_ESC_END "] "); }
#undef ANSI_ESC_COLOR
#undef ANSI_ESC_END
#else /* _WIN32 */
// TODO: Add Win32 support.
void TestSuite::PrintFail(FILE *f)
	{ fprintf(f, "[FAIL] "); }
void TestSuite::PrintWarn(FILE *f)
	{ fprintf(f, "[WARN] "); }
void TestSuite::PrintPass(FILE *f)
	{ fprintf(f, "[PASS] "); }
#endif /* _WIN32 */


/**
 * Internal function to indicate a test passed.
 * @param expr Stringified expression, or NULL if nothing should be printed.
 */
void TestSuite::assertPass(const char *expr)
{
	m_tests_total++;
	m_section_total++;
	
	if (expr)
	{
		PrintPass(stderr);
		fprintf(stderr, "Test `%s' passed.\n", expr);
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
	
	if (expr)
	{
		PrintFail(stderr);
		fprintf(stderr, "Test `%s' failed.\n", expr);
	}
}


/**
 * Check two uint32_t values for equality. (hexadecimal output)
 * @param test Test name.
 * @param expected Expected value.
 * @param actual Actual value.
 */
void TestSuite::assertEquals_u32x(const char *test, uint32_t expected, uint32_t actual)
{
	m_tests_total++;
	m_section_total++;
	
	if (expected == actual)
		PrintPass(stderr);
	else
	{
		m_tests_failed++;
		m_section_failed++;
		PrintFail(stderr);
	}
	
	fprintf(stderr, "%s: expected %08X, got %08X\n",
		test, expected, actual);
}

} }
