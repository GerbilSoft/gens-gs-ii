/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgSave.cpp: Savestate handler. (Saving functions)                     *
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

// C includes. (C++ namespace)
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cstdlib>

namespace LibZomg
{

/**
 * Save a file to the ZOMG file.
 * @param filename Filename to save in the ZOMG file.
 * @param buf Buffer containing the file contents.
 * @param len Length of the buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveToZomg(const utf8_str *filename, const void *buf, int len)
{
	if (m_mode != ZOMG_SAVE || !m_zip)
		return -1;

	// Open the new file in the ZOMG file.
	zip_fileinfo zipfi;
	memcpy(&zipfi.tmz_date, &m_zipfi.tmz_date, sizeof(zipfi.tmz_date));
	zipfi.dosDate = 0;
	zipfi.internal_fa = 0x0000; // TODO: Set to 0x0001 for text files.
	zipfi.external_fa = 0x0000; // MS-DOS directory attribute byte.

	int ret = zipOpenNewFileInZip(
		m_zip,			// zipFile
		filename,		// Filename in the Zip archive
		&zipfi,			// File information (timestamp, attributes)
		nullptr,		// extrafield_local
		0,			// size_extrafield_local,
		nullptr,		// extrafield_global,
		0,			// size_extrafield_global,
		nullptr,		// comment
		Z_DEFLATED,		// method
		Z_DEFAULT_COMPRESSION	// level
		);

	if (ret != UNZ_OK) {
		// Error opening the new file in the Zip archive.
		// TODO: Define return codes somewhere.
		return -2;
	}

	// Write the file.
	zipWriteInFileInZip(m_zip, buf, len);	// TODO: Check the return value!
	zipCloseFileInZip(m_zip);		// TODO: Check the return value!

	// TODO: What should we return?
	return 0;
}


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
int Zomg::savePreview(const void *img_buf, size_t siz)
{
	// Verify the PNG "magic number".
	static const uint8_t png_magic[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	
	if (siz < sizeof(png_magic) ||
	    memcmp(img_buf, png_magic, sizeof(png_magic)) != 0)
	{
		// Invalid "magic number".
		return -2;
	}
	
	// Write the image buffer to the ZOMG file.
	return saveToZomg("preview.png", img_buf, siz);
}


/**
 * Templated byteswap class.
 * Use this for any memory block that requires byteswapping.
 */
namespace {
template<ZomgByteorder_t zomg_order>
class SaveMemByteswap {
	public:
		SaveMemByteswap(const void *mem, size_t siz, ZomgByteorder_t emu_order);
		~SaveMemByteswap();

	private:
		uintptr_t m_mem;	// Low bit is 1 if we allocated the memory.
		size_t m_siz;

		// put this before data() for more optimal inlining
		void *ncdata(void) const
			{ return reinterpret_cast<void*>(m_mem & ~((uintptr_t)1)); }

	public:
		const void *data(void) const
			{ return ncdata(); }

		size_t size(void) const
			{ return m_siz; }
};

template<ZomgByteorder_t zomg_order>
SaveMemByteswap<zomg_order>::SaveMemByteswap(const void *mem, size_t siz, ZomgByteorder_t emu_order)
	: m_mem(reinterpret_cast<uintptr_t>(mem))
	, m_siz(siz)
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
		{
			assert(emu_order == ZOMG_BYTEORDER_16LE || emu_order == ZOMG_BYTEORDER_16BE);
			// 16-bit data needs to be byteswapped.
			// TODO: Byteswapping memcpy().
			void *bswap_buf = malloc(siz);
			memcpy(bswap_buf, mem, siz);
			__zomg_byte_swap_16_array(bswap_buf, siz);
			m_mem = reinterpret_cast<uintptr_t>(bswap_buf) | 1;
			break;
		}

		case ZOMG_BYTEORDER_32LE:
		case ZOMG_BYTEORDER_32BE:
		{
			assert(emu_order == ZOMG_BYTEORDER_32LE || emu_order == ZOMG_BYTEORDER_32BE);
			// 32-bit data needs to be byteswapped.
			// TODO: Byteswapping memcpy().
			void *bswap_buf = malloc(siz);
			memcpy(bswap_buf, mem, siz);
			__zomg_byte_swap_32_array(bswap_buf, siz);
			m_mem = reinterpret_cast<uintptr_t>(bswap_buf) | 1;
			break;
		}

		default:
			assert(false);
	}
}

template<ZomgByteorder_t zomg_order>
SaveMemByteswap<zomg_order>::~SaveMemByteswap()
{
	if (m_mem & 1) {
		// Memory was allocated by us.
		// Free it.
		free(ncdata());
	}
}

};


/** VDP **/


/**
 * Save VDP registers.
 * File: common/vdp_reg.bin
 * @param reg Source buffer for VDP registers.
 * @param siz Number of VDP registers to load.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpReg(const uint8_t *reg, size_t siz)
{
	return saveToZomg("common/vdp_reg.bin", reg, siz);
}


/**
 * Save VDP control registers. (8-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Source buffer for VDP control registers.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpCtrl_8(const Zomg_VdpCtrl_8_t *ctrl)
{
	// Verify the header.
	if (ctrl->header != ZOMG_VDPCTRL_8_HEADER)
		return -1;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_VdpCtrl_8_t bswap_ctrl;
	memcpy(&bswap_ctrl, ctrl, sizeof(bswap_ctrl));

	// Byteswap the header.
	bswap_ctrl.header = cpu_to_be32(bswap_ctrl.header);

	// Byteswap the fields.
	bswap_ctrl.address = cpu_to_be16(bswap_ctrl.address);

	// Clear the reserved fields.
	bswap_ctrl.reserved1 = 0;
	bswap_ctrl.reserved2 = 0;

	// Save the file.
	return saveToZomg("common/vdp_ctrl.bin", &bswap_ctrl, sizeof(bswap_ctrl));
#else
	// Save the file as-is.
	return saveToZomg("common/vdp_ctrl.bin", &ctrl, sizeof(*ctrl));
#endif
}


/**
 * Save VDP control registers. (16-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Source buffer for VDP control registers.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpCtrl_16(const Zomg_VdpCtrl_16_t *ctrl)
{
	// Verify the header.
	if (ctrl->header != ZOMG_VDPCTRL_16_HEADER)
		return -1;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_VdpCtrl_16_t bswap_ctrl;
	memcpy(&bswap_ctrl, ctrl, sizeof(bswap_ctrl));

	// Byteswap the header.
	bswap_ctrl.header = cpu_to_be32(bswap_ctrl.header);

	// Byteswap the fields.
	bswap_ctrl.ctrl_word[0]	= cpu_to_be16(bswap_ctrl.ctrl_word[0]);
	bswap_ctrl.ctrl_word[1]	= cpu_to_be16(bswap_ctrl.ctrl_word[1]);
	bswap_ctrl.address	= cpu_to_be16(bswap_ctrl.address);
	bswap_ctrl.status	= cpu_to_be16(bswap_ctrl.status);

	// FIFO
	for (int i = 0; i < 4; i++)
		bswap_ctrl.data_fifo[i] = cpu_to_be16(bswap_ctrl.data_fifo[i]);

	// DMA
	bswap_ctrl.dma_src_address = cpu_to_be32(bswap_ctrl.dma_src_address);
	bswap_ctrl.dma_length = cpu_to_be16(bswap_ctrl.dma_length);

	// Clear the reserved fields.
	bswap_ctrl.reserved2 = 0;

	// Save the file.
	return saveToZomg("common/vdp_ctrl.bin", &bswap_ctrl, sizeof(bswap_ctrl));
#else
	// Save the file as-is.
	return saveToZomg("common/vdp_ctrl.bin", &ctrl, sizeof(*ctrl));
#endif
}


/**
 * Save VRam.
 * File: common/VRam.bin
 * @param vram Destination buffer for VRam.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder)
{
	// TODO: MD-only; update for other systems later.
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(vram, siz, byteorder);
	return saveToZomg("common/VRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}


/**
 * Save CRam.
 * File: common/CRam.bin
 * @param cram Destination buffer for CRam.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveCRam(const Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
{
	// TODO: MD only; GG is 16LE; SMS is 8.
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(cram, sizeof(cram->md), byteorder);
	return saveToZomg("common/CRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}


/**
 * Save VSRam. (MD-specific)
 * File: MD/VSRam.bin
 * @param vsram Destination buffer for VSRam.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
{
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(vsram, siz, byteorder);
	return saveToZomg("MD/VSRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}


/** Audio **/


/**
 * Save PSG registers.
 * File: common/psg.bin
 * 16-bit fields are always byteswapped from host-endian.
 * @param state PSG register buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::savePsgReg(const Zomg_PsgSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_BIG_ENDIAN
	Zomg_PsgSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));
	
	// Byteswap the 16-bit fields.
	for (int i = 0; i < 4; i++)
	{
		bswap_state.tone_reg[i] = cpu_to_le16(bswap_state.tone_reg[i]);
		bswap_state.tone_ctr[i] = cpu_to_le16(bswap_state.tone_ctr[i]);
	}
	bswap_state.lfsr_state = cpu_to_le16(bswap_state.lfsr_state);
	return saveToZomg("common/psg.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("common/psg.bin", state, sizeof(*state));
#endif
}


/**
 * Save YM2612 registers. (MD-specific)
 * File: MD/YM2612_reg.bin
 * @param state YM2612 register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_YM2612_reg(const Zomg_Ym2612Save_t *state)
{
	return saveToZomg("MD/YM2612_reg.bin", state, sizeof(*state));
}


/** Z80 **/


/**
 * Save Z80 memory.
 * File: common/Z80_mem.bin
 * @param mem Z80 memory buffer.
 * @param siz Size of the Z80 memory buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveZ80Mem(const uint8_t *mem, size_t siz)
{
	return saveToZomg("common/Z80_mem.bin", mem, siz);
}


/**
 * Save Z80 registers.
 * File: common/Z80_reg.bin
 * 16-bit fields are always byteswapped from host-endian.
 * @param state Z80 register buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveZ80Reg(const Zomg_Z80RegSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_BIG_ENDIAN
	Zomg_Z80RegSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));

	// Byteswap the 16-bit fields.

	// Main register set.
	bswap_state.AF = cpu_to_le16(bswap_state.AF);
	bswap_state.BC = cpu_to_le16(bswap_state.BC);
	bswap_state.DE = cpu_to_le16(bswap_state.DE);
	bswap_state.HL = cpu_to_le16(bswap_state.HL);
	bswap_state.IX = cpu_to_le16(bswap_state.IX);
	bswap_state.IY = cpu_to_le16(bswap_state.IY);
	bswap_state.PC = cpu_to_le16(bswap_state.PC);
	bswap_state.SP = cpu_to_le16(bswap_state.SP);

	// Shadow register set.
	bswap_state.AF2 = cpu_to_le16(bswap_state.AF2);
	bswap_state.BC2 = cpu_to_le16(bswap_state.BC2);
	bswap_state.DE2 = cpu_to_le16(bswap_state.DE2);
	bswap_state.HL2 = cpu_to_le16(bswap_state.HL2);

	return saveToZomg("common/Z80_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("common/Z80_reg.bin", state, sizeof(*state));
#endif
}


/** M68K (MD-specific) **/


/**
 * Save M68K memory. (MD-specific)
 * File: MD/M68K_mem.bin
 * @param mem M68K memory buffer.
 * @param siz Size of the M68K memory buffer.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveM68KMem(const uint16_t *mem, size_t siz, ZomgByteorder_t byteorder)
{
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(mem, siz, byteorder);
	return saveToZomg("MD/M68K_mem.bin", saveMemByteswap.data(), saveMemByteswap.size());
}


/**
 * Save M68K registers. (MD-specific)
 * File: MD/M68K_reg.bin
 * 16-bit and 32-bit fields are always byteswapped from host-endian.
 * @param state M68K register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveM68KReg(const Zomg_M68KRegSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_M68KRegSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));

	// Byteswap the registers.
	for (int i = 0; i < 8; i++)
		bswap_state.dreg[i] = cpu_to_be32(bswap_state.dreg[i]);
	for (int i = 0; i < 7; i++)
		bswap_state.areg[i] = cpu_to_be32(bswap_state.areg[i]);

	bswap_state.ssp = cpu_to_be32(bswap_state.ssp);
	bswap_state.usp = cpu_to_be32(bswap_state.usp);
	bswap_state.pc  = cpu_to_be32(bswap_state.pc);
	bswap_state.sr  = cpu_to_be16(bswap_state.sr);

	// Clear the reserved fields.
	bswap_state.reserved1 = 0;
	bswap_state.reserved2 = 0;

	return saveToZomg("MD/M68K_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	// TODO: Make sure the reserved fields are cleared.
	return saveToZomg("MD/M68K_reg.bin", state, sizeof(*state));
#endif
}


/** MD-specific registers. **/


/**
 * Save MD I/O port registers. (MD-specific)
 * @param state MD I/O port register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_IO(const Zomg_MD_IoSave_t *state)
{
	return saveToZomg("MD/IO.bin", state, sizeof(*state));
}


/**
 * Save MD Z80 control registers. (MD-specific)
 * 16-bit fields are always byteswapped from host-endian.
 * @param state MD Z80 control register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_Z80Ctrl(const Zomg_MD_Z80CtrlSave_t *state)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_MD_Z80CtrlSave_t bswap_state;
	memcpy(&bswap_state, state, sizeof(bswap_state));

	// Byteswap the 16-bit fields.
	bswap_state.m68k_bank = be16_to_cpu(bswap_state.m68k_bank);

	return saveToZomg("MD/Z80_ctrl.bin", &bswap_state, sizeof(bswap_state));
#else
	return saveToZomg("MD/Z80_ctrl.bin", state, sizeof(*state));
#endif
}


/**
 * Save MD /TIME registers. (MD-specific)
 * @param state MD /TIME register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_TimeReg(const Zomg_MD_TimeReg_t *state)
{
	return saveToZomg("MD/TIME_reg.bin", state, sizeof(*state));
}


/** Miscellaneous **/


/**
 * Save SRAM.
 * @param sram Pointer to SRAM buffer.
 * @param siz Size of SRAM buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveSRam(const uint8_t *sram, size_t siz)
{
	return saveToZomg("common/SRam.bin", sram, siz);
}

}
