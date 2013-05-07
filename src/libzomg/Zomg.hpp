/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.hpp: Savestate handler.                                            *
 *                                                                         *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#include "ZomgBase.hpp"
#include "libzomg/zomg_byteorder.h"

// MiniZip
#include "minizip/zip.h"
#include "minizip/unzip.h"

namespace LibZomg
{

class Zomg : public ZomgBase
{
	public:
		Zomg(const utf8_str *filename, ZomgFileMode mode);
		virtual ~Zomg(void);
		
		inline bool isOpen(void) const { return (m_mode != ZOMG_CLOSED); }
		void close(void);
		
		/**
		 * DetectFormat(): Detect if a savestate is supported by this class.
		 * @param filename Savestate filename.
		 * @return True if the savestate is supported; false if not.
		 */
		static bool DetectFormat(const utf8_str *filename);
		
		/**
		 * Load savestate functions.
		 * @param siz Number of bytes to read.
		 * @return Bytes read on success; negative on error.
		 * TODO: Standardize error codes.
		 */
		
		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)
		
		/**
		 * Load the preview image.
		 * @param img_buf Image buffer.
		 * @param siz Size of the image buffer.
		 * @return Bytes read on success; negative on error.
		 */
		int loadPreview(void *img_buf, size_t siz);
		
		// VDP
		int loadVdpReg(uint8_t *reg, size_t siz);
		int loadVdpCtrl_8(Zomg_VdpCtrl_8_t *ctrl);
		int loadVdpCtrl_16(Zomg_VdpCtrl_16_t *ctrl);
		int loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder);
		int loadCRam(Zomg_CRam_t *cram, ZomgByteorder_t byteorder);
		int loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder);	/// MD-specific
		
		// Audio
		int loadPsgReg(Zomg_PsgSave_t *state);
		int loadMD_YM2612_reg(Zomg_Ym2612Save_t *state);	/// MD-specific
		
		// Z80
		int loadZ80Mem(uint8_t *mem, size_t siz);
		int loadZ80Reg(Zomg_Z80RegSave_t *state);
		
		// M68K (MD-specific)
		int loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder);
		int loadM68KReg(Zomg_M68KRegSave_t *state);
		
		// MD-specific registers
		int loadMD_IO(Zomg_MD_IoSave_t *state);
		int loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state);
		int loadMD_TimeReg(Zomg_MD_TimeReg_t *state);
		
		// Miscellaneous
		int loadSRam(uint8_t *sram, size_t siz);
		
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
		int saveVdpCtrl_8(const Zomg_VdpCtrl_8_t *ctrl);
		int saveVdpCtrl_16(const Zomg_VdpCtrl_16_t *ctrl);
		int saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder);
		int saveCRam(const Zomg_CRam_t *cram, ZomgByteorder_t byteorder);
		int saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder);	/// MD-specific
		
		// Audio
		int savePsgReg(const Zomg_PsgSave_t *state);
		int saveMD_YM2612_reg(const Zomg_Ym2612Save_t *state);	/// MD-specific
		
		// Z80
		int saveZ80Mem(const uint8_t *mem, size_t siz);
		int saveZ80Reg(const Zomg_Z80RegSave_t *state);
		
		// M68K (MD-specific)
		int saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder);
		int saveM68KReg(const Zomg_M68KRegSave_t *state);
		
		// MD-specific registers
		int saveMD_IO(const Zomg_MD_IoSave_t *state);
		int saveMD_Z80Ctrl(const Zomg_MD_Z80CtrlSave_t *state);
		int saveMD_TimeReg(const Zomg_MD_TimeReg_t *state);
		
		// Miscellaneous
		int saveSRam(const uint8_t *sram, size_t siz);
	
	protected:
		unzFile m_unz;
		zipFile m_zip;
		
		// Current time in Zip format.
		// NOTE: Only used when saving ZOMG files.
		zip_fileinfo m_zipfi;
		
		int initZomgLoad(const utf8_str *filename);
		int initZomgSave(const utf8_str *filename);
		
		int loadFromZomg(const utf8_str *filename, void *buf, int len);
		int saveToZomg(const utf8_str *filename, const void *buf, int len);
};

}

#endif /* __LIBZOMG_ZOMG_HPP__ */
