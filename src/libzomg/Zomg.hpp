/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.hpp: Savestate handler.                                            *
 *                                                                         *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#ifndef __LIBZOMG_ZOMG_HPP__
#define __LIBZOMG_ZOMG_HPP__

// MiniZip
#include "../extlib/minizip/zip.h"
#include "../extlib/minizip/unzip.h"

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// utf8_str
#include "../libgens/macros/common.h"

// ZOMG save structs.
#include "zomg_vdp.h"
#include "zomg_psg.h"
#include "zomg_ym2612.h"
#include "zomg_m68k.h"
#include "zomg_z80.h"
#include "zomg_md_io.h"
#include "zomg_md_z80_ctrl.h"

namespace LibZomg
{

class Zomg
{
	public:
		enum ZomgFileMode
		{
			ZOMG_CLOSED,
			ZOMG_LOAD,
			ZOMG_SAVE
		};
		Zomg(const utf8_str *filename, ZomgFileMode mode);
		~Zomg();
		
		inline bool isOpen(void) const { return (m_mode != ZOMG_CLOSED); }
		void close(void);
		
		/**
		 * Load savestate functions.
		 * @param siz Number of bytes to read.
		 * @return Bytes read on success; negative on error.
		 * TODO: Standardize error codes.
		 */
		
		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)
		
		// VDP
		int loadVdpReg(uint8_t *reg, size_t siz);
		int loadVRam(void *vram, size_t siz, bool byteswap);
		int loadCRam(void *cram, size_t siz, bool byteswap);
		int loadMD_VSRam(uint16_t *vsram, size_t siz, bool byteswap);	/// MD-specific
		
		// Audio
		int loadPsgReg(Zomg_PsgSave_t *state);
		int loadMD_YM2612_reg(Zomg_Ym2612Save_t *state);	/// MD-specific
		
		// Z80
		int loadZ80Mem(uint8_t *mem, size_t siz);
		int loadZ80Reg(Zomg_Z80RegSave_t *state);
		
		// M68K (MD-specific)
		int loadM68KMem(uint16_t *mem, size_t siz, bool byteswap);
		int loadM68KReg(Zomg_M68KRegSave_t *state);
		
		// MD-specific registers.
		int loadMD_IO(Zomg_MD_IoSave_t *state);
		int loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state);
		
		/**
		 * Save savestate functions.
		 * @param siz Number of bytes to write.
		 * @return 0 on success; non-zero on error.
		 * TODO: Standardize error codes.
		 */
		
		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)
		
		/**
		 * savePreview(): Save the preview image.
		 * @param img_buf Image buffer. (Must have a PNG image.)
		 * @param siz Size of the image buffer.
		 * @return 0 on success; non-zero on error.
		 */
		int savePreview(const void *img_buf, size_t siz);
		
		// VDP
		int saveVdpReg(const uint8_t *reg, size_t siz);
		int saveVRam(const void *vram, size_t siz, bool byteswap);
		int saveCRam(const void *cram, size_t siz, bool byteswap);
		int saveMD_VSRam(const uint16_t *vsram, size_t siz, bool byteswap);	/// MD-specific
		
		// Audio
		int savePsgReg(const Zomg_PsgSave_t *state);
		int saveMD_YM2612_reg(const Zomg_Ym2612Save_t *state);	/// MD-specific
		
		// Z80
		int saveZ80Mem(const uint8_t *mem, size_t siz);
		int saveZ80Reg(const Zomg_Z80RegSave_t *state);
		
		// M68K (MD-specific)
		int saveM68KMem(const uint16_t *mem, size_t siz, bool byteswap);
		int saveM68KReg(const Zomg_M68KRegSave_t *state);
		
		// MD-specific registers.
		int saveMD_IO(const Zomg_MD_IoSave_t *state);
		int saveMD_Z80Ctrl(const Zomg_MD_Z80CtrlSave_t *state);
	
	protected:
		std::string m_filename;
		ZomgFileMode m_mode;
		unzFile m_unz;
		zipFile m_zip;
		
		int loadFromZomg(const utf8_str *filename, void *buf, int len);
		int saveToZomg(const utf8_str *filename, const void *buf, int len);
};

}

#endif /* __LIBZOMG_ZOMG_HPP__ */
