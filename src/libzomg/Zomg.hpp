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

class ZomgIni;
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
		virtual void close(void) override;

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
		 * @param img_buf Image buffer.
		 * @param siz Size of the image buffer.
		 * @return Bytes read on success; negative on error.
		 */
		virtual int loadPreview(void *img_buf, size_t siz) override;

		// VDP
		virtual int loadVdpReg(uint8_t *reg, size_t siz) override;
		virtual int loadVdpCtrl_8(Zomg_VdpCtrl_8_t *ctrl) override;
		virtual int loadVdpCtrl_16(Zomg_VdpCtrl_16_t *ctrl) override;
		virtual int loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int loadCRam(Zomg_CRam_t *cram, ZomgByteorder_t byteorder) override;
		/// MD-specific
		virtual int loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int loadMD_VDP_SAT(uint16_t *vdp_sat, size_t siz, ZomgByteorder_t byteorder) override;

		// Audio
		virtual int loadPsgReg(Zomg_PsgSave_t *state) override;
		/// MD-specific
		virtual int loadMD_YM2612_reg(Zomg_Ym2612Save_t *state) override;

		// Z80
		virtual int loadZ80Mem(uint8_t *mem, size_t siz) override;
		virtual int loadZ80Reg(Zomg_Z80RegSave_t *state) override;

		// M68K (MD-specific)
		virtual int loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int loadM68KReg(Zomg_M68KRegSave_t *state) override;

		// MD-specific registers
		virtual int loadMD_IO(Zomg_MD_IoSave_t *state) override;
		virtual int loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state) override;
		virtual int loadMD_TimeReg(Zomg_MD_TimeReg_t *state) override;
		virtual int loadMD_TMSS_reg(Zomg_MD_TMSS_reg_t *tmss) override;

		// Miscellaneous
		virtual int loadSRam(uint8_t *sram, size_t siz) override;

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
		 * @param zomgIni ZomgIni class with information about the savestate.
		 * @return 0 on success; non-zero on error.
		 */
		int saveZomgIni(const ZomgIni *zomgIni);

		/**
		 * Save the preview image.
		 * @param img_buf Image buffer. (Must have a PNG image.)
		 * @param siz Size of the image buffer.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int savePreview(const void *img_buf, size_t siz) override;

		// VDP
		virtual int saveVdpReg(const uint8_t *reg, size_t siz) override;
		virtual int saveVdpCtrl_8(const Zomg_VdpCtrl_8_t *ctrl) override;
		virtual int saveVdpCtrl_16(const Zomg_VdpCtrl_16_t *ctrl) override;
		virtual int saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int saveCRam(const Zomg_CRam_t *cram, ZomgByteorder_t byteorder) override;
		/// MD-specific
		virtual int saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int saveMD_VDP_SAT(const void *vdp_sat, size_t siz, ZomgByteorder_t byteorder) override;

		// Audio
		virtual int savePsgReg(const Zomg_PsgSave_t *state) override;
		/// MD-specific
		virtual int saveMD_YM2612_reg(const Zomg_Ym2612Save_t *state) override;

		// Z80
		virtual int saveZ80Mem(const uint8_t *mem, size_t siz) override;
		virtual int saveZ80Reg(const Zomg_Z80RegSave_t *state) override;

		// M68K (MD-specific)
		virtual int saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder) override;
		virtual int saveM68KReg(const Zomg_M68KRegSave_t *state) override;

		// MD-specific registers
		virtual int saveMD_IO(const Zomg_MD_IoSave_t *state) override;
		virtual int saveMD_Z80Ctrl(const Zomg_MD_Z80CtrlSave_t *state) override;
		virtual int saveMD_TimeReg(const Zomg_MD_TimeReg_t *state) override;
		virtual int saveMD_TMSS_reg(const Zomg_MD_TMSS_reg_t *tmss) override;

		// Miscellaneous
		virtual int saveSRam(const uint8_t *sram, size_t siz) override;
};

}

#endif /* __LIBZOMG_ZOMG_HPP__ */
