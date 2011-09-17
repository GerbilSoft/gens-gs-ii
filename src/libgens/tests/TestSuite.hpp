/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * TestSuite.hpp: Test Suite base class.                                   *
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

#ifndef __LIBGENS_TESTS_TESTSUITE_HPP__
#define __LIBGENS_TESTS_TESTSUITE_HPP__

// C includes. (C++ namespace)
#include <cstdio>

/**
 * Run a test on a given expression.
 * If the expression is true, the test passes. (No text is printed.)
 * Otherwise, the test fails and an error message is printed.
 * @param expr Expression to check.
 */
#define TESTSUITE_ASSERT(expr) \
	if (expr) assertPass(0); \
	else      assertFail(#expr);

namespace LibGens
{

class TestSuite
{
	public:
		TestSuite();
		
		/**
		 * Execute the test suite.
		 * @return 0 on success; negative on fatal error; positive if tests failed.
		 */
		virtual int exec(void) = 0;
		
		/**
		 * Get the number of passed tests.
		 */
		int testsPassed(void) const;
		
		/**
		 * Get the number of failed tests.
		 */
		int testsFailed(void) const;
		
		/**
		 * Get the total number of tests.
		 */
		int testsTotal(void) const;
	
	protected:
		/**
		 * Start a new test section.
		 */
		void newSection(void);
		
		/**
		 * Print the test results and indicate the tests are completed.
		 */
		void testsCompleted(void);
		
		static void PrintFail(FILE *f);
		static void PrintWarn(FILE *f);
		static void PrintPass(FILE *f);
		
		/**
		 * Internal function to indicate a test passed.
		 * @param expr Stringified expression, or NULL if nothing should be printed.
		 */
		void assertPass(const char *expr);
		
		/**
		 * Internal function to indicate a test failed.
		 * @param expr Stringified expression, or NULL if nothing should be printed.
		 */
		void assertFail(const char *expr);
	
	private:
		/** All tests. **/
		
		int m_tests_total;	// Total number of tests executed.
		int m_tests_failed;	// Total number of tests failed.
		
		/** Test section. **/
		
		int m_section_total;	// Number of tests executed in this section.
		int m_section_failed;	// Number of tests failed in this section.
};

/**
 * Get the number of passed tests.
 */
inline int TestSuite::testsPassed(void) const
	{ return (m_tests_total - m_tests_failed); }

/**
 * Get the number of failed tests.
 */
inline int TestSuite::testsFailed(void) const
	{ return m_tests_failed; }

/**
 * Get the total number of tests.
 */
inline int TestSuite::testsTotal(void) const
	{ return m_tests_total; }

}

#endif /* __LIBGENS_TESTS_TESTSUITE_HPP__ */
