/***************************************************************************
 * libzomg/tests: Zipped Original Memory from Genesis. (Test Suite)        *
 * PrintMetadata.cpp: Print ZOMG.ini metadata.                             *
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

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <climits>

// C++ includes.
#include <string>
using std::string;

#include "libgens/lg_main.hpp"
#include "Metadata.hpp"
using LibZomg::Metadata;

/**
 * Test suite main function.
 * Called by gtest_main.inc.cpp's main().
 */
static int test_main(int argc, char *argv[])
{
	// We aren't using any command line parameters here.
	((void)argc);
	((void)argv);

	fprintf(stderr, "LibZomg test suite: Print Metadata.\n"
		"Testing system metadata only; no ROM information is loaded.\n\n");
	LibGens::Init();

	// Create a Metadata object with no ROM information.
	Metadata *metadata = new Metadata();
	string zomgIni;

	printf("Metadata with metaFlags = MF_Default:\n");
	zomgIni = metadata->toZomgIni(Metadata::MF_Default);
	fputs(zomgIni.c_str(), stdout);

	putchar('\n');
	printf("Metadata with metaFlags = INT_MAX (all):\n");
	zomgIni = metadata->toZomgIni(INT_MAX);
	fputs(zomgIni.c_str(), stdout);

	return 0;
}

#include "libcompat/tests/gtest_main.inc.cpp"
