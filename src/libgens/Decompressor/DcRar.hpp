/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcRar.hpp: RAR decompressor class.                                      *
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

#ifndef __LIBGENS_DECOMPRESSOR_DCRAR_HPP__
#define __LIBGENS_DECOMPRESSOR_DCRAR_HPP__

#include "Decompressor.hpp"

#ifdef _WIN32
#include "UnRAR_dll.hpp"
#endif

// C++ includes.
#include <string>

namespace LibGens
{

class DcRar : public Decompressor
{
	public:
		DcRar(FILE *f, const utf8_str *filename);
		~DcRar();
		
		/**
		 * DetectFormat(): Detect if the file can be handled by this decompressor.
		 * This function should be reimplemented by derived classes.
		 * NOTE: Do NOT call this function like a virtual function!
		 * @param f File pointer.
		 * @return True if the file can be handled by this decompressor.
		 */
		static bool DetectFormat(FILE *f);
		
		/**
		 * getFileInfo(): Get information about all files in the archive.
		 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
		 * @return MDP error code. [TODO]
		 */
		int getFileInfo(mdp_z_entry_t **z_entry_out);
		
		/**
		 * getFile(): Get a file from the archive.
		 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
		 * @param buf		[out] Buffer to read the file into.
		 * @param siz		[in]  Size of buf.
		 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
		 * @return MDP error code. [TODO]
		 */
		int getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz);
	
	protected:
		/**
		 * ms_RarBinary: RAR executable filename.
		 * Unix: Filename of "rar" or "unrar".
		 * Windows: Filename of "unrar.dll".
		 */
		static std::string ms_RarBinary;
		
#ifdef _WIN32
		/**
		 * m_unicode: True if the system supports Unicode.
		 * TODO: Move to a "mini w32u" subsystem?
		 */
		bool m_unicode;
		
		/**
		 * m_unrarDll: UnRAR DLL.
		 * Win32 only!
		 */
		UnRAR_dll m_unrarDll;
		
		/**
		 * RarState_t: RAR state struct.
		 */
		struct RarState_t
		{
			uint8_t *buf;	// Buffer.
			size_t siz;	// Size of buffer.
			size_t pos;	// Current position.
			
			DcRar *owner;	// DcRar instance that owns this RarState_t.
		};
		
		/**
		 * RarCallback(): Win32 UnRAR.dll callback function. [STATIC]
		 */
		static int CALLBACK RarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);
		
		/**
		 * rarCallback(): Win32 UnRAR.dll callback function.
		 */
		static int CALLBACK rarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);
#endif
};

}

#endif /* __LIBGENS_DECOMPRESSOR_DECOMPRESSOR_HPP__ */
