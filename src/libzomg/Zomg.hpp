/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.hpp: Savestate handler.                                            *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#ifndef __LIBZOMG_ZOMG_HPP__
#define __LIBZOMG_ZOMG_HPP__

#include "ZomgBase.hpp"

namespace LibZomg {

class Metadata;

class ZomgPrivate;
class Zomg : public ZomgBase
{
	public:
		Zomg(const utf8_str *filename, ZomgFileMode mode);
		virtual ~Zomg(void);

	protected:
		friend class ZomgPrivate;
		ZomgPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		Zomg(const Zomg &);
		Zomg &operator=(const Zomg &);

	public:
		virtual void close(void) final;

		/**
		 * Detect if a savestate is supported by this class.
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
		 * TODO: Metadata?
		 * @param img_data Image data. (Caller must free img_data->data.)
		 * @return 0 on success; non-zero on error.
		 */
		virtual int loadPreview(_Zomg_Img_Data_t *img_data) final;

		// VDP
		virtual int loadVdpReg(uint8_t *reg, size_t siz) final;
		virtual int loadVdpCtrl_8(_Zomg_VDP_ctrl_8_t *ctrl) final;
		virtual int loadVdpCtrl_16(_Zomg_VDP_ctrl_16_t *ctrl) final;
		virtual int loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int loadCRam(_Zomg_CRam_t *cram, ZomgByteorder_t byteorder) final;
		/// MD-specific
		virtual int loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int loadMD_VDP_SAT(uint16_t *vdp_sat, size_t siz, ZomgByteorder_t byteorder) final;

		// Audio
		virtual int loadPsgReg(_Zomg_PsgSave_t *state) final;
		/// MD-specific
		virtual int loadMD_YM2612_reg(_Zomg_Ym2612Save_t *state) final;

		// Z80
		virtual int loadZ80Mem(uint8_t *mem, size_t siz) final;
		virtual int loadZ80Reg(_Zomg_Z80RegSave_t *state) final;

		// M68K (MD-specific)
		virtual int loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int loadM68KReg(_Zomg_M68KRegSave_t *state) final;

		// MD-specific registers
		virtual int loadMD_IO(_Zomg_MD_IoSave_t *state) final;
		virtual int loadMD_Z80Ctrl(_Zomg_MD_Z80CtrlSave_t *state) final;
		virtual int loadMD_TimeReg(_Zomg_MD_TimeReg_t *state) final;
		virtual int loadMD_TMSS_reg(_Zomg_MD_TMSS_reg_t *tmss) final;

		// Miscellaneous
		virtual int loadSRam(uint8_t *sram, size_t siz) final;
		virtual int loadEEPRomCtrl(_Zomg_EPR_ctrl_t *ctrl) final;
		virtual int loadEEPRomCache(uint8_t *cache, size_t siz) final;
		virtual int loadEEPRom(uint8_t *eeprom, size_t siz) final;

		/**
		 * Save savestate functions.
		 * @param siz Number of bytes to write.
		 * @return 0 on success; non-zero on error.
		 * TODO: Standardize error codes.
		 */

		// TODO: Determine siz and is16bit from the system type?
		// (once FORMAT.ini is implemented)

		/**
		 * Save ZOMG.ini.
		 * This function MUST be called before any other function when saving!
		 * @param metadata Metadata class with information about the savestate.
		 * @return 0 on success; non-zero on error.
		 */
		int saveZomgIni(const Metadata *metadata);

		/**
		 * Save the preview image.
		 *
		 * NOTE: Defined here as well as in ZomgBase because otherwise,
		 * gcc and MSVC will complain that there's no matching function call.
		 * FIXME: Figure out why, and/or remove this function.
		 *
		 * No metadata other than creation time will be saved.
		 * @param img_data	[in] Image data.
		 * @return 0 on success; non-zero on error.
		 */
		int savePreview(const _Zomg_Img_Data_t *img_data)
			{ return savePreview(img_data, nullptr, -1 /*Metadata::MF_Default*/); }

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
		virtual int saveVdpReg(const uint8_t *reg, size_t siz) final;
		virtual int saveVdpCtrl_8(const _Zomg_VDP_ctrl_8_t *ctrl) final;
		virtual int saveVdpCtrl_16(const _Zomg_VDP_ctrl_16_t *ctrl) final;
		virtual int saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int saveCRam(const _Zomg_CRam_t *cram, ZomgByteorder_t byteorder) final;
		/// MD-specific
		virtual int saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int saveMD_VDP_SAT(const void *vdp_sat, size_t siz, ZomgByteorder_t byteorder) final;

		// Audio
		virtual int savePsgReg(const _Zomg_PsgSave_t *state) final;
		/// MD-specific
		virtual int saveMD_YM2612_reg(const _Zomg_Ym2612Save_t *state) final;

		// Z80
		virtual int saveZ80Mem(const uint8_t *mem, size_t siz) final;
		virtual int saveZ80Reg(const _Zomg_Z80RegSave_t *state) final;

		// M68K (MD-specific)
		virtual int saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder) final;
		virtual int saveM68KReg(const _Zomg_M68KRegSave_t *state) final;

		// MD-specific registers
		virtual int saveMD_IO(const _Zomg_MD_IoSave_t *state) final;
		virtual int saveMD_Z80Ctrl(const _Zomg_MD_Z80CtrlSave_t *state) final;
		virtual int saveMD_TimeReg(const _Zomg_MD_TimeReg_t *state) final;
		virtual int saveMD_TMSS_reg(const _Zomg_MD_TMSS_reg_t *tmss) final;

		// Miscellaneous
		virtual int saveSRam(const uint8_t *sram, size_t siz) final;
		virtual int saveEEPRomCtrl(const _Zomg_EPR_ctrl_t *ctrl) final;
		virtual int saveEEPRomCache(const uint8_t *cache, size_t siz) final;
		virtual int saveEEPRom(const uint8_t *eeprom, size_t siz) final;
};

}

#endif /* __LIBZOMG_ZOMG_HPP__ */
