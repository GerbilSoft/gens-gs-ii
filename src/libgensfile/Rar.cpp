/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Rar.hpp: RAR archive handler.                                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2016 by David Korth.                                 *
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

#include "Rar.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>
// C++ includes.
#include <memory>
using std::auto_ptr;

// stat()
#include <sys/types.h>
#include <sys/stat.h>

// MSVC lacks some stat macros.
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & _S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & _S_IFMT) == S_IFREG)
#endif

// Character set conversion.
#include "libgenstext/Encoding.hpp"

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif /* _WIN32 */

// C++ includes.
#include <string>
using std::string;
using std::wstring;

// FIXME: MinGW doesn't have wcscasecmp() or wcscmp() wrappers.
// These macros are defined in c++11-compat.msvc.h,
// but that's only included in MSVC builds.
#if defined(_WIN32) && defined(__GNUC__)
#define wcscasecmp(s1, s2)     _wcsicmp(s1, s2)
#define wcsncasecmp(s1, s2, n) _wcsnicmp(s1, s2, n)
#endif

// Filename comparison functions.
// Windows, Mac: Case-insensitive.
// All others: Case-sensitive.
#if defined(_WIN32) || defined(__APPLE__)
#define WCS_FILENAME_COMPARE(file1, file2) wcscasecmp((file1), (file2))
#define STR_FILENAME_COMPARE(file1, file2) strcasecmp((file1), (file2))
#else
#define WCS_FILENAME_COMPARE(file1, file2) wcscmp((file1), (file2))
#define STR_FILENAME_COMPARE(file1, file2) strcmp((file1), (file2))
#endif

namespace LibGensFile {

/** Static class variable initialization. **/

// UnRAR.dll filename.
const char Rar::m_unrarDll_filename[] =
#ifdef _WIN32
#if defined(__amd64__) || defined(__x86_64__)
	"unrar64.dll";
#else
	"unrar.dll";
#endif
#else /* !_WIN32 */
	"libgensunrar.so";
#endif

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Rar::Rar(const char *filename)
	: Archive(filename)
{
	if (!m_file)
		return;

#ifdef _WIN32
	// Convert the filename from UTF-8 to wchar_t.
	// This is needed on Windows, since UnRAR.dll on Windows
	// only supports ANSI and UTF-16.
	m_filenameW = LibGensText::Utf8_to_Wchar(m_filename);
	if (m_filenameW.empty()) {
		// Error converting the filename.
		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EINVAL;	// TODO: MDP error code.
		return;
	}

	if (!W32U_IsUnicode()) {
		// System isn't using Unicode.
		// Convert the filename from UTF-16 to ANSI.
		char *filenameA = W32U_UTF16_to_mbs(m_filenameW.c_str(), CP_ACP);
		if (!filenameA) {
			// Error converting the filename.
			fclose(m_file);
			m_file = nullptr;
			// TODO: Error code?
			m_lastError = EINVAL;
			return; // TODO: MDP error code.
		}

		m_filenameA = string(filenameA);
		free(filenameA);
	}
#endif /* _WIN32 */

	// Load UnRAR.dll.
	// TODO: Set an error flag if it can't be loaded.
	m_unrarDll.load(m_unrarDll_filename);
	if (!m_unrarDll.isLoaded()) {
		// Error loading UnRAR.dll.
		// TODO: Find the actual DLL error.
		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Check if UnRAR.dll can open the file.
	HANDLE hRar = openRar(RAR_OM_LIST);
	if (!hRar) {
		// Error opening the file.
		// openRar() already set m_lastError.
		m_unrarDll.unload();
		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Close the RAR archive.
	m_unrarDll.pRARCloseArchive(hRar);

	// File is a RAR archive.
}

/**
 * Delete the Rar object.
 */
Rar::~Rar()
{
	// Unload UnRAR.dll.
	m_unrarDll.unload();
}

/**
 * Close the archive file.
 */
void Rar::close(void)
{
	// Unload UnRAR.dll.
	m_unrarDll.unload();

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Open the Archive's file using UnRAR.dll.
 * @param mode RAR open mode.
 * @return RAR handle, or nullptr on error. (TODO: Error code?)
 */
HANDLE Rar::openRar(int mode)
{
	if (!m_file || m_filename.empty())
		return nullptr;

	// Check for RAR magic first.
	// If it's not there, this is either a really old
	// RAR 1.x archive, or it isn't a RAR archive at all.
	// TODO: How do we check for RAR 1.x?
	static const uint8_t rar_magic[] = {'R', 'a', 'r', '!'};
	int ret = checkMagic(rar_magic, sizeof(rar_magic));
	if (ret != 0) {
		// Not a RAR archive.
		// (Or, it's a really old RAR archive.)
		m_lastError = -ret;
		return nullptr;
	}

	// RAR handle.
	HANDLE hRar;

	// Attempt to open the RAR archive using UnRAR.dll.
	if (m_unrarDll.pRAROpenArchive) {
		// UnRAR.dll v3 or later.
		struct RAROpenArchiveDataEx rar_open;
		memset(&rar_open, 0, sizeof(rar_open));
		rar_open.OpenMode = mode;

#ifdef _WIN32
		if (!W32U_IsUnicode()) {
			// System is not Unicode.
			// Use the ANSI filename.
			rar_open.ArcName = (char*)m_filenameA.c_str();
		} else {
			// System is Unicode.
			// Use the Unicode filename.
			rar_open.ArcNameW = (wchar_t*)m_filenameW.c_str();
		}
#else /* !_WIN32 */
		// Linux/Unix: Use the native 8-bit encoding.
		// The system locale must be set to UTF-8 for
		// Unicode to be handled correctly.
		rar_open.ArcName = (char*)m_filename.c_str();
#endif /* _WIN32 */

		// Open the RAR file.
		hRar = m_unrarDll.pRAROpenArchiveEx(&rar_open);
	} else {
		// UnRAR.dll v2.
		// RAROpenArchive doesn't support Unicode filenames,
		// so we have to use ANSI.
		struct RAROpenArchiveData rar_open;
		memset(&rar_open, 0, sizeof(rar_open));
		rar_open.OpenMode = mode;

#ifdef _WIN32
		// Win32: ANSI filename.
		rar_open.ArcName = (char*)m_filenameA.c_str();
#else /* !_WIN32 */
		// Linux: UTF-8 filename.
		// TODO: Is v2 even available for Linux, and
		// does it support UTF-8 correctly?
		rar_open.ArcName = (char*)m_filename.c_str();
#endif /* _WIN32 */

		// Open the RAR file.
		hRar = m_unrarDll.pRAROpenArchive(&rar_open);
	}

	if (!hRar) {
		// Error opening the RAR file.
		// TODO: Correct error code?
		m_lastError = EIO;
	}

	return hRar;
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Rar::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_unrarDll.isLoaded()) {
		// UnRAR.dll is not loaded.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		m_lastError = ENOSYS;	// TODO: Better error?
		return -m_lastError; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// Open the RAR file.
	HANDLE hRar = openRar(RAR_OM_LIST);
	if (!hRar) {
		// Error opening the RAR file.
		// TODO: Correct error code?
		m_lastError = EIO;
		return -m_lastError; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// Process the archive.
	struct RARHeaderDataEx rar_header;

	// Filenames in a RAR archive have a maximum length of 1,024 characters.
	char utf8_buf[1024*4];
	wchar_t wcs_buf[1024];

	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0) {
		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

		// Store the ROM file information.
		// TODO: What do we do if rar_header.UnpSizeHigh is set (indicating >4 GB)?
		z_entry_cur->filesize = rar_header.UnpSize;
		z_entry_cur->next = nullptr;

		// UnRAR.dll always presents valid "ANSI" and Unicode filenames.
#ifdef _WIN32
		// Windows: rar_header.FileNameA is ANSI.
		// Convert the Unicode filename to UTF-8.
		z_entry_cur->filename = W32U_UTF16_to_mbs(rar_header.FileNameW, CP_UTF8);
#else /* !_WIN32 */
		// Other systems: Use the "ANSI" filename directly.
		// The "ANSI" filename is converted from the original
		// Unicode using mbsrtowcs(), so if the system locale
		// is set to UTF-8, the filename will be UTF-8.
		z_entry_cur->filename = strdup(rar_header.FileName);
#endif

		if (!z_entry_head) {
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_tail = z_entry_cur;
		} else {
			// Append the mdp_z_entry to the end of the list.
			z_entry_tail->next = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}

		// Go to the next file.
		if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, nullptr, nullptr) != 0)
			break;
	}

	// Close the RAR file.
	m_unrarDll.pRARCloseArchive(hRar);

	// If there are no files in the archive, return an error.
	if (!z_entry_head) {
		// TODO: Better error?
		m_lastError = ENOENT;
		return -m_lastError; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}

	// Return the list of files.
	*z_entry_out = z_entry_head;
	return 0; // TODO: return MDP_ERR_OK;
}

/**
 * Read all or part of a file from the archive.
 * NOTE: This function is NOT optimized for random seeking.
 * The skip functionality is intended for skipping over headers,
 * e.g. for SMD-format ROMs.
 *
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
 * @param start_pos	[in]  Starting position within the file.
 * @param read_len	[in]  Number of bytes to read.
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf. (Must be >= read_len.)
 * @param ret_siz	[out] Pointer to file_offset_t to store the number of bytes read.
 * @return 0 on success; negative POSIX error code on error.
 */
int Rar::readFile(const mdp_z_entry_t *z_entry,
		  file_offset_t start_pos, file_offset_t read_len,
		  void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
	if (!z_entry || !buf ||
	    start_pos < 0 || start_pos >= z_entry->filesize ||
	    read_len < 0 || z_entry->filesize - read_len < start_pos ||
	    siz <= 0 || siz < read_len)
	{
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_unrarDll.isLoaded()) {
		// UnRAR.dll is not loaded.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		m_lastError = ENOSYS;	// TODO: Better error?
		return -m_lastError; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// Make sure the z_entry filename is valid.
	if (!z_entry->filename) {
		// Invalid filename.
		m_lastError = EINVAL;
		return -m_lastError; // TODO: Return an appropriate MDP error code.
	}

	// Convert z_entry->filename to wchar_t.
	wstring z_filenameW = LibGensText::Utf8_to_Wchar(
		z_entry->filename, strlen(z_entry->filename));
	if (z_filenameW.empty()) {
		// Error converting the filename.
		m_lastError = EINVAL;
		return m_lastError; // TODO: Return an appropriate MDP error code.
	}

#ifdef _WIN32
	// Convert the filename from UTF-16 to ANSI in case files don't have a Unicode filename.
	// TODO: Implement for Linux?
	auto_ptr<char> z_filenameA(W32U_UTF16_to_mbs(z_filenameW.c_str(), CP_ACP));
	if (!z_filenameA.get()) {
		// Error converting the filename.
		m_lastError = EINVAL;
		return -m_lastError; // TODO: Return an appropriate MDP error code.
	}
#endif /* _WIN32 */

	// Open the RAR file.
	HANDLE hRar = openRar(RAR_OM_EXTRACT);
	if (!hRar) {
		// Error opening the RAR file.
		// TODO: Correct error code?
		m_lastError = EIO;
		return -m_lastError; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}

	// Search for the file.
	struct RARHeaderDataEx rar_header;
	*ret_siz = 0;
	int cmp;
	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0) {
		if (rar_header.FileNameW[0] != 0x00) {
			// Use the Unicode filename.
			cmp = WCS_FILENAME_COMPARE(z_filenameW.c_str(), rar_header.FileNameW);
		} else {
			// Use the ANSI filename. (rar_header.FileName)
			// TODO: Proper codepage detection.
			// We're using CP_ACP for archives created on MS-DOS, OS/2, or Win32.
			// We're using UTF-8 for archives created on Unix

			// TODO: ANSI support on Linux/Unix?
#ifdef _WIN32
			if (rar_header.HostOS < 3) {
				// MS-DOS, OS/2, Win32.
				// Use the ANSI codepage.
				cmp = STR_FILENAME_COMPARE(z_filenameA.get(), rar_header.FileName);
			} else
#endif /* _WIN32 */
			{
				// Unix. Assume the ANSI filename is UTF-8.
				cmp = STR_FILENAME_COMPARE(z_entry->filename, rar_header.FileName);
			}
		}

		if (cmp != 0) {
			// Not a match. Skip the file.
			if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, nullptr, nullptr) != 0)
				break;
			continue;
		}

		// Found the file.

		// Create the RAR state.
		RarState_t rar_state;
		rar_state.buf = (uint8_t*)buf;
		rar_state.siz = siz;
		rar_state.pos = 0;
		rar_state.owner = this;

		// Set up the RAR callback.
		m_unrarDll.pRARSetCallback(hRar, &RarCallback, (LPARAM)&rar_state);

		// Process the file.
		// Possible errors:
		// - 0: Success.
		// - ERAR_UNKNOWN: Read the maximum amount of data for the ubuffer.
		// - Others: Read error; abort. (TODO: Show an error message.)
		int ret = m_unrarDll.pRARProcessFile(hRar, RAR_TEST, nullptr, nullptr);

		// TODO: DcRar.cpp returns the filesize processed on error.
		// This just returns 0.
		if (ret != 0 && ret != ERAR_UNKNOWN)
			break;

		// File processed.
		*ret_siz = rar_state.pos;
		break;
	}

	// Close the RAR file.
	m_unrarDll.pRARCloseArchive(hRar);

	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
}

/**
 * Win32 UnRAR.dll callback function. [STATIC]
 */
int CALLBACK Rar::RarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
{
	RarState_t *pRarState = (RarState_t*)UserData;

	switch (msg) {
		default:
		case UCM_CHANGEVOLUME:
		case UCM_NEEDPASSWORD:
			// Unhandled message.
			// TODO: Support at least UCM_NEEDPASSWORD.
			return -1;

		case UCM_CHANGEVOLUMEW:
		case UCM_NEEDPASSWORDW:
			// Unicode versions of the above unhandled messages.
			// TODO: Support at least UCM_NEEDPASSWORDW.
			return 1;

		case UCM_PROCESSDATA: {
			// Process data.
			const uint8_t *buf = (const uint8_t*)P1;
			size_t siz = (size_t)P2;

			// FIXME: Use subtraction to check for overflow.
			if ((pRarState->pos + siz) > pRarState->siz) {
				// Overflow!
				size_t siz_diff = (pRarState->siz - pRarState->pos);
				memcpy(&pRarState->buf[pRarState->pos], buf, siz_diff);
				pRarState->pos += siz_diff;
				return -1;
			}

			// Copy the data.
			memcpy(&pRarState->buf[pRarState->pos], buf, siz);
			pRarState->pos += siz;
			break;
		}
	}

	return 0;
}

#if 0
// TODO: Port to Archive.

/**
 * Check if the specified external RAR program is usable.
 * @param extprg	[in] External RAR program filename.
 * @param prg_info	[out] If not nullptr, contains RAR/UnRAR version information.
 * @return Possible error codes:
 * -  0: Program is usable.
 * - -1: File not found.
 * - -2: File isn't executable
 * - -3: File isn't a regular file. (e.g. it's a directory)
 * - -4: Error calling stat().
 * - -5: Wrong DLL API version. (Win32 only)
 * - -6: Version information not found.
 * - -7: Not RAR, UnRAR, or UnRAR.dll.
 * TODO: Use MDP error code constants.
 */
uint32_t DcRar::CheckExtPrg(const char *extprg, ExtPrgInfo *prg_info)
{
	// Program information.
	ExtPrgInfo my_prg_info;
	memset(&my_prg_info, 0x00, sizeof(my_prg_info));
	if (prg_info)
		memset(prg_info, 0x00, sizeof(*prg_info));

	// Check that the UnRAR DLL is available.
	if (access(extprg, F_OK) != 0)
		return -1;

	// Make sure that this is a regular file.
	struct _stati64 st_buf;
	if (stat(extprg, &st_buf) != 0)
		return -4;
	if (!S_ISREG(st_buf.st_mode))
		return -3;

	// Attempt to load the DLL manually.
	// TODO: Use W32U?
	HINSTANCE hDll = LoadLibrary(extprg);
	if (!hDll)
		return -2;
	FreeLibrary(hDll);

	// Get the UnRAR DLL version information.
	DWORD dwLen;
	void *lpData, *lpVerInfo;
	unsigned int lenVerInfo;

	// TODO: Do we need Unicode translation for VerQueryValue?
	// Alternatively, just use the "A" functions.
	dwLen = GetFileVersionInfoSize(extprg, nullptr);
	if (dwLen == 0)
		return -6;
	lpData = malloc(dwLen);
	if (!GetFileVersionInfo(extprg, 0, dwLen, lpData)) {
		free(lpData);
		return -6;
	}
	if (!VerQueryValue(lpData, "\\", &lpVerInfo, &lenVerInfo)) {
		free(lpData);
		return -6;
	}

	// Get the file version.
	VS_FIXEDFILEINFO *lpFixedFileInfo = (VS_FIXEDFILEINFO*)lpVerInfo;
	my_prg_info.dll_major    = (lpFixedFileInfo->dwFileVersionMS >> 16);
	my_prg_info.dll_minor    = (lpFixedFileInfo->dwFileVersionMS & 0xFFFF);
	my_prg_info.dll_revision = (lpFixedFileInfo->dwFileVersionLS >> 16);
	my_prg_info.dll_build    = (lpFixedFileInfo->dwFileVersionLS & 0xFFFF);
	free(lpData);

	// Attempt to load the DLL.
	UnRAR_dll dll;
	dll.load(extprg);
	if (!dll.isLoaded()) {
		// Error loading the DLL.
		// TODO: Determine what the error is.
		// Assume that it's not UnRAR.dll for now.
		// (It could also be that the DLL is too old and is missing a function.)
		if (prg_info)
			memcpy(prg_info, &my_prg_info, sizeof(*prg_info));
		return -7;
	}

	// Get the UnRAR DLL API version.
	my_prg_info.api_version = dll.pRARGetDllVersion();
	if (my_prg_info.api_version < RAR_DLL_VERSION) {
		// DLL is too old.
		if (prg_info)
			memcpy(prg_info, &my_prg_info, sizeof(*prg_info));
		return -5;
	}

	// This is UnRAR.dll.
	my_prg_info.rar_type = ExtPrgInfo::RAR_ET_UNRAR_DLL;

	// RAR version obtained.
	if (prg_info)
		memcpy(prg_info, &my_prg_info, sizeof(*prg_info));

	// RAR is usable.
	return 0;
}
#endif

}
