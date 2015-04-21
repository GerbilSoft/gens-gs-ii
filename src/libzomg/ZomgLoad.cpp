/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgLoad.cpp: Savestate handler. (Loading functions)                    *
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

#include "Zomg.hpp"
#include "zomg_byteswap.h"

// MiniZip
#include "minizip/unzip.h"

// ZOMG save structs.
#include "zomg_vdp.h"
#include "zomg_psg.h"
#include "zomg_ym2612.h"
#include "zomg_m68k.h"
#include "zomg_z80.h"
#include "zomg_md_io.h"
#include "zomg_md_z80_ctrl.h"
#include "zomg_md_time_reg.h"
#include "zomg_md_tmss_reg.h"
#include "zomg_eeprom.h"

// C includes. (C++ namespace)
#include <cstdint>
#include <cstring>
#include <cassert>

// PngReader.
#include "PngReader.hpp"
#include "img_data.h"

#include "Zomg_p.hpp"
namespace LibZomg {

/**
 * Load a file from the ZOMG file.
 * @param filename Filename to load from the ZOMG file.
 * @param buf Buffer to store the file in.
 * @param len Length of the buffer.
 * @return Length of file loaded, or negative number on error.
 */
int ZomgPrivate::loadFromZomg(const utf8_str *filename, void *buf, int len)
{
	if (q->m_mode != ZomgBase::ZOMG_LOAD || !this->unz)
		return -EBADF;

	// Locate the file in the ZOMG file.
	int ret = unzLocateFile(this->unz, filename, 2);
	if (ret != UNZ_OK) {
		// File not found.
		return -ENOENT;
	}

	// Open the current file.
	ret = unzOpenCurrentFile(this->unz);
	if (ret != UNZ_OK) {
		// Error opening the current file.
		return -EIO;
	}

	// Read the file.
	ret = unzReadCurrentFile(this->unz, buf, len);
	unzCloseCurrentFile(this->unz);	// TODO: Check the return value!

	// Return the number of bytes read.
	return ret;
}

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
 * @param img_data Image data. (Caller must free img_data->data.)
 * @return 0 on success; non-zero on error.
 */
int Zomg::loadPreview(Zomg_Img_Data_t *img_data)
{
	// Load the preview image.
	// TODO: Function to automatically allocate memory for this.
	// TODO: Improve API.
	// (Maybe use a C++ class for img_data that frees itself automatically?)
	if (m_mode != ZomgBase::ZOMG_LOAD || !d->unz)
		return -EBADF;

	// Locate the file in the ZOMG file.
	int ret = unzLocateFile(d->unz, "preview.png", 2);
	if (ret != UNZ_OK) {
		// File not found.
		return -ENOENT;
	}

	// Check the size of the current file.
	// We'll apply a hard limit of 4 MB.
	// (Screenshots shouldn't be more than 600 KB,
	// and that's assuming 320x480, 32-bit color,
	// raw bitmap format.)
	unz_file_info64 file_info;
	ret = unzGetCurrentFileInfo64(d->unz, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
	if (ret != UNZ_OK) {
		// Error getting the file information.
		return -EIO;
	}

	// Check the filesize.
	if (file_info.uncompressed_size > 4*1024*1024) {
		// File is too big.
		return -ENOMEM;
	}

	// Open the current file.
	ret = unzOpenCurrentFile(d->unz);
	if (ret != UNZ_OK) {
		// Error opening the current file.
		return -EIO;
	}

	// Allocate a memory buffer.
	int len = (int)file_info.uncompressed_size;
	uint8_t *buf = (uint8_t*)malloc(len);
	if (!buf) {
		// Error allocating memory.
		unzCloseCurrentFile(d->unz);    // TODO: Check the return value!
		return -ENOMEM;
	}

	// Read the image data.
	ret = unzReadCurrentFile(d->unz, buf, len);
	unzCloseCurrentFile(d->unz);    // TODO: Check the return value!
	if (ret != len) {
		// Error reading the image data.
		free(buf);
		return -EIO;
	}

	// Convert from PNG format to 32-bit xBGR.
	PngReader pngReader;    // TODO: Make it static?
	ret = pngReader.readFromMem(img_data, buf, len);
	free(buf);
	if (ret < 0) {
		// Error decoding the image.
		return -EIO;    // TODO: Better error code?
	}

	// Preview image loaded.
	return 0;
}

namespace {

/**
 * Templated byteswap function.
 * Use this for any memory block that requires byteswapping.
 * TODO: Verify that this function is optimized out if zomg_order == emu_order.
 */
template<ZomgByteorder_t zomg_order>
static inline void LoadMemByteswap(void *mem, size_t siz, ZomgByteorder_t emu_order)
{
	// Byteswap mem based on zomg_order.
	if (zomg_order == emu_order) {
		// Byteorder is identical.
		return;
	}
	if (emu_order == ZOMG_BYTEORDER_8) {
		// 8-bit data is never byteswapped.
		return;
	}

	switch (zomg_order) {
		case ZOMG_BYTEORDER_8:
			// 8-bit data is never byteswapped.
			break;

		case ZOMG_BYTEORDER_16LE:
		case ZOMG_BYTEORDER_16BE:
			assert(emu_order == ZOMG_BYTEORDER_16LE || emu_order == ZOMG_BYTEORDER_16BE);
			// 16-bit data needs to be byteswapped.
			__zomg_byte_swap_16_array(mem, siz);
			break;

		case ZOMG_BYTEORDER_32LE:
		case ZOMG_BYTEORDER_32BE:
			assert(emu_order == ZOMG_BYTEORDER_32LE || emu_order == ZOMG_BYTEORDER_32BE);
			// 32-bit data needs to be byteswapped.
			__zomg_byte_swap_32_array(mem, siz);
			break;

		default:
			assert(false);
	}
}

}

/** VDP **/

/**
 * Load VDP registers.
 * File: common/vdp_reg.bin
 * @param reg Destination buffer for VDP registers.
 * @param siz Number of VDP registers to load.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpReg(uint8_t *reg, size_t siz)
{
	return d->loadFromZomg("common/vdp_reg.bin", reg, siz);
}

/**
 * Load VDP control registers. (8-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Destination buffer for VDP control registers.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpCtrl_8(Zomg_VDP_ctrl_8_t *ctrl)
{
	int ret = d->loadFromZomg("common/vdp_ctrl.bin", ctrl, sizeof(*ctrl));

	// Verify that the control registers are valid.
	if (ret != (int)(sizeof(*ctrl)))
		return -1;

	// Verify the header.
	ctrl->header = be32_to_cpu(ctrl->header);
	if (ctrl->header != ZOMG_VDPCTRL_8_HEADER)
		return -2;

	// Byteswap the fields.
	ctrl->address = be16_to_cpu(ctrl->address);

	// Return the number of bytes read.
	return ret;
}

/**
 * Load VDP control registers. (16-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Destination buffer for VDP control registers.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVdpCtrl_16(Zomg_VDP_ctrl_16_t *ctrl)
{
	uint8_t data[sizeof(Zomg_VDP_ctrl_16_t)];
	int ret = d->loadFromZomg("common/vdp_ctrl.bin", &data, sizeof(data));

	if (ret == (int)sizeof(Zomg_VDP_ctrl_16_old_t)) {
		// OLD VERSION. (pre-10abf8f3)
		// DEPRECATED: Remove this once ZOMG is completed.
		fprintf(stderr, "Zomg::loadVdpCtrl_16(): %s: WARNING: Deprecated (pre-10abf8f3) 24-byte common/vdp_ctrl.bin found.\n",
			m_filename.c_str());
		Zomg_VDP_ctrl_16_old_t old_ctrl;
		memcpy(&old_ctrl, &data, sizeof(old_ctrl));

		// Verify the header.
		ctrl->header = be32_to_cpu(old_ctrl.header);
		if (ctrl->header != ZOMG_VDPCTRL_16_HEADER)
			return -2;

		// Byteswap the fields.
		ctrl->address	= (uint32_t)be16_to_cpu(old_ctrl.address);
		ctrl->status	= be16_to_cpu(old_ctrl.status);

		// Bytesswap the FIFO.
		for (int i = 0; i < 4; i++) {
			ctrl->data_fifo[i] = be16_to_cpu(old_ctrl.data_fifo[i]);
		}
		
		// addr_hi_latch isn't present in the old version.
		ctrl->addr_hi_latch = (ctrl->address & 0xC000);
		// FIXME: This wasn't present in the old version...
		ctrl->data_read_buffer = 0;
	} else if (ret == (int)sizeof(Zomg_VDP_ctrl_16_t)) {
		// New version. (10abf8f3 or later)
		memcpy(ctrl, &data, sizeof(*ctrl));

		// Verify the header.
		ctrl->header = be32_to_cpu(ctrl->header);
		if (ctrl->header != ZOMG_VDPCTRL_16_HEADER)
			return -2;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
		// Byteswapping is required.

		// Byteswap the fields.
		ctrl->address	= be32_to_cpu(ctrl->address);
		ctrl->status	= be16_to_cpu(ctrl->status);
		ctrl->data_read_buffer = be16_to_cpu(ctrl->data_read_buffer);

		// Bytesswap the FIFO.
		for (int i = 0; i < 4; i++) {
			ctrl->data_fifo[i] = be16_to_cpu(ctrl->data_fifo[i]);
		}
#endif	
	} else {
		// Invalid size.
		fprintf(stderr, "Zomg::loadVdpCtrl_16(): %s: ERROR: Invalid common/vdp_ctrl.bin found. (%d bytes)\n",
			m_filename.c_str(), ret);
		return -1;
	}

	// Return the number of bytes read.
	return ret;
}

/**
 * Load VRam.
 * File: common/VRam.bin
 * @param vram Destination buffer for VRam.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadVRam(void *vram, size_t siz, ZomgByteorder_t byteorder)
{
	int ret = d->loadFromZomg("common/VRam.bin", vram, siz);
	// TODO: MD only; Sega 8-bit systems use ZOMG_BYTEORDER_8.
	// TODO: Clear the rest of vram if ret < siz.
	LoadMemByteswap<ZOMG_BYTEORDER_16BE>(vram, siz, byteorder);
	return ret;
}

/**
 * Load CRam.
 * File: common/CRam.bin
 * @param cram Destination buffer for CRam.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::loadCRam(Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
{
	int ret = d->loadFromZomg("common/CRam.bin", cram->md, sizeof(cram->md));
	// TODO: MD only; GG is 16LE; SMS is 8.
	// TODO: Clear the rest of cram if ret < sizeof(cram->md).
	LoadMemByteswap<ZOMG_BYTEORDER_16BE>(cram, sizeof(cram->md), byteorder);
	return ret;
}

/**
 * Load VSRam. (MD-specific)
 * File: MD/VSRam.bin
 * @param vsram Destination buffer for VSRam.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_VSRam(uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
{
	int ret = d->loadFromZomg("MD/VSRam.bin", vsram, siz);
	// TODO: Clear the rest of vsram if ret < siz.
	LoadMemByteswap<ZOMG_BYTEORDER_16BE>(vsram, siz, byteorder);
	return ret;
}

/**
 * Load the cached VDP Sprite Attribute Table. (MD-specific)
 * File: MD/vdp_sat.bin
 * @param vdp_sat Destination buffer for the VDP SAT.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_VDP_SAT(uint16_t *vdp_sat, size_t siz, ZomgByteorder_t byteorder)
{
	int ret = d->loadFromZomg("MD/vdp_sat.bin", vdp_sat, siz);
	// TODO: Clear the rest of the vdp_sat if ret < siz.
	LoadMemByteswap<ZOMG_BYTEORDER_16BE>(vdp_sat, siz, byteorder);
	return ret;
}

/** Audio **/

/**
 * Load PSG registers.
 * File: common/psg.bin
 * 16-bit fields are always byteswapped to host-endian.
 * @param state PSG register buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadPsgReg(Zomg_PsgSave_t *state)
{
	int ret = d->loadFromZomg("common/psg.bin", state, sizeof(*state));

	// Byteswap the 16-bit fields.
	for (int i = 0; i < 4; i++) {
		state->tone_reg[i] = le16_to_cpu(state->tone_reg[i]);
		state->tone_ctr[i] = le16_to_cpu(state->tone_ctr[i]);
	}
	state->lfsr_state = le16_to_cpu(state->lfsr_state);

	return ret;
}

/**
 * Load YM2612 registers. (MD-specific)
 * File: MD/YM2612_reg.bin
 * @param state YM2612 register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_YM2612_reg(Zomg_Ym2612Save_t *state)
{
	return d->loadFromZomg("MD/YM2612_reg.bin", state, sizeof(*state));
}

/** Z80 **/

/**
 * Load Z80 memory.
 * File: common/Z80_mem.bin
 * @param mem Z80 memory buffer.
 * @param siz Size of the Z80 memory buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadZ80Mem(uint8_t *mem, size_t siz)
{
	return d->loadFromZomg("common/Z80_mem.bin", mem, siz);
}

/**
 * Load Z80 registers.
 * File: common/Z80_reg.bin
 * 16-bit fields are always byteswapped to host-endian.
 * @param state Z80 register buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadZ80Reg(Zomg_Z80RegSave_t *state)
{
	int ret = d->loadFromZomg("common/Z80_reg.bin", state, sizeof(*state));

	// Byteswap the 16-bit fields.

	// Main register set.
	state->AF = le16_to_cpu(state->AF);
	state->BC = le16_to_cpu(state->BC);
	state->DE = le16_to_cpu(state->DE);
	state->HL = le16_to_cpu(state->HL);
	state->IX = le16_to_cpu(state->IX);
	state->IY = le16_to_cpu(state->IY);
	state->PC = le16_to_cpu(state->PC);
	state->SP = le16_to_cpu(state->SP);

	// Shadow register set.
	state->AF2 = le16_to_cpu(state->AF2);
	state->BC2 = le16_to_cpu(state->BC2);
	state->DE2 = le16_to_cpu(state->DE2);
	state->HL2 = le16_to_cpu(state->HL2);

	return ret;
}

/** M68K (MD-specific) **/

/**
 * Load M68K memory. (MD-specific)
 * File: MD/M68K_mem.bin
 * @param mem M68K memory buffer.
 * @param siz Size of the M68K memory buffer.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadM68KMem(uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
{
	int ret = d->loadFromZomg("MD/M68K_mem.bin", mem, siz);
	// TODO: Clear the rest of mem if ret < siz.
	LoadMemByteswap<ZOMG_BYTEORDER_16BE>(mem, siz, byteorder);
	return ret;
}

/**
 * Load M68K registers. (MD-specific)
 * File: MD/M68K_reg.bin
 * 16-bit and 32-bit fields are always byteswapped to host-endian.
 * @param state M68K register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadM68KReg(Zomg_M68KRegSave_t *state)
{
	uint8_t data[sizeof(Zomg_M68KRegSave_t)];
	int ret = d->loadFromZomg("MD/M68K_reg.bin", &data, sizeof(data));

	if (ret == (int)sizeof(Zomg_M68KRegSave_Old_t)) {
		// OLD VERSION. (pre-c43510fe)
		// DEPRECATED: Remove this once ZOMG is completed.
		fprintf(stderr, "Zomg::loadM68KReg(): %s: WARNING: Deprecated (pre-c43510fe) 74-byte MD/M68K_reg.bin found.\n",
			m_filename.c_str());
		Zomg_M68KRegSave_Old_t old_state;
		memcpy(&old_state, &data, sizeof(old_state));

		// Byteswap the 32-bit fields.
		for (int i = 0; i < 8; i++)
			state->dreg[i] = be32_to_cpu(old_state.dreg[i]);
		for (int i = 0; i < 7; i++)
			state->areg[i] = be32_to_cpu(old_state.areg[i]);

		// Byteswap the program counter.
		state->pc = be32_to_cpu(old_state.pc);

		// Byteswap the status register.
		state->sr = be16_to_cpu(old_state.sr);

		// Byteswap the stack pointers.
		if (state->sr & 0x2000) {
			// Supervisor mode.
			// old_state->areg[7] == ssp
			// old_state->asp     == usp
			state->ssp = be32_to_cpu(old_state.areg[7]);
			state->usp = be32_to_cpu(old_state.asp);
		} else {
			// User mode.
			// old_state->areg[7] == usp
			// old_state->asp     == ssp
			state->ssp = be32_to_cpu(old_state.asp);
			state->usp = be32_to_cpu(old_state.areg[7]);
		}
	} else if (ret >= (int)sizeof(Zomg_M68KRegSave_t)) {
		// New version. (c43510fe or later)
		memcpy(state, &data, sizeof(*state));

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
		// Byteswapping is required.

		// Byteswap the main registers.
		for (int i = 0; i < 8; i++)
			state->dreg[i] = be32_to_cpu(state->dreg[i]);
		for (int i = 0; i < 7; i++)
			state->areg[i] = be32_to_cpu(state->areg[i]);

		// Byteswap the other registers.
		state->ssp = be32_to_cpu(state->ssp);
		state->usp = be32_to_cpu(state->usp);
		state->pc  = be32_to_cpu(state->pc);
		state->sr  = be16_to_cpu(state->sr);
#endif	
	} else {
		// Invalid size.
		fprintf(stderr, "Zomg::loadM68KReg(): %s: ERROR: Invalid MD/M68K_reg.bin found. (%d bytes)\n",
			m_filename.c_str(), ret);
		return -1;
	}

	// Clear the reserved fields.
	state->reserved1 = 0;
	state->reserved2 = 0;

	return ret;
}

/** MD-specific registers. **/

/**
 * Load MD I/O port registers. (MD-specific)
 * @param state MD I/O port register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_IO(Zomg_MD_IoSave_t *state)
{
	return d->loadFromZomg("MD/IO.bin", state, sizeof(*state));
}

/**
 * Load MD Z80 control registers. (MD-specific)
 * 16-bit fields are always byteswapped to host-endian.
 * @param state MD Z80 control register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_Z80Ctrl(Zomg_MD_Z80CtrlSave_t *state)
{
	int ret = d->loadFromZomg("MD/Z80_ctrl.bin", state, sizeof(*state));

	// Byteswap the 16-bit fields.
	state->m68k_bank = be16_to_cpu(state->m68k_bank);

	return ret;
}

/**
 * Load MD /TIME registers. (MD-specific)
 * @param state MD /TIME register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_TimeReg(Zomg_MD_TimeReg_t *state)
{
	memset(state, 0xFF, sizeof(*state));
	int ret = d->loadFromZomg("MD/TIME_reg.bin", state, sizeof(*state));

	if (ret <= 0xF1) {
		// SRAM control register wasn't loaded.
		// Set it to 2 by default:
		// - ROM accessible
		// - SRAM write-protected
		state->SRAM_ctrl = 2;
	}

	return ret;
}

/**
 * Load MD TMSS registers. (MD-specific)
 * @param state MD TMSS register buffer.
 * @return Number of bytes read on success; negative on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::loadMD_TMSS_reg(Zomg_MD_TMSS_reg_t *tmss)
{
	int ret = d->loadFromZomg("MD/TMSS_reg.bin", tmss, sizeof(*tmss));

	// Byteswap the 32-bit fields.
	// TODO: Verify tmss->header?
	tmss->header = be32_to_cpu(tmss->header);
	tmss->a14000 = be32_to_cpu(tmss->header);

	return ret;
}

/** Miscellaneous **/

/**
 * Load SRAM.
 * @param sram Pointer to SRAM buffer.
 * @param siz Size of SRAM buffer.
 * @return Number of bytes read on success; negative on error.
 * NOTE: If the loaded SRAM file is smaller than the specified SRAM buffer,
 * the remainder of the SRAM buffer is initialized to 0xFF.
 */
int Zomg::loadSRam(uint8_t *sram, size_t siz)
{
	int ret = d->loadFromZomg("common/SRam.bin", sram, siz);
	if (ret > 0) {
		// Data was loaded.
		// If the data is less than the size of the SRAM buffer,
		// set the rest of the SRAM buffer to 0xFF.
		if (ret < (int)siz) {
			int diff = ((int)siz - ret);
			memset(&sram[ret], 0xFF, diff);
		}
	}

	return ret;
}

/**
 * Load the EEPROM control data.
 * @param ctrl EEPROM control data.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadEEPRomCtrl(Zomg_EPR_ctrl_t *ctrl)
{
	// TODO
	return 0;
}

/**
 * Load the EEPROM page cache.
 * @param cache Pointer to EEPROM page cache buffer.
 * @param siz Size of EEPROM page cache buffer.
 * @return Number of bytes read on success; negative on error.
 */
int Zomg::loadEEPRomCache(uint8_t *cache, size_t siz)
{
	int ret = d->loadFromZomg("common/EPR_cache.bin", cache, siz);
	if (ret != (int)siz)
		return -1;
	return ret;
}

/**
 * Load EEPROM.
 * @param eeprom Pointer to SRAM buffer.
 * @param siz Size of SRAM buffer.
 * @return Number of bytes read on success; negative on error.
 * NOTE: If the loaded EEPROM file is smaller than the specified EEPROM buffer,
 * the remainder of the EEPROM buffer is initialized to 0xFF.
 */
int Zomg::loadEEPRom(uint8_t *eeprom, size_t siz)
{
	// TODO
	return 0;
}

}
