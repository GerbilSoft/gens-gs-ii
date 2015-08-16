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

#include "ZomgBase.hpp"
#include "Metadata.hpp"

namespace LibZomg {

ZomgBase::ZomgBase(const char *filename, ZomgFileMode mode)
	: m_mode(ZOMG_CLOSED)	// No file open initially.
	, m_lastError(0)
	, m_mtime(0)
{
	((void)filename);

	if (m_mode == ZOMG_SAVE) {
		// Initialize m_mtime.
		m_mtime = time(nullptr);
	}
}

ZomgBase::~ZomgBase()
{
	// NOTE: Can't call close() here because close() is virtual.
	// Subclass destructor has to call close();
}

/**
 * Detect if a savestate is supported by this class.
 * @param filename Savestate filename.
 * @return True if the savestate is supported; false if not.
 */
bool ZomgBase::DetectFormat(const char *filename)
{
	// TODO: Replace DetectFormat() with a factory class?
	((void)filename);
	return false;
}

/** Metadata functions. **/

/**
 * Get the savestate's modification time.
 * @return File modification time. (If 0, mtime is invalid.)
 */
time_t ZomgBase::mtime(void) const
{
	return m_mtime;
}

/** Load functions. **/

/**
 * Load the preview image.
 * TODO: Metadata?
 * @param img_data Image data. (Caller must free img_data->data.)
 * @return Bytes read on success; negative errno on error.
 */
int ZomgBase::loadPreview(_Zomg_Img_Data_t *img_data)
{
	((void)img_data);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** VDP **/

int ZomgBase::loadVdpReg(uint8_t *reg, size_t siz)
{
	((void)reg);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadVdpCtrl_8(_Zomg_VDP_ctrl_8_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadVdpCtrl_16(_Zomg_VDP_ctrl_16_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vram);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadCRam(_Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
{
	((void)cram);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** VDP (MD-specific **/

int ZomgBase::loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vsram);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadMD_VDP_SAT(uint16_t *vdp_sat, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vdp_sat);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Audio **/

int ZomgBase::loadPsgReg(_Zomg_PsgSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Audio (MD-specific) **/

int ZomgBase::loadMD_YM2612_reg(_Zomg_Ym2612Save_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Z80 **/

int ZomgBase::loadZ80Mem(uint8_t *mem, size_t siz)
{
	((void)mem);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadZ80Reg(_Zomg_Z80RegSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** M68K (MD-specific) **/

int ZomgBase::loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
{
	((void)mem);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadM68KReg(_Zomg_M68KRegSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** MD-specific registers **/
int ZomgBase::loadMD_IO(_Zomg_MD_IoSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadMD_Z80Ctrl(_Zomg_MD_Z80CtrlSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadMD_TimeReg(_Zomg_MD_TimeReg_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadMD_TMSS_reg(_Zomg_MD_TMSS_reg_t *tmss)
{
	((void)tmss);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Miscellaneous **/

int ZomgBase::loadSRam(uint8_t *sram, size_t siz)
{
	((void)sram);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadEEPRomCtrl(_Zomg_EPR_ctrl_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadEEPRomCache(uint8_t *cache, size_t siz)
{
	((void)cache);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::loadEEPRom(uint8_t *eeprom, size_t siz)
{
	((void)eeprom);
	((void)siz);
	return -ENOSYS;
}

/** Save functions. **/

/**
 * Save the preview image.
 * No metadata other than creation time will be saved.
 * @param img_data	[in] Image data.
 * @return 0 on success; non-zero on error.
 */
int ZomgBase::savePreview(const _Zomg_Img_Data_t *img_data)
{
	return savePreview(img_data, nullptr, Metadata::MF_Default);
}

/**
 * Save the preview image.
 * @param img_data	[in] Image data.
 * @param metadata	[in, opt] Extra metadata.
 * @param metaFlags	[in, opt] Metadata flags.
 * @return 0 on success; non-zero on error.
 */
int ZomgBase::savePreview(const _Zomg_Img_Data_t *img_data,
			const Metadata *metadata, int metaFlags)
{
	((void)img_data);
	((void)metadata);
	((void)metaFlags);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** VDP **/

int ZomgBase::saveVdpReg(const uint8_t *reg, size_t siz)
{
	((void)reg);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveVdpCtrl_8(const _Zomg_VDP_ctrl_8_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return 0;
}

int ZomgBase::saveVdpCtrl_16(const _Zomg_VDP_ctrl_16_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vram);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveCRam(const _Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
{
	((void)cram);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** VDP (MD-specific) **/

int ZomgBase::saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vsram);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}
int ZomgBase::saveMD_VDP_SAT(const void *vdp_sat, size_t siz, ZomgByteorder_t byteorder)
{
	((void)vdp_sat);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Audio **/

int ZomgBase::savePsgReg(const _Zomg_PsgSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Audio (MD-specific) **/

int ZomgBase::saveMD_YM2612_reg(const _Zomg_Ym2612Save_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Z80 **/

int ZomgBase::saveZ80Mem(const uint8_t *mem, size_t siz)
{
	((void)mem);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveZ80Reg(const _Zomg_Z80RegSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** M68K (MD-specific) **/

int ZomgBase::saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
{
	((void)mem);
	((void)siz);
	((void)byteorder);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveM68KReg(const _Zomg_M68KRegSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** MD-specific registers **/

int ZomgBase::saveMD_IO(const _Zomg_MD_IoSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveMD_Z80Ctrl(const _Zomg_MD_Z80CtrlSave_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveMD_TimeReg(const _Zomg_MD_TimeReg_t *state)
{
	((void)state);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveMD_TMSS_reg(const _Zomg_MD_TMSS_reg_t *tmss)
{
	((void)tmss);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

/** Miscellaneous **/

int ZomgBase::saveSRam(const uint8_t *sram, size_t siz)
{
	((void)sram);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveEEPRomCtrl(const _Zomg_EPR_ctrl_t *ctrl)
{
	((void)ctrl);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveEEPRomCache(const uint8_t *cache, size_t siz)
{
	((void)cache);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

int ZomgBase::saveEEPRom(const uint8_t *eeprom, size_t siz)
{
	((void)eeprom);
	((void)siz);
	m_lastError = -ENOSYS;
	return -ENOSYS;
}

}
