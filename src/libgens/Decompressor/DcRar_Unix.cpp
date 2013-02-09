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

// C includes.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
 * RAR executable filename.
 * Unix: Filename of "rar" or "unrar".
 * Windows: Filename of "unrar.dll".
 */
string DcRar::ms_RarBinary = "/usr/bin/unrar";


/**
 * Create a new DcRar object.
 * @param f File pointer.
 * @param filename Filename.
 */
DcRar::DcRar(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	// MiniZip doesn't support opening files by fd.
	if (!f || !filename) {
		// No filename specified.
		m_file = nullptr;
		m_filename.clear();
		return;
	}

	// TODO: Check that the RAR binary filename is valid.
}

/**
 * ~DcRar(): Delete the RAR Decompressor object.
 */
DcRar::~DcRar()
{
	// TODO
}


/**
 * Detect if the file can be handled by this decompressor.
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
	if (ret < sizeof(header)) {
		// Error reading the "magic number".
		return false;
	}

	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, rar_magic, sizeof(header)));
}


/**
 * Get information about all files in the archive.
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

	// Check that the RAR executable is available.
	if (access(ms_RarBinary.c_str(), X_OK) != 0) {
		// Cannot run the RAR executable.
		return -3; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// Build the command line.
	string sCmdLine = "\"" + ms_RarBinary + "\" v \"" + m_filename + "\"";

	// Open the RAR file.
	FILE *pRar = popen(sCmdLine.c_str(), "r");
	if (!pRar) {
		// Error running the RAR executable.
		return -4; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// Read from the pipe.
	char buf[4096+1];
	size_t rv;
	stringstream ss;
	while ((rv = fread(buf, 1, sizeof(buf)-1, pRar))) {
		buf[rv] = 0x00;
		ss << buf;
	}

	// Close the RAR file.
	pclose(pRar);

	// Get the string and go through it to get the file listing.
	const string &data = ss.str();

	// Find the "---", which indicates the start of the file listing.
	size_t listStart = data.find("---");
	if (listStart == string::npos) {
		// Not found. Either there are no files, or the archive is broken.
		return -5; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}

	// Find the newline after the list start.
	size_t listStartLF = data.find('\n', listStart);
	if (listStart == string::npos) {
		// Not found. Either there are no files, or the archive is broken.
		return -5; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// Parse all lines until we hit another "---" (or EOF).
	size_t curStartPos = listStartLF + 1; // (+ 1) to skip the newline.
	size_t curEndPos;
	string curLine;
	bool endOfRar = false;

	// Temporary data.
	string tmp_filename;
	size_t tmp_filesize;

	while (!endOfRar) {
		curEndPos = data.find('\n', curStartPos);
		if (curEndPos == string::npos) {
			// End of file listing.
			break;
		}

		// Get the current line.
		curLine = data.substr(curStartPos, (curEndPos - curStartPos));

		// First line in a RAR file listing is the filename.
		// (starting at the second character)
		if (curLine.size() < 2)
			break;
		if (curLine.at(0) == '-') {
			// End of file listing.
			break;
		}

		tmp_filename = curLine.substr(1);

		// Get the second line, which contains the filesize and filetype.
		curStartPos = curEndPos + 1; // (+ 1) to skip the newline.
		curEndPos = data.find('\n', curStartPos);
		if (curEndPos == string::npos) {
			// End of file listing.
			break;
		}

		// Get the current line.
		curLine = data.substr(curStartPos, (curEndPos - curStartPos));

		// Check if this is a normal file.
		if (curLine.size() < 62)
			break;

		// Normal file.
		tmp_filesize = atoi(curLine.substr(12, 10).c_str());

		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

		// Store the ROM file information.
		z_entry_cur->filename = strdup(tmp_filename.c_str());
		z_entry_cur->filesize = tmp_filesize;
		z_entry_cur->next = nullptr;

		if (!z_entry_head) {
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_tail = z_entry_cur;
		} else {
			// Append the mdp_z_entry to the end of the list.
			z_entry_tail->next = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}

		// Go to the next file in the listing.
		curStartPos = curEndPos + 1; // (+ 1) to skip the newline.
	}

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
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the z_entry filename is valid.
	if (!z_entry->filename)
	{
		// Invalid filename.
		return -1; // TODO: Return an appropriate MDP error code.
	}
	
	// Check that the RAR executable is available.
	if (access(ms_RarBinary.c_str(), X_OK) != 0)
	{
		// Cannot run the RAR executable.
		return -3; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// Build the command line.
	string sCmdLine = "\"" + ms_RarBinary + "\" p -ierr \"" + m_filename +
			"\" \"" + string(z_entry->filename) + "\" 2>/dev/null";
	
	// Open the RAR file.
	FILE *pRar = popen(sCmdLine.c_str(), "r");
	if (!pRar)
	{
		// Error running the RAR executable.
		return -4; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// Read from the pipe.
	size_t extracted_size = 0;
	size_t rv;
	uint8_t bufRar[4096];
	uint8_t *buf8 = (uint8_t*)buf;
	
	while ((rv = fread(bufRar, 1, sizeof(bufRar), pRar)))
	{
		if (extracted_size + rv > siz)
		{
			// Do a partial copy.
			rv = (siz - extracted_size);
			if (rv > 0)
			{
				memcpy(&buf8[extracted_size], &bufRar, rv);
				extracted_size += rv;
			}
			break;
		}
		
		// Do a full copy.
		memcpy(&buf8[extracted_size], &bufRar, rv);
		extracted_size += rv;
	}
	
	// Close the RAR file.
	pclose(pRar);
	
	// File extracted successfully.
	*ret_siz = extracted_size;
	return 0; // TODO: return MDP_ERR_OK;
}


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
uint32_t DcRar::CheckExtPrg(const utf8_str *extprg, ExtPrgInfo *prg_info)
{
	// Program information.
	// Clear out fields not used by the Unix version.
	ExtPrgInfo my_prg_info;
	memset(&my_prg_info, 0x00, sizeof(my_prg_info));
	if (prg_info)
		memset(prg_info, 0x00, sizeof(*prg_info));

	// Check that the RAR executable is available.
	if (access(extprg, F_OK) != 0)
		return -1;
	if (access(extprg, X_OK) != 0)
		return -2;

	// Make sure that this is a regular file.
	struct stat st_buf;
	if (stat(extprg, &st_buf) != 0)
		return -4;
	if (!S_ISREG(st_buf.st_mode))
		return -3;

	// Build the command line.
	string sCmdLine = "\"" + string(extprg) + "\"";

	// Open the RAR file.
	FILE *pRar = popen(sCmdLine.c_str(), "r");
	if (!pRar)
		return -2;

	// Read 1 KB maximum from the pipe.
	char buf[1024+1];
	size_t rv = fread(buf, 1, sizeof(buf)-1, pRar);
	buf[rv] = 0x00;
	pclose(pRar);

	// Pipe contents should start with one of the following:
	// RAR: "\nRAR x.xx"
	// UnRAR: "\nUNRAR x.xx"
	// TODO: Use strtok() on platforms that don't have strtok_r().
	char *token, *saveptr;
	char *strtol_endptr;

	token = strtok_r(buf, "\n ", &saveptr);
	if (!token)
		return -7;

	if (!strncasecmp(token, "UNRAR", 6))
		my_prg_info.rar_type = ExtPrgInfo::RAR_ET_UNRAR;
	else if (!strncasecmp(token, "RAR", 4))
		my_prg_info.rar_type = ExtPrgInfo::RAR_ET_RAR;
	else
		return -7;

	// Get the RAR major version.
	token = strtok_r(nullptr, ".", &saveptr);
	if (!token)
		return -6;
	my_prg_info.dll_major = strtol(token, &strtol_endptr, 10);
	if (!strtol_endptr || *strtol_endptr != 0x00)
		return -6;

	// Get the RAR minor version.
	token = strtok_r(nullptr, " ", &saveptr);
	if (!token)
		return -6;
	my_prg_info.dll_minor = strtol(token, &strtol_endptr, 10);
	if (!strtol_endptr || *strtol_endptr != 0x00)
		return -6;

	// RAR version obtained.
	if (prg_info)
		memcpy(prg_info, &my_prg_info, sizeof(*prg_info));

	// RAR is usable.
	return 0;
}

}
