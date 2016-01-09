/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Lzma.hpp: Lzma archive handler.                                         *
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

#include "Lzma.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

// Byteswapping macros.
#include "libcompat/byteswap.h"

// 7-Zip includes.
#include "lzma/7zAlloc.h"

namespace LibGensFile {

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Lzma::Lzma(const char *filename)
	: LzmaSdk(filename)
#if 0
	, m_inBuf(nullptr), m_outBuf(nullptr)
	, m_inBufSz(0), m_outBufSz(0)
#endif
{
	if (!m_file) {
		return;
	} else if (!filename) {
		fclose(m_file);
		m_file = nullptr;
		m_lastError = EINVAL;
		return;
	}

	// Check for Lzma magic first.
	// If it's not there, this isn't an Lzma archive.
	// NOTE: Lzma magic is a bit weird, so we're checking it here
	// instead of using checkMagic().
	uint8_t header[13];
	rewind(m_file);
	size_t szread = fread(header, 1, sizeof(header), m_file);
	bool is_lzma = false;
	if (szread == sizeof(header)) {
		// First byte indicates properties.
		// XZ-Utils always uses 0x5D.
		// 'file' also checks that the next two bytes are 0.
		if (header[0] == 0x5D && header[1] == 0 && header[2] == 0) {
			// Check the uncompressed file size.
			// it should either be -1 (unknown) or
			// an unsigned value less than 256 GB.
			// TODO: Add 64-bit byteswapping to byteswap.h.
			uint64_t lzsize = header[5] |
				((uint64_t)header[ 6] << 8) |
				((uint64_t)header[ 7] << 16) |
				((uint64_t)header[ 8] << 24) |
				((uint64_t)header[ 9] << 32) |
				((uint64_t)header[10] << 40) |
				((uint64_t)header[11] << 48) |
				((uint64_t)header[12] << 56);
			if (lzsize == ~0ULL || lzsize < (256ULL*1024*1024*1024)) {
				// Size is valid.
				is_lzma = true;
			} else {
				// Size is invalid.
				// TODO: Better error code?
				m_lastError = EINVAL;
			}
		} else {
			// Header is invalid.
			// TODO: Better error code?
			m_lastError = EINVAL;
		}
	} else {
		// Error reading the file.
		m_lastError = errno;
	}

	if (!is_lzma) {
		// Not an Lzma archive.
		if (m_lastError == 0) {
			// Unknown error...
			m_lastError = EIO; // TODO: MDP error code.
		}
		fclose(m_file);
		m_file = nullptr;
		return;
	}

	// Initialize the LZMA SDK.
	int ret = lzmaInit();
	if (ret != 0) {
		// LZMA SDK initialization failed.
		fclose(m_file);
		m_file = nullptr;
		// TODO: Consistently set m_lastError immediately before
		// the return statement in all Archive handlers.
		m_lastError = -ret;
		return;
	}

#if 0
	// TODO: Only read the first stream?
	Xzs_Construct(&m_xzs);

	// Read the Xz footer.
	int64_t startPosition;
	SRes res = Xzs_ReadBackward(&m_xzs, &m_lookStream.s, &startPosition, nullptr, &m_allocImp);
	if (res != SZ_OK || startPosition != 0) {
		// Error reading the Xz footer.
		// TODO: Store non-zero startPosition and use it?
		Xzs_Free(&m_xzs, &m_allocImp);
		File_Close(&m_archiveStream.file);

		fclose(m_file);
		m_file = nullptr,
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Initialize the Xz unpacker.
	XzUnpacker_Construct(&m_xzu, &m_allocImp);
#endif

	// Xz archive is opened.
}

/**
 * Delete the Lzma object.
 */
Lzma::~Lzma()
{
#if 0
	// Free any allocated buffers.
	free(m_inBuf);
	free(m_outBuf);

	// Close the Xz file.
	if (m_file) {
		Xzs_Free(&m_xzs, &m_allocImp);
		// File_Close() is called by LzmaSdk.
	}
#endif
}

/**
 * Close the archive file.
 */
void Lzma::close(void)
{
#if 0
	// Free any allocated buffers.
	free(m_inBuf);
	m_inBuf = nullptr;
	m_inBufSz = 0;
	free(m_outBuf);
	m_outBuf = nullptr;
	m_outBufSz = 0;

	// Close the Xz file.
	if (m_file) {
		Xzs_Free(&m_xzs, &m_allocImp);
		// File_Close() is called by LzmaSdk.
	}
#endif

	// LzmaSdk class closes m_archiveStream.file.
	// Base class closes the FILE*.
	LzmaSdk::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Lzma::getFileInfo(mdp_z_entry_t **z_entry_out)
{
#if 0
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Create a fake mdp_z_entry_t containing the uncompressed
	// filesize and the archive's filename.

	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	if (!z_entry) {
		m_lastError = ENOMEM;
		return -m_lastError;
	}

	// Set the elements of the list entry.
	// FIXME: z_entry->filesize should be changed to int64_t.
	// For filesize, check if zlib is operating on a compressed file.
	// If it is, use gzsize; otherwise, use filesize.
	z_entry->filesize = (size_t)Xzs_GetUnpackSize(&m_xzs);
	z_entry->filename = (!m_filename.empty() ? strdup(m_filename.c_str()) : nullptr);
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0; // TODO: return MDP_ERR_OK;
#endif
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
int Lzma::readFile(const mdp_z_entry_t *z_entry,
		   file_offset_t start_pos, file_offset_t read_len,
		   void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
#if 0
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
	}

	// Seek to the beginning of the file.
	// TODO: Use startPosition from the header?
	// TODO: Check return value?
	int64_t startPosition = 0;
	m_lookStream.s.Seek(&m_lookStream, &startPosition, SZ_SEEK_SET);
	if (startPosition != 0) {
		// Error seeking in the file.
		m_lastError = EIO;
		return -m_lastError;
	}
		
	// Allocate buffers.
	if (!m_inBuf) {
		m_inBufSz = (1 << 15);		// 32 KB
		m_inBuf = (uint8_t*)malloc(m_inBufSz);
	}
	if (!m_outBuf) {
		m_outBufSz = (1 << 21);		// 2 MB
		m_outBuf = (uint8_t*)malloc(m_outBufSz);
	}
		
	// (Re-)Initialize the XzUnpacker.
	XzUnpacker_Init(&m_xzu);

	// Buffer space remaining.
	// NOTE: We're using read_len, not siz,
	// since we only want to read read_len bytes.
	file_offset_t buf_spc_rem = read_len;

	// Read the file into the buffer.
	// Based on LZMA SDK 15.14's XzHandler.cpp: CDecoder::Decode()
	SRes res;
	uint32_t inSize = 0;
	size_t inPos = 0;
	size_t outPos = 0;
	*ret_siz = 0;

	while (true) {
		if (inPos == inSize) {
			inPos = 0;
			// NOTE: ISeqInStream->Read() uses the same
			// variable for buffer size and processed size.
			// The COM version in XzHandler.cpp uses
			// separate parameters for each.
			inSize = m_inBufSz;
			res = m_lookStream.s.Read(&m_lookStream, m_inBuf, &inSize);
			if (res != 0) {
				// TODO: Properly handle this error.
				m_lastError = EIO;
				return -m_lastError;
			}
		}

		size_t inLen = inSize - inPos;
		size_t outLen = m_outBufSz - outPos;
		ECoderStatus status;

		res = XzUnpacker_Code(&m_xzu,
			m_outBuf + outPos, &outLen,
			m_inBuf + inPos, &inLen,
			(inSize == 0 ? CODER_FINISH_END : CODER_FINISH_ANY), &status);

		inPos += inLen;
		outPos += outLen;

		bool finished = ((inLen == 0 && outLen == 0) || res != SZ_OK);

		// Current output buffer.
		// This is adjusted to handle start_pos if necessary.
		uint8_t *outBuf = m_outBuf;
		if (start_pos > 0) {
			// Starting position is set.
			// We want to remove these bytes from the
			// beginning of the input stream.
			if (outPos < start_pos) {
				// Not enough data read yet.
				// Discard the entire buffer.
				start_pos -= outPos;
				outPos = 0;
			} else {
				// We've read at least start_pos bytes.
				outBuf += start_pos;
				outPos -= start_pos;
				start_pos = 0;
			}
		}

		// Have we read any data yet?
		if (outPos > 0) {
			// Copy data to the output buffer.
			if (buf_spc_rem > outPos) {
				// Space is available.
				memcpy(buf, outBuf, outPos);
				buf_spc_rem -= outPos;
				buf = ((uint8_t*)buf + outPos);
				*ret_siz += outPos;
			} else {
				// Either there's not enough space,
				// or this will fill up the buffer.
				memcpy(buf, outBuf, buf_spc_rem);
				*ret_siz += buf_spc_rem;
				buf_spc_rem = 0;
			}
			outPos = 0;
		}

		if (buf_spc_rem <= 0) {
			// Out of space in the buffer.
			// TODO: Error?
			break;
		}

		if (finished) {
			// TODO: ExtraSize stuff?
			// See LZMA SDK 15.14, CPP/7zip/Archive/XzHandler.cpp:597.
			break;
		}
	}

	// Verify that the correct amount of data was read.
	if (res != SZ_OK || *ret_siz != read_len) {
		// Either the LZMA SDK encountered an error,
		// or a short read occurred. Something went wrong.
		// TODO: Convert the 7z error to MDP?
		m_lastError = EIO;
		return -m_lastError;
	}
	return 0; // TODO: return MDP_ERR_OK;
#endif
}

}
