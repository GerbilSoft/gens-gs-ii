/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgBase.hpp: Savestate base class.                                     *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBZOMG_ZOMGBASE_HPP__
#define __LIBZOMG_ZOMGBASE_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// utf8_str
// TODO: Don't depend on LibGens.
#include "../libgens/macros/common.h"

// ZOMG byteorder.
#include "libzomg/zomg_byteorder.h"

// ZOMG save structs.
struct _Zomg_VdpCtrl_8_t;
struct _Zomg_VdpCtrl_16_t;
union _Zomg_CRam_t;
struct _Zomg_PsgSave_t;
struct _Zomg_Ym2612Save_t;
struct _Zomg_Z80RegSave_t;
struct _Zomg_M68KRegSave_t;
struct _Zomg_MD_IoSave_t;
struct _Zomg_MD_Z80CtrlSave_t;
struct _Zomg_MD_TimeReg_t;
struct _Zomg_MD_TMSS_reg_t;
struct _Zomg_EPR_ctrl_t;

namespace LibZomg {

class ZomgBase
{
	public:
		enum ZomgFileMode {
			ZOMG_CLOSED,
			ZOMG_LOAD,
			ZOMG_SAVE
		};
		
		ZomgBase(const utf8_str *filename, ZomgFileMode mode)
		{
			(void)filename; (void)mode;	// Unused parameters.
			
			m_mode = ZOMG_CLOSED;	// No file open initially.
			m_preview_size = 0;	// Assume no preview image by default.
		}
		virtual ~ZomgBase() { }		// NOTE: Can't call close() here because close() is virtual.
		
		inline bool isOpen(void) const
			{ return (m_mode != ZOMG_CLOSED); }
		virtual void close(void) = 0;
		
		/**
		 * DetectFormat(): Detect if a savestate is supported by this class.
		 * @param filename Savestate filename.
		 * @return True if the savestate is supported; false if not.
		 */
		static bool DetectFormat(const utf8_str *filename)
			{ (void)filename; return false; }
		
		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)
		
		/**
		 * Get the size of the preview image.
		 * @return Size of the preview image, or 0 if none was found.
		 */
		inline size_t getPreviewSize(void) const { return m_preview_size; }
		
		/**
		 * Load the preview image.
		 * @param img_buf Image buffer.
		 * @param siz Size of the image buffer.
		 * @return Bytes read on success; negative on error.
		 */
		virtual int loadPreview(void *img_buf, size_t siz)
			{ (void)img_buf; (void)siz; return 0; }
		
		// VDP
		virtual int loadVdpReg(uint8_t *reg, size_t siz)
			{ (void)reg; (void)siz; return 0; }
		virtual int loadVdpCtrl_8(_Zomg_VdpCtrl_8_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int loadVdpCtrl_16(_Zomg_VdpCtrl_16_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder)
			{ (void)vram; (void)siz; (void)byteorder; return 0; }
		virtual int loadCRam(_Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
			{ (void)cram; (void)byteorder; return 0; }
		/// MD-specific
		virtual int loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
			{ (void)vsram; (void)siz; (void)byteorder; return 0; }
		
		// Audio
		virtual int loadPsgReg(_Zomg_PsgSave_t *state)
			{ (void)state; return 0; }
		virtual int loadMD_YM2612_reg(_Zomg_Ym2612Save_t *state) /// MD-specific
			{ (void)state; return 0; }
		
		// Z80
		virtual int loadZ80Mem(uint8_t *mem, size_t siz)
			{ (void)mem; (void)siz; return 0; }
		virtual int loadZ80Reg(_Zomg_Z80RegSave_t *state)
			{ (void)state; return 0; }

		// M68K (MD-specific)
		virtual int loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
			{ (void)mem; (void)siz; (void)byteorder; return 0; }
		virtual int loadM68KReg(_Zomg_M68KRegSave_t *state)
			{ (void)state; return 0; }
		
		// MD-specific registers
		virtual int loadMD_IO(_Zomg_MD_IoSave_t *state)
			{ (void)state; return 0; }
		virtual int loadMD_Z80Ctrl(_Zomg_MD_Z80CtrlSave_t *state)
			{ (void)state; return 0; }
		virtual int loadMD_TimeReg(_Zomg_MD_TimeReg_t *state)
			{ (void)state; return 0; }
		virtual int loadMD_TMSS_reg(_Zomg_MD_TMSS_reg_t *tmss)
			{ (void)tmss; return 0; }
		
		// Miscellaneous
		virtual int loadSRam(uint8_t *sram, size_t siz)
			{ (void)sram; (void)siz; return 0; }
		virtual int loadEEPRomCtrl(_Zomg_EPR_ctrl_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int loadEEPRom(uint8_t *eeprom, size_t siz)
			{ (void)eeprom; (void)siz; return 0; }
		
		/**
		 * Save savestate functions.
		 * @param siz Number of bytes to write.
		 * @return 0 on success; non-zero on error.
		 * TODO: Standardize error codes.
		 */
		
		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)
		
		/**
		 * Save the preview image.
		 * @param img_buf Image buffer. (Must have a PNG image.)
		 * @param siz Size of the image buffer.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int savePreview(const void *img_buf, size_t siz)
			{ (void)img_buf; (void)siz; return 0; }
		
		// VDP
		virtual int saveVdpReg(const uint8_t *reg, size_t siz)
			{ (void)reg; (void)siz; return 0; }
		virtual int saveVdpCtrl_8(const _Zomg_VdpCtrl_8_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int saveVdpCtrl_16(const _Zomg_VdpCtrl_16_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder)
			{ (void)vram; (void)siz; (void)byteorder; return 0; }
		virtual int saveCRam(const _Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
			{ (void)cram; (void)byteorder; return 0; }
		/// MD-specific
		virtual int saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
			{ (void)vsram; (void)siz; (void)byteorder; return 0; }
		
		// Audio
		virtual int savePsgReg(const _Zomg_PsgSave_t *state)
			{ (void)state; return 0; }
		/// MD-specific
		virtual int saveMD_YM2612_reg(const _Zomg_Ym2612Save_t *state)
			{ (void)state; return 0; }
		
		// Z80
		virtual int saveZ80Mem(const uint8_t *mem, size_t siz)
			{ (void)mem; (void)siz; return 0; }
		virtual int saveZ80Reg(const _Zomg_Z80RegSave_t *state)
			{ (void)state; return 0; }
		
		// M68K (MD-specific)
		virtual int saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
			{ (void)mem; (void)siz; (void)byteorder; return 0; }
		virtual int saveM68KReg(const _Zomg_M68KRegSave_t *state)
			{ (void)state; return 0; }
		
		// MD-specific registers
		virtual int saveMD_IO(const _Zomg_MD_IoSave_t *state)
			{ (void)state; return 0; }
		virtual int saveMD_Z80Ctrl(const _Zomg_MD_Z80CtrlSave_t *state)
			{ (void)state; return 0; }
		virtual int saveMD_TimeReg(const _Zomg_MD_TimeReg_t *state)
			{ (void)state; return 0; }
		virtual int saveMD_TMSS_reg(const _Zomg_MD_TMSS_reg_t *tmss)
			{ (void)tmss; return 0; }

		// Miscellaneous
		virtual int saveSRam(const uint8_t *sram, size_t siz)
			{ (void)sram; (void)siz; return 0; }
		virtual int saveEEPRomCtrl(const _Zomg_EPR_ctrl_t *ctrl)
			{ (void)ctrl; return 0; }
		virtual int saveEEPRom(const uint8_t *eeprom, size_t siz)
			{ (void)eeprom; (void)siz; return 0; }

	protected:
		std::string m_filename;
		ZomgFileMode m_mode;
		
		// PNG preview image.
		size_t m_preview_size;
};

}

#endif /* __LIBZOMG_ZOMGBASE_HPP__ */
