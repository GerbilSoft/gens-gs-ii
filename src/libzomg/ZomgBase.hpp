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

// C includes. (C++ namespace)
#include <ctime>

// C++ includes.
#include <string>

// utf8_str
// TODO: Don't depend on LibGens.
#include "../libgens/macros/common.h"

// ZOMG byteorder.
#include "libzomg/zomg_byteorder.h"

// ZOMG save structs.
struct _Zomg_VDP_ctrl_8_t;
struct _Zomg_VDP_ctrl_16_t;
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

// Image data struct.
// Used for preview images.
extern "C" struct _Zomg_Img_Data_t;

namespace LibZomg {

class Metadata;

class ZomgBase
{
	public:
		enum ZomgFileMode {
			ZOMG_CLOSED,
			ZOMG_LOAD,
			ZOMG_SAVE
		};

		ZomgBase(const utf8_str *filename, ZomgFileMode mode);
		virtual ~ZomgBase();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		ZomgBase(const ZomgBase &);
		ZomgBase &operator=(const ZomgBase &);

	public:
		inline bool isOpen(void) const
			{ return (m_mode != ZOMG_CLOSED); }
		virtual void close(void) = 0;

		/**
		 * Get the last error code.
		 * @return 0 if no error; negative errno on error.
		 */
		inline int lastError(void) const
			{ return m_lastError; }

		/**
		 * Detect if a savestate is supported by this class.
		 * @param filename Savestate filename.
		 * @return True if the savestate is supported; false if not.
		 */
		static bool DetectFormat(const utf8_str *filename);

		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)

		// ZOMG functions will return an error code
		// as well as set m_lastError if an error occurs.
		// On success, they will return bytes read and
		// set m_lastError to 0.

		/** General metadata functions. **/

		/**
		 * Get the savestate's modification time.
		 * @return File modification time. (If 0, mtime is invalid.)
		 */
		time_t mtime(void) const;

		/** Load functions. **/

		/**
		 * Load the preview image.
		 * TODO: Metadata?
		 * @param img_data Image data. (Caller must free img_data->data.)
		 * @return Bytes read on success; negative errno on error.
		 */
		virtual int loadPreview(_Zomg_Img_Data_t *img_data);

		// VDP
		virtual int loadVdpReg(uint8_t *reg, size_t siz);
		virtual int loadVdpCtrl_8(_Zomg_VDP_ctrl_8_t *ctrl);
		virtual int loadVdpCtrl_16(_Zomg_VDP_ctrl_16_t *ctrl);
		virtual int loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder);
		virtual int loadCRam(_Zomg_CRam_t *cram, ZomgByteorder_t byteorder);
		/// MD-specific
		virtual int loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder);
		virtual int loadMD_VDP_SAT(uint16_t *vdp_sat, size_t siz, ZomgByteorder_t byteorder);

		// Audio
		virtual int loadPsgReg(_Zomg_PsgSave_t *state);
		/// MD-specific
		virtual int loadMD_YM2612_reg(_Zomg_Ym2612Save_t *state);

		// Z80
		virtual int loadZ80Mem(uint8_t *mem, size_t siz);
		virtual int loadZ80Reg(_Zomg_Z80RegSave_t *state);

		// M68K (MD-specific)
		virtual int loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder);
		virtual int loadM68KReg(_Zomg_M68KRegSave_t *state);

		// MD-specific registers
		virtual int loadMD_IO(_Zomg_MD_IoSave_t *state);
		virtual int loadMD_Z80Ctrl(_Zomg_MD_Z80CtrlSave_t *state);
		virtual int loadMD_TimeReg(_Zomg_MD_TimeReg_t *state);
		virtual int loadMD_TMSS_reg(_Zomg_MD_TMSS_reg_t *tmss);

		// Miscellaneous
		virtual int loadSRam(uint8_t *sram, size_t siz);
		virtual int loadEEPRomCtrl(_Zomg_EPR_ctrl_t *ctrl);
		virtual int loadEEPRomCache(uint8_t *cache, size_t siz);
		virtual int loadEEPRom(uint8_t *eeprom, size_t siz);

		/** Save functions. **/

		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)

		/**
		 * Save the preview image.
		 * No metadata other than creation time will be saved.
		 * @param img_data	[in] Image data.
		 * @return 0 on success; non-zero on error.
		 */
		int savePreview(const _Zomg_Img_Data_t *img_data);

		/**
		 * Save the preview image.
		 * @param img_data	[in] Image data.
		 * @param metadata	[in, opt] Extra metadata.
		 * @param metaFlags	[in, opt] Metadata flags.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int savePreview(const _Zomg_Img_Data_t *img_data,
					const Metadata *metadata, int metaFlags);

		// VDP
		virtual int saveVdpReg(const uint8_t *reg, size_t siz);
		virtual int saveVdpCtrl_8(const _Zomg_VDP_ctrl_8_t *ctrl);
		virtual int saveVdpCtrl_16(const _Zomg_VDP_ctrl_16_t *ctrl);
		virtual int saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder);
		virtual int saveCRam(const _Zomg_CRam_t *cram, ZomgByteorder_t byteorder);
		/// MD-specific
		virtual int saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder);
		virtual int saveMD_VDP_SAT(const void *vdp_sat, size_t siz, ZomgByteorder_t byteorder);

		// Audio
		virtual int savePsgReg(const _Zomg_PsgSave_t *state);
		/// MD-specific
		virtual int saveMD_YM2612_reg(const _Zomg_Ym2612Save_t *state);

		// Z80
		virtual int saveZ80Mem(const uint8_t *mem, size_t siz);
		virtual int saveZ80Reg(const _Zomg_Z80RegSave_t *state);

		// M68K (MD-specific)
		virtual int saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder);
		virtual int saveM68KReg(const _Zomg_M68KRegSave_t *state);

		// MD-specific registers
		virtual int saveMD_IO(const _Zomg_MD_IoSave_t *state);
		virtual int saveMD_Z80Ctrl(const _Zomg_MD_Z80CtrlSave_t *state);
		virtual int saveMD_TimeReg(const _Zomg_MD_TimeReg_t *state);
		virtual int saveMD_TMSS_reg(const _Zomg_MD_TMSS_reg_t *tmss);

		// Miscellaneous
		virtual int saveSRam(const uint8_t *sram, size_t siz);
		virtual int saveEEPRomCtrl(const _Zomg_EPR_ctrl_t *ctrl);
		virtual int saveEEPRomCache(const uint8_t *cache, size_t siz);
		virtual int saveEEPRom(const uint8_t *eeprom, size_t siz);

	protected:
		// TODO: Move to a private class?
		std::string m_filename;
		ZomgFileMode m_mode;
		int m_lastError;

		/**
		 * Modified Time.
		 * ZOMG_SAVE: Initialized to the current time.
		 * ZOMG_LOAD: Initialized to 0.
		 *
		 * Subclasses should load the file's mtime in
		 * their constructors.
		 *
		 * NOTE: Save files may have an internal CreationTime.
		 * That should be used in favor of the file system's
		 * mtime if available.
		 *
		 * NOTE: If this value is 0, it should be assumed to
		 * be "invalid". (Genesis emulators in 1970?)
		 *
		 * TODO: 64-bit time_t on 32-bit Linux?
		 */
		time_t m_mtime;
};

}

#endif /* __LIBZOMG_ZOMGBASE_HPP__ */
