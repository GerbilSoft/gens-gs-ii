/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcRar_Unix.cpp: RAR decompressor class. (Unix version)                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "DcRar.hpp"
#include "UnRAR_dll.hpp"

// Win32 Unicode Translation Layer.
#include "../Win32/W32U_mini.h"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// stat()
#include <sys/types.h>
#include <sys/stat.h>

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::stringstream;

namespace LibGens
{

/** Static class variable initialization. **/

/**
 * ms_RarBinary: RAR executable filename.
 * Unix: Filename of "rar" or "unrar".
 * Windows: Filename of "unrar.dll".
 */
#if defined(__amd64__)
string DcRar::ms_RarBinary = "unrar64.dll";
#else
string DcRar::ms_RarBinary = "unrar.dll";
#endif


/**
 * DcRar(): Create a new DcRar object.
 * @param f File pointer.
 * @param filename Filename.
 */
DcRar::DcRar(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	// MiniZip doesn't support opening files by fd.
	if (!f || !filename)
	{
		// No filename specified.
		m_file = NULL;
		m_filename.clear();
		return;
	}
	
	// Load UnRAR.dll.
	// TODO: Set an error flag if it can't be loaded.
	m_unrarDll.load(ms_RarBinary.c_str());
}

/**
 * ~DcRar(): Delete the RAR Decompressor object.
 */
DcRar::~DcRar()
{
	// Unload UnRAR.dll.
	m_unrarDll.unload();
}


/**
 * DetectFormat(): Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool DcRar::DetectFormat(FILE *f)
{
	static const uint8_t rar_magic[] = {'R', 'a', 'r', '!'};
	
	// Seek to the beginning of the file.
	// TODO: fseeko()/fseeko64() support on Linux.
	fseek(f, 0, SEEK_SET);
	
	// Read the "magic number".
	uint8_t header[sizeof(rar_magic)];
	size_t ret = fread(&header, 1, sizeof(header), f);
	if (ret < sizeof(header))
	{
		// Error reading the "magic number".
		return false;
	}
	
	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, rar_magic, sizeof(header)));
}


/**
 * getFileInfo(): Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int DcRar::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file)
		return -2;
	
	// Check that UnRAR.dll is loaded.
	if (!m_unrarDll.isLoaded())
	{
		// Error loading UnRAR.dll.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		return -3; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// TODO: Open the RAR file in the constructor.
	HANDLE hRar;
	char *filenameA = NULL;
	wchar_t *filenameW = NULL;
	
	// Convert the filename from UTF-8 to UTF-16 first.
	filenameW = W32U_mbs_to_UTF16(m_filename.c_str(), CP_UTF8);
	if (!filenameW)
		return -9; // TODO: Figure out an MDP error code for this.
	
	if (!W32U_IsUnicode)
	{
		// System isn't using Unicode.
		// Convert the filename from UTF-16 to ANSI.
		filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		free(filenameW);
		filenameW = NULL;
		
		if (!filenameA)
			return -9; // TODO: Figure out an MDP error code for this.
	}
	
	// Open the RAR file.
	struct RAROpenArchiveDataEx rar_open;
	rar_open.OpenMode = RAR_OM_LIST;
	rar_open.CmtBuf = NULL;
	rar_open.CmtBufSize = 0;
	
	if (W32U_IsUnicode)
	{
		// Unicode mode.
		rar_open.ArcName = NULL;
		rar_open.ArcNameW = filenameW;
	}
	else
	{
		// ANSI mode.
		rar_open.ArcName = filenameA;
		rar_open.ArcNameW = NULL;
	}
	
	hRar = m_unrarDll.pRAROpenArchiveEx(&rar_open);
	if (!hRar)
	{
		// Error opening the RAR file.
		free(filenameA);
		free(filenameW);
		return -4; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// List head and tail.
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_tail = NULL;
	
	// Process the archive.
	struct RARHeaderDataEx rar_header;
	
	// Filenames in a RAR archive have a maximum length of 1,024 characters.
	char utf8_buf[1024*4];
	wchar_t wcs_buf[1024];
	
	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0)
	{
		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		// TODO: What do we do if rar_header.UnpSizeHigh is set (indicating >4 GB)?
		z_entry_cur->filesize = rar_header.UnpSize;
		z_entry_cur->next = NULL;
		
		if (rar_header.FileNameW[0] != 0x00)
		{
			// Use the Unicode filename. (rar_header.FileNameW)
			// Convert UTF-16 to UTF-8.
			WideCharToMultiByte(CP_UTF8, 0, rar_header.FileNameW, -1,
						utf8_buf, sizeof(utf8_buf), NULL, NULL);
			z_entry_cur->filename = strdup(utf8_buf);
		}
		else
		{
			// Use the ANSI filename. (rar_header.FileName)
			// TODO: Proper codepage detection.
			// We're using CP_ACP for archives created on MS-DOS, OS/2, or Win32.
			// We're using UTF-8 for archives created on Unix
			if (rar_header.HostOS < 3)
			{
				// MS-DOS, OS/2, Win32.
				
				// Convert ANSI to UTF-16.
				MultiByteToWideChar(CP_ACP, 0, rar_header.FileName, -1,
							wcs_buf, (sizeof(wcs_buf)/sizeof(wcs_buf[0])));
				// Convert UTF-16 to UTF-8.
				WideCharToMultiByte(CP_UTF8, 0, wcs_buf, -1,
							utf8_buf, sizeof(utf8_buf), NULL, NULL);
				
				// Save the filename.
				z_entry_cur->filename = strdup(utf8_buf);
			}
			else
			{
				// Unix. Assume the ANSI filename is UTF-8.
				z_entry_cur->filename = strdup(rar_header.FileName);
			}
		}
		
		if (!z_entry_head)
		{
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}
		else
		{
			// Append the mdp_z_entry to the end of the list.
			z_entry_tail->next = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}
		
		// Go to the next file.
		if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, NULL, NULL) != 0)
			break;
	}
	
	// Close the RAR file.
	m_unrarDll.pRARCloseArchive(hRar);
	free(filenameA);
	free(filenameW);
	
	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -1; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return 0; // TODO: return MDP_ERR_OK;
}


/**
 * getFile(): Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int DcRar::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file)
		return -2;
	
	// Check that UnRAR.dll is loaded.
	if (!m_unrarDll.isLoaded())
	{
		// Error loading UnRAR.dll.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		return -3; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// TODO: Open the RAR file in the constructor.
	HANDLE hRar;
	char *filenameA = NULL;
	wchar_t *filenameW = NULL;
	
	// Convert the filename from UTF-8 to UTF-16 first.
	filenameW = W32U_mbs_to_UTF16(m_filename.c_str(), CP_UTF8);
	if (!filenameW)
		return -9; // TODO: Figure out an MDP error code for this.
	
	if (!W32U_IsUnicode)
	{
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		free(filenameW);
		filenameW = NULL;
		
		if (!filenameA)
			return -9; // TODO: Figure out an MDP error code for this.
	}
	
	// Open the RAR file.
	struct RAROpenArchiveDataEx rar_open;
	rar_open.OpenMode = RAR_OM_EXTRACT;
	rar_open.CmtBuf = NULL;
	rar_open.CmtBufSize = 0;
	
	if (W32U_IsUnicode)
	{
		// Unicode mode.
		rar_open.ArcName = NULL;
		rar_open.ArcNameW = filenameW;
	}
	else
	{
		// ANSI mode.
		rar_open.ArcName = filenameA;
		rar_open.ArcNameW = NULL;
	}
	
	hRar = m_unrarDll.pRAROpenArchiveEx(&rar_open);
	if (!hRar)
	{
		// Error opening the RAR file.
		free(filenameA);
		free(filenameW);
		return -4; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// Convert z_entry->filename to UTF-16.
	// TODO: Move this to another function.
	char *z_filenameA = NULL;
	wchar_t *z_filenameW = NULL;
	
	// Convert the z_entry filename from UTF-8 to UTF-16 first.
	z_filenameW = W32U_mbs_to_UTF16(z_entry->filename, CP_UTF8);
	if (!z_filenameW)
		return -9; // TODO: Figure out an MDP error code for this.
	
	// Convert the filename from UTF-16 to ANSI in case files don't have a Unicode filename.
	z_filenameA = W32U_UTF16_to_mbs(z_filenameW, CP_ACP);
	if (!z_filenameA)
		return -9; // TODO: Figure out an MDP error code for this.
	
	// Search for the file.
	struct RARHeaderDataEx rar_header;
	*ret_siz = 0;
	int cmp;
	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0)
	{
		if (rar_header.FileNameW[0] != 0x00)
		{
			// Use the Unicode filename.
			cmp = _wcsicmp(z_filenameW, rar_header.FileNameW);
		}
		else
		{
			// Use the ANSI filename. (rar_header.FileName)
			// TODO: Proper codepage detection.
			// We're using CP_ACP for archives created on MS-DOS, OS/2, or Win32.
			// We're using UTF-8 for archives created on Unix
			
			if (rar_header.HostOS < 3)
			{
				// MS-DOS, OS/2, Win32.
				// Use the ANSI codepage.
				cmp = _stricmp(z_filenameA, rar_header.FileName);
			}
			else
			{
				// Unix. Assume the ANSI filename is UTF-8.
				cmp = _stricmp(z_entry->filename, rar_header.FileName);
			}
		}
		
		if (cmp != 0)
		{
			// Not a match. Skip the file.
			if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, NULL, NULL) != 0)
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
		int ret = m_unrarDll.pRARProcessFile(hRar, RAR_TEST, NULL, NULL);
		
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
	free(filenameA);
	free(filenameW);
	free(z_filenameA);
	free(z_filenameW);
	
	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
}


/**
 * RarCallback(): Win32 UnRAR.dll callback function. [STATIC]
 */
int CALLBACK DcRar::RarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
{
	RarState_t *pRarState = (RarState_t*)UserData;
	return pRarState->owner->rarCallback(msg, UserData, P1, P2);
}


/**
 * rarCallback(): Win32 UnRAR.dll callback function.
 */
int CALLBACK DcRar::rarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
{
	switch (msg)
	{
		default:
		case UCM_CHANGEVOLUME:
		case UCM_NEEDPASSWORD:
			// Unhandled message.
			// TODO: Support at least UCM_NEEDPASSWORD.
			return -1;
		
		case UCM_PROCESSDATA:
		{
			// Process data.
			RarState_t *pRarState = (RarState_t*)UserData;
			
			const uint8_t *buf = (const uint8_t*)P1;
			size_t siz = (size_t)P2;
			
			if ((pRarState->pos + siz) > pRarState->siz)
			{
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


/**
 * CheckExtPrg(): Check if the specified external RAR program is usable.
 * @param extprg	[in] External RAR program filename.
 * @param prg_info	[out] If not NULL, contains RAR/UnRAR version information.
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
uint32_t DcRar::CheckExtPrg(const utf8_str *extprg, ExtPrgInfo *prg_info)
{
	// Program information.
	ExtPrgInfo my_prg_info;
	memset(&my_prg_info, 0x00, sizeof(my_prg_info));
	if (prg_info)
		memset(prg_info, 0x00, sizeof(*prg_info));
	
	// Check that the UnRAR DLL is available.
	// TODO: W32U version of access()?
	if (access(extprg, F_OK) != 0)
		return -1;
	
	// Make sure that this is a regular file.
	struct stat st_buf;
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
	dwLen = GetFileVersionInfoSize(extprg, NULL);
	if (dwLen == 0)
		return -6;
	lpData = malloc(dwLen);
	if (!GetFileVersionInfo(extprg, 0, dwLen, lpData))
	{
		free(lpData);
		return -6;
	}
	if (!VerQueryValue(lpData, "\\", &lpVerInfo, &lenVerInfo))
	{
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
	if (!dll.isLoaded())
	{
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
	if (my_prg_info.api_version < RAR_DLL_VERSION)
	{
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

}
