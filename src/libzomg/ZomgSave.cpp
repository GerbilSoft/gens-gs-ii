/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgSave.cpp: Savestate handler. (Saving functions)                     *
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

#include "Zomg.hpp"
#include "libcompat/byteswap.h"
#include "Metadata.hpp"

// MiniZip
#include "minizip/zip.h"

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
#include <cstdlib>
#include <cerrno>

// C++ includes.
#include <string>
using std::string;

// PngWriter.
#include "PngWriter.hpp"
#include "img_data.h"

// OS-dependent Zip fields.
// References:
// - https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
// - http://unix.stackexchange.com/questions/14705/the-zip-formats-external-file-attribute
// - http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
#if defined(__APPLE__)
# if defined(__MACH__)
#  define ZIP_CREATOR_SYSTEM 19			/* OS X (Darwin) */
/* TODO: ZIP_EXTERNAL_FA for Mac OS X. */
#  error Needs ZIP_EXTERNAL_FA for Mac OS X.
# else
#  define ZIP_CREATOR_SYSTEM 7			/* Macintosh */
#  define ZIP_EXTERNAL_FA 0			/* No reliable data for this... */
# endif
#elif defined(__unix) || defined(__unix__) || \
      defined(__linux) || defined(__linux__)
# define ZIP_CREATOR_SYSTEM 3			/* UNIX */
# define ZIP_EXTERNAL_FA (0100644 << 16)	/* UNIX file permissions + MS-DOS */
#else
# define ZIP_CREATOR_SYSTEM 0			/* MS-DOS, OS/2, Windows, other FAT-based systems */
# define ZIP_EXTERNAL_FA 0
#endif

#define ZIP_VERSION_MADE_BY (ZIP_CREATOR_SYSTEM << 8)

#include "Zomg_p.hpp"
namespace LibZomg {

/**
 * Save a file to the ZOMG file.
 * @param filename     [in] Filename to save in the ZOMG file.
 * @param buf          [in] Buffer containing the file contents.
 * @param len          [in] Length of the buffer.
 * @param fileType     [in] File type, e.g. binary or text.
 * @return 0 on success; non-zero on error.
 */
int ZomgPrivate::saveToZomg(const char *filename, const void *buf, int len,
			    ZomgZipFileType_t fileType)
{
	if (q->m_mode != ZomgBase::ZOMG_SAVE || !this->zip)
		return -EBADF;

	// Open the new file in the ZOMG file.
	zip_fileinfo zipfi;
	memcpy(&zipfi.tmz_date, &this->zipfi.tmz_date, sizeof(zipfi.tmz_date));
	zipfi.dosDate = 0;
	assert(fileType >= ZOMG_FILE_BINARY && fileType <= ZOMG_FILE_TEXT);
	zipfi.internal_fa = fileType;
	zipfi.external_fa = ZIP_EXTERNAL_FA;	// External attributes. (OS-dependent)

	int ret = zipOpenNewFileInZip4(
		this->zip,		// zipFile
		filename,		// Filename in the Zip archive
		&zipfi,			// File information (timestamp, attributes)
		nullptr,		// extrafield_local
		0,			// size_extrafield_local,
		nullptr,		// extrafield_global,
		0,			// size_extrafield_global,
		nullptr,		// comment
		Z_DEFLATED,		// method
		Z_DEFAULT_COMPRESSION,  // level
		// The following values, except for versionMadeBy,
		// are all defaults from zipOpenNewFileInZip().
		0,			// raw
		-MAX_WBITS,		// windowBits
		DEF_MEM_LEVEL,		// memLevel
		Z_DEFAULT_STRATEGY,	// strategy
		nullptr,		// password
		0,			// crcForCrypting
		ZIP_VERSION_MADE_BY,	// versionMadeBy
		0			// flagBase
		);

	if (ret != UNZ_OK) {
		// Error opening the new file in the Zip archive.
		return -EIO;
	}

	// Write the file.
	zipWriteInFileInZip(this->zip, buf, len);	// TODO: Check the return value!
	zipCloseFileInZip(this->zip);			// TODO: Check the return value!

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
 * Save ZOMG.ini.
 * This function MUST be called before any other function when saving!
 * @param metadata Metadata class with information about the savestate.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveZomgIni(const Metadata *metadata)
{
	// TODO: Synchronize Zip timestamp with metadata?
	// TODO: Save system ID? (once it's an enum...)
	// TODO: Return an error in other functions if metadata wasn't saved.

	string zomgIniStr = metadata->toZomgIni();
	if (zomgIniStr.empty())
		return -1;

	// Write ZOMG.ini to the ZOMG file.
	// TODO: Set a flag indicating ZOMG.ini has been saved.
	return d->saveToZomg("ZOMG.ini",
		zomgIniStr.data(), zomgIniStr.size(),
		ZomgPrivate::ZOMG_FILE_TEXT);
}

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
int Zomg::savePreview(const _Zomg_Img_Data_t *img_data)
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
int Zomg::savePreview(const Zomg_Img_Data_t *img_data,
		      const Metadata *metadata, int metaFlags)
{
	if (m_mode != ZomgBase::ZOMG_SAVE || !d->zip)
		return -EBADF;

	// Open the new file in the ZOMG file.
	zip_fileinfo zipfi;
	memcpy(&zipfi.tmz_date, &d->zipfi.tmz_date, sizeof(zipfi.tmz_date));
	zipfi.dosDate = 0;
	zipfi.internal_fa = ZomgPrivate::ZOMG_FILE_BINARY;
	zipfi.external_fa = ZIP_EXTERNAL_FA;	// External attributes. (OS-dependent)

	int ret = zipOpenNewFileInZip4(
		d->zip,			// zipFile
		"preview.png",		// Filename in the Zip archive
		&zipfi,			// File information (timestamp, attributes)
		nullptr,		// extrafield_local
		0,			// size_extrafield_local,
		nullptr,		// extrafield_global,
		0,			// size_extrafield_global,
		nullptr,		// comment
		Z_DEFLATED,		// method
		Z_DEFAULT_COMPRESSION,	// level
		// The following values, except for versionMadeBy,
		// are all defaults from zipOpenNewFileInZip().
		0,			// raw
		-MAX_WBITS,		// windowBits
		DEF_MEM_LEVEL,		// memLevel
		Z_DEFAULT_STRATEGY,	// strategy
		nullptr,		// password
		0,			// crcForCrypting
		ZIP_VERSION_MADE_BY,	// versionMadeBy
		0			// flagBase
		);

	if (ret != UNZ_OK) {
		// Error opening the new file in the Zip archive.
		return -EIO;
	}

	// Write the file.
	PngWriter pngWriter;	// TODO: Make it static?
	ret = pngWriter.writeToZip(img_data, d->zip, metadata, metaFlags);
	zipCloseFileInZip(d->zip);	// TODO: Check the return value!

	return ret;
}

namespace {

/**
 * Templated byteswap class.
 * Use this for any memory block that requires byteswapping.
 * TODO: Verify that this class is optimized out if zomg_order == emu_order.
 */
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
inline SaveMemByteswap<zomg_order>::SaveMemByteswap(const void *mem, size_t siz, ZomgByteorder_t emu_order)
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
		case ZOMG_BYTEORDER_16BE: {
			assert(emu_order == ZOMG_BYTEORDER_16LE || emu_order == ZOMG_BYTEORDER_16BE);
			// 16-bit data needs to be byteswapped.
			// TODO: Byteswapping memcpy().
			void *bswap_buf = malloc(siz);
			memcpy(bswap_buf, mem, siz);
			__byte_swap_16_array(bswap_buf, siz);
			m_mem = reinterpret_cast<uintptr_t>(bswap_buf) | 1;
			break;
		}

		case ZOMG_BYTEORDER_32LE:
		case ZOMG_BYTEORDER_32BE: {
			assert(emu_order == ZOMG_BYTEORDER_32LE || emu_order == ZOMG_BYTEORDER_32BE);
			// 32-bit data needs to be byteswapped.
			// TODO: Byteswapping memcpy().
			void *bswap_buf = malloc(siz);
			memcpy(bswap_buf, mem, siz);
			__byte_swap_32_array(bswap_buf, siz);
			m_mem = reinterpret_cast<uintptr_t>(bswap_buf) | 1;
			break;
		}

		default:
			assert(false);
	}
}

template<ZomgByteorder_t zomg_order>
inline SaveMemByteswap<zomg_order>::~SaveMemByteswap()
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
	return d->saveToZomg("common/vdp_reg.bin", reg, siz);
}

/**
 * Save VDP control registers. (8-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Source buffer for VDP control registers.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpCtrl_8(const Zomg_VDP_ctrl_8_t *ctrl)
{
	// Verify the header.
	if (ctrl->header != ZOMG_VDPCTRL_8_HEADER)
		return -1;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_VDP_ctrl_8_t bswap_ctrl;
	memcpy(&bswap_ctrl, ctrl, sizeof(bswap_ctrl));

	// Byteswap the header.
	bswap_ctrl.header = cpu_to_be32(bswap_ctrl.header);

	// Byteswap the fields.
	bswap_ctrl.address = cpu_to_be16(bswap_ctrl.address);

	// Save the file.
	return d->saveToZomg("common/vdp_ctrl.bin", &bswap_ctrl, sizeof(bswap_ctrl));
#else
	// Save the file as-is.
	return d->saveToZomg("common/vdp_ctrl.bin", &ctrl, sizeof(*ctrl));
#endif
}

/**
 * Save VDP control registers. (16-bit)
 * File: common/vdp_ctrl.bin
 * @param ctrl Source buffer for VDP control registers.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveVdpCtrl_16(const Zomg_VDP_ctrl_16_t *ctrl)
{
	// Verify the header.
	if (ctrl->header != ZOMG_VDPCTRL_16_HEADER)
		return -1;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_VDP_ctrl_16_t bswap_ctrl;
	memcpy(&bswap_ctrl, ctrl, sizeof(bswap_ctrl));

	// Byteswap the header.
	bswap_ctrl.header = cpu_to_be32(bswap_ctrl.header);

	// Byteswap the fields.
	bswap_ctrl.address	= cpu_to_be32(bswap_ctrl.address);
	bswap_ctrl.status	= cpu_to_be16(bswap_ctrl.status);
	bswap_ctrl.data_read_buffer = cpu_to_be16(bswap_ctrl.data_read_buffer);

	// Byteswap the FIFO.
	for (int i = 0; i < 4; i++) {
		bswap_ctrl.data_fifo[i] = cpu_to_be16(bswap_ctrl.data_fifo[i]);
	}

	// DMA (TODO)

	// Clear the reserved fields.
	bswap_ctrl.reserved1 = 0;
	bswap_ctrl.reserved2 = 0;

	// Save the file.
	return d->saveToZomg("common/vdp_ctrl.bin", &bswap_ctrl, sizeof(bswap_ctrl));
#else
	// Save the file as-is.
	return d->saveToZomg("common/vdp_ctrl.bin", &ctrl, sizeof(*ctrl));
#endif
}

/**
 * Save VRam.
 * File: common/VRam.bin
 * @param vram Source buffer for VRam.
 * @param siz Number of bytes to read.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveVRam(const void *vram, size_t siz, ZomgByteorder_t byteorder)
{
	// TODO: MD-only; update for other systems later.
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(vram, siz, byteorder);
	return d->saveToZomg("common/VRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}

/**
 * Save CRam.
 * File: common/CRam.bin
 * @param cram Source buffer for CRam.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveCRam(const Zomg_CRam_t *cram, ZomgByteorder_t byteorder)
{
	// TODO: MD only; GG is 16LE; SMS is 8.
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(cram, sizeof(cram->md), byteorder);
	return d->saveToZomg("common/CRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}

/**
 * Save VSRam. (MD-specific)
 * File: MD/VSRam.bin
 * @param vsram Source buffer for VSRam.
 * @param siz Number of bytes to save.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_VSRam(const uint16_t *vsram, size_t siz, ZomgByteorder_t byteorder)
{
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(vsram, siz, byteorder);
	return d->saveToZomg("MD/VSRam.bin", saveMemByteswap.data(), saveMemByteswap.size());
}

/**
 * Save the cached VDP Sprite Attribute Table. (MD-specific)
 * File: MD/vdp_sat.bin
 * @param vdp_sat Source buffer for the VDP SAT.
 * @param siz Number of bytes to save.
 * @param byteorder ZOMG byteorder to use for the memory buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Apply byteswapping only for MD.
 */
int Zomg::saveMD_VDP_SAT(const void *vdp_sat, size_t siz, ZomgByteorder_t byteorder)
{
	// TODO: MD-only; update for other systems later.
	SaveMemByteswap<ZOMG_BYTEORDER_16BE> saveMemByteswap(vdp_sat, siz, byteorder);
	return d->saveToZomg("MD/vdp_sat.bin", saveMemByteswap.data(), saveMemByteswap.size());
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
	for (int i = 0; i < 4; i++) {
		bswap_state.tone_reg[i] = cpu_to_le16(bswap_state.tone_reg[i]);
		bswap_state.tone_ctr[i] = cpu_to_le16(bswap_state.tone_ctr[i]);
	}
	bswap_state.lfsr_state = cpu_to_le16(bswap_state.lfsr_state);
	return d->saveToZomg("common/psg.bin", &bswap_state, sizeof(bswap_state));
#else
	return d->saveToZomg("common/psg.bin", state, sizeof(*state));
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
	return d->saveToZomg("MD/YM2612_reg.bin", state, sizeof(*state));
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
	return d->saveToZomg("common/Z80_mem.bin", mem, siz);
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

	// Additional internal state.
	bswap_state.WZ = cpu_to_le16(bswap_state.WZ);

	return d->saveToZomg("common/Z80_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	return d->saveToZomg("common/Z80_reg.bin", state, sizeof(*state));
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
	return d->saveToZomg("MD/M68K_mem.bin", saveMemByteswap.data(), saveMemByteswap.size());
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

	return d->saveToZomg("MD/M68K_reg.bin", &bswap_state, sizeof(bswap_state));
#else
	// TODO: Make sure the reserved fields are cleared.
	return d->saveToZomg("MD/M68K_reg.bin", state, sizeof(*state));
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
	return d->saveToZomg("MD/IO.bin", state, sizeof(*state));
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
	bswap_state.m68k_bank = cpu_to_be16(bswap_state.m68k_bank);

	return d->saveToZomg("MD/Z80_ctrl.bin", &bswap_state, sizeof(bswap_state));
#else
	return d->saveToZomg("MD/Z80_ctrl.bin", state, sizeof(*state));
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
	return d->saveToZomg("MD/TIME_reg.bin", state, sizeof(*state));
}

/**
 * Save MD TMSS registers. (MD-specific)
 * @param state MD TMSS register buffer.
 * @return 0 on success; non-zero on error.
 * TODO: Return an error if the system isn't MD.
 */
int Zomg::saveMD_TMSS_reg(const Zomg_MD_TMSS_reg_t *tmss)
{
#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_MD_TMSS_reg_t bswap_tmss;
	bswap_tmss.header = cpu_to_be32(tmss->header);
	bswap_tmss.a14000 = cpu_to_be32(tmss->a14000);
	bswap_tmss.n_cart_ce = tmss->n_cart_ce;
	return d->saveToZomg("MD/TMSS_reg.bin", &bswap_tmss, sizeof(bswap_tmss));
#else
	return d->saveToZomg("MD/TMSS_reg.bin", tmss, sizeof(*tmss));
#endif
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
	// TODO: Don't allow >65,536?
	return d->saveToZomg("common/SRam.bin", sram, siz);
}

/**
 * Save the EEPROM control data.
 * @param ctrl EEPROM control data.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveEEPRomCtrl(const Zomg_EPR_ctrl_t *ctrl)
{
	// TODO: Handle other types of EEPROM.
	if (ctrl->epr_type != ZOMG_EPR_TYPE_I2C)
		return -1;

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	Zomg_EPR_ctrl_t bswap_eeprom;
	memcpy(&bswap_eeprom, ctrl, sizeof(bswap_eeprom));

	// Byteswap the header fields.
	bswap_eeprom.header     = cpu_to_be32(bswap_eeprom.header);
	bswap_eeprom.epr_type   = cpu_to_be32(bswap_eeprom.epr_type);
 
	switch (ctrl->epr_type) {
		case ZOMG_EPR_TYPE_I2C:
			// Byteswap the I2C fields.
			bswap_eeprom.i2c.size           = cpu_to_be32(bswap_eeprom.i2c.size);
			bswap_eeprom.i2c.page_size      = cpu_to_be16(bswap_eeprom.i2c.page_size);
			bswap_eeprom.i2c.address        = cpu_to_be32(bswap_eeprom.i2c.address);
			break;

		default:
			return -1;
       }

	return d->saveToZomg("common/EPR_ctrl.bin", &bswap_eeprom, sizeof(bswap_eeprom));
#else
	return d->saveToZomg("common/EPR_ctrl.bin", ctrl, sizeof(*ctrl));
#endif
}

/**
 * Save the EEPROM page cache.
 * @param cache EEPROM page cache.
 * @param siz Size of EEPROM page cache.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveEEPRomCache(const uint8_t *cache, size_t siz)
{
	// TODO: Don't allow >256?
	return d->saveToZomg("common/EPR_cache.bin", cache, siz);
}

/**
 * Save EEPROM.
 * @param sram Pointer to EEPROM buffer.
 * @param siz Size of EEPROM buffer.
 * @return 0 on success; non-zero on error.
 */
int Zomg::saveEEPRom(const uint8_t *eeprom, size_t siz)
{
	// TODO: Don't allow >65,536?
	return d->saveToZomg("common/EEPRom.bin", eeprom, siz);
}

}
