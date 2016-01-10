/***************************************************************************
 * libcompat/W32U/tests: Win32 Unicode Translation Layer. (Test Suite)     *
 * MiniZip_iowin32u_Test.cpp: MiniZip iowin32u tests.                      *
 *                                                                         *
 * Copyright (c) 2011-2016 by David Korth.                                 *
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

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// MiniZip.
#include "minizip/unzip.h"
#include "minizip/iowin32.h"
#include "../minizip_iowin32u.h"

namespace LibCompat { namespace W32U { namespace Tests {

struct iowin32u_Test_Flags {
	// Filename of the Zip file.
	const char *src_filename_ANSI;
	const wchar_t *src_filename_WCHAR;
	const char *src_filename_UTF8;

	// Filename inside the file.
	// NOTE: Only stored as UTF-8.
	const char *in_filename_UTF8;

	iowin32u_Test_Flags(const char *sa, const wchar_t *sw, const char *su,
			    const char *iu)
	{
		src_filename_ANSI = sa;
		src_filename_WCHAR = sw;
		src_filename_UTF8 = su;
		in_filename_UTF8 = iu;
	}
};

class MiniZip_iowin32u_Test : public ::testing::TestWithParam<iowin32u_Test_Flags>
{
	protected:
		MiniZip_iowin32u_Test()
			: ::testing::TestWithParam<iowin32u_Test_Flags>()
			, m_unzFile(nullptr) { }
		virtual ~MiniZip_iowin32u_Test() { }

		virtual void SetUp(void);
		virtual void TearDown(void);

	protected:
		unzFile m_unzFile;
};

/**
 * Set up the test.
 */
void MiniZip_iowin32u_Test::SetUp(void)
{ }

/**
 * Tear down the test.
 */
void MiniZip_iowin32u_Test::TearDown(void)
{
	if (m_unzFile) {
		unzClose(m_unzFile);
		m_unzFile = nullptr;
	}
}

/** Test cases. **/

/**
 * Test MiniZip using iowin32's ANSI functions.
 */
TEST_P(MiniZip_iowin32u_Test, iowin32_ANSI)
{
	iowin32u_Test_Flags flags = GetParam();

	// Open the ANSI filename.
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A(&ffunc);
	m_unzFile = unzOpen2_64(flags.src_filename_ANSI, &ffunc);
	ASSERT_TRUE(m_unzFile != nullptr);

	// Get the first header within the file.
	ASSERT_EQ(UNZ_OK, unzGoToFirstFile(m_unzFile));
	
	char in_filename[4096];
	unz_file_info zinfo;
	unzGetCurrentFileInfo(m_unzFile, &zinfo,
		in_filename, sizeof(in_filename),
		nullptr, 0, nullptr, 0);
	in_filename[sizeof(in_filename)-1] = 0;	// TODO: Not needed?
	EXPECT_STREQ(flags.in_filename_UTF8, in_filename);
}

/**
 * Test MiniZip using iowin32's Unicode functions.
 */
TEST_P(MiniZip_iowin32u_Test, iowin32_Unicode)
{
	iowin32u_Test_Flags flags = GetParam();

	// Open the Unicode filename.
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64W(&ffunc);
	m_unzFile = unzOpen2_64(flags.src_filename_WCHAR, &ffunc);
	ASSERT_TRUE(m_unzFile != nullptr);

	// Get the first header within the file.
	ASSERT_EQ(UNZ_OK, unzGoToFirstFile(m_unzFile));
	
	char in_filename[4096];
	unz_file_info zinfo;
	unzGetCurrentFileInfo(m_unzFile, &zinfo,
		in_filename, sizeof(in_filename),
		nullptr, 0, nullptr, 0);
	in_filename[sizeof(in_filename)-1] = 0;	// TODO: Not needed?
	EXPECT_STREQ(flags.in_filename_UTF8, in_filename);
}

/**
 * Test MiniZip using iowin32u's UTF-8 functions.
 */
TEST_P(MiniZip_iowin32u_Test, iowin32u_UTF8)
{
	iowin32u_Test_Flags flags = GetParam();

	// Open the UTF-8 filename.
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_unzFile = unzOpen2_64(flags.src_filename_UTF8, &ffunc);
	ASSERT_TRUE(m_unzFile != nullptr);

	// Get the first header within the file.
	ASSERT_EQ(UNZ_OK, unzGoToFirstFile(m_unzFile));
	
	char in_filename[4096];
	unz_file_info zinfo;
	unzGetCurrentFileInfo(m_unzFile, &zinfo,
		in_filename, sizeof(in_filename),
		nullptr, 0, nullptr, 0);
	in_filename[sizeof(in_filename)-1] = 0;	// TODO: Not needed?
	EXPECT_STREQ(flags.in_filename_UTF8, in_filename);
}

static const char    ansi_inside_txt_UTF8[]    = "ansi_inside.txt";
static const char    unicode_inside_txt_UTF8[] = "unicode_inside_\xE2\x98\x8E.txt";

static const char    ansi_in_ansi_out_zip_ANSI[]  =  "ansi_in_ansi_out.zip";
static const wchar_t ansi_in_ansi_out_zip_WCHAR[] = L"ansi_in_ansi_out.zip";
static const char    ansi_in_ansi_out_zip_UTF8[]  =  "ansi_in_ansi_out.zip";

static const char    ansi_in_unicode_out_zip_ANSI[]  =  "ansi_in_unicode_out_?.zip";
static const wchar_t ansi_in_unicode_out_zip_WCHAR[] = {'a', 'n', 's', 'i', '_', 'i', 'n', '_', 'u', 'n', 'i', 'c', 'o', 'd', 'e', '_', 'o', 'u', 't', '_', 0x267B, '.', 'z', 'i', 'p', 0};
static const char    ansi_in_unicode_out_zip_UTF8[]  =  "ansi_in_unicode_out_\xE2\x99\xBB.zip";

static const char    unicode_in_ansi_out_zip_ANSI[]  =  "unicode_in_ansi_out.zip";
static const wchar_t unicode_in_ansi_out_zip_WCHAR[] = L"unicode_in_ansi_out.zip";
static const char    unicode_in_ansi_out_zip_UTF8[]  =  "unicode_in_ansi_out.zip";

static const char    unicode_in_unicode_out_zip_ANSI[]  =  "unicode_in_unicode_out_?.zip";
static const wchar_t unicode_in_unicode_out_zip_WCHAR[] = {'u', 'n', 'i', 'c', 'o', 'd', 'e', '_', 'i', 'n', '_', 'u', 'n', 'i', 'c', 'o', 'd', 'e', '_', 'o', 'u', 't', '_', 0x267B, '.', 'z', 'i', 'p', 0};
static const char    unicode_in_unicode_out_zip_UTF8[]  =  "unicode_in_unicode_out_\xE2\x99\xBB.zip";

INSTANTIATE_TEST_CASE_P(ansi_out_tests, MiniZip_iowin32u_Test,
	::testing::Values(
		iowin32u_Test_Flags(ansi_in_ansi_out_zip_ANSI, ansi_in_ansi_out_zip_WCHAR, ansi_in_ansi_out_zip_UTF8,
				    ansi_inside_txt_UTF8),
		iowin32u_Test_Flags(unicode_in_ansi_out_zip_ANSI, unicode_in_ansi_out_zip_WCHAR, unicode_in_ansi_out_zip_UTF8,
				    unicode_inside_txt_UTF8),
		iowin32u_Test_Flags(ansi_in_ansi_out_zip_ANSI, ansi_in_ansi_out_zip_WCHAR, ansi_in_ansi_out_zip_UTF8,
				    ansi_inside_txt_UTF8),
		iowin32u_Test_Flags(unicode_in_ansi_out_zip_ANSI, unicode_in_ansi_out_zip_WCHAR, unicode_in_ansi_out_zip_UTF8,
				    unicode_inside_txt_UTF8))
);

INSTANTIATE_TEST_CASE_P(unicode_out_tests, MiniZip_iowin32u_Test,
	::testing::Values(
		iowin32u_Test_Flags(ansi_in_unicode_out_zip_ANSI, ansi_in_unicode_out_zip_WCHAR, ansi_in_unicode_out_zip_UTF8,
				    ansi_inside_txt_UTF8),
		iowin32u_Test_Flags(unicode_in_unicode_out_zip_ANSI, unicode_in_unicode_out_zip_WCHAR, unicode_in_unicode_out_zip_UTF8,
				    unicode_inside_txt_UTF8),
		iowin32u_Test_Flags(ansi_in_unicode_out_zip_ANSI, ansi_in_unicode_out_zip_WCHAR, ansi_in_unicode_out_zip_UTF8,
				    ansi_inside_txt_UTF8),
		iowin32u_Test_Flags(unicode_in_unicode_out_zip_ANSI, unicode_in_unicode_out_zip_WCHAR, unicode_in_unicode_out_zip_UTF8,
				    unicode_inside_txt_UTF8))
);

} } }

/**
 * Test suite main function.
 * Called by gtest_main.inc.cpp's main().
 */
static int test_main(int argc, char *argv[])
{
	fprintf(stderr, "LibCompat/W32U test suite: MiniZip iowin32u tests.\n\n");
	// Run the test.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#include "../../tests/gtest_main.inc.cpp"
