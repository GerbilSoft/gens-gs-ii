/***************************************************************************
 * libcompat/tests: Compatibility Library. (Test Suite)                    *
 * gtest_main.inc.cpp: main() function for test suites.                    *
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

// NOTE: You must have a static int test_main() function that has the
// actual test case code. This file merely initializes gtest for certain
// embedded systems.

#if defined(HW_RVL) || defined(HW_DOL)
// libogc: Wii or GameCube hardware.
#include "gtest_main.ogc.inc.cpp"

#else

/* Standard desktop system. */

int main(int argc, char *argv[])
{
	return test_main(argc, argv);
}

#endif
