/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * TmssReg.cpp: MD TMSS registers.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
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

#include "TmssReg.hpp"

#include <libgens/config.libgens.h>

#include "EmuContext.hpp"
#include "Rom.hpp"

// C++ includes.
#include <memory>
#include <string>
using std::auto_ptr;
using std::string;

#include "Util/byteswap.h"

// TODO: Move into byteswap.h.
/**
 * BYTE_ADDR_INVERT: Inversion flag for byte-addressing.
 * ROM and RAM is stored in host 16-bit endian.
 * Hence, bytewide access needs to have the LSB inverted
 * on little-endian machines.
 */
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define BYTE_ADDR_INVERT 1
#else /* GENS_BYTEORDER = GENS_BIG_ENDIAN */
#define BYTE_ADDR_INVERT 0
#endif

namespace LibGens {

TmssReg::TmssReg()
	: n_cart_ce(0)
	, m_tmssRom(nullptr)
	, m_tmssRom_size_real(0)
	, m_tmssRom_size(0)
	, m_tmssRom_mask(0)
{
	a14000.d = 0;
}

TmssReg::~TmssReg()
{
	delete m_tmssRom;
}

/**
 * Load the TMSS ROM and enable TMSS.
 * M68K memory map must be updated afterwards.
 * @return 0 on success; non-zero on error.
 */
int TmssReg::loadTmssRom(void)
{
	// Assume TMSS is disabled by default.
	clearTmssRom();

	// Check if TMSS is enabled.
	// TODO: Make non-static and move somewhere else?
	if (!EmuContext::TmssEnabled()) {
		// TMSS is disabled.
		return -1;
	}

	// TMSS is enabled.
	string tmssFilename = EmuContext::TmssRomFilename();
	if (tmssFilename.empty()) {
		// TMSS ROM filename is empty.
		return -2;
	}

	// Attempt to load the ROM.
	auto_ptr<LibGens::Rom> rom(new LibGens::Rom(tmssFilename.c_str()));
	if (!rom->isOpen() || rom->isMultiFile()) {
		// Invalid ROM.
		return -3;
	}

	// Check the TMSS ROM filesize.
	// Minimum size is 4 KB.
	// Maximum size is 512 KB.
	static const uint32_t minTmssRomSize = 4096;
	static const uint32_t maxTmssRomSize = 524288;
	if (rom->romSize() < 0 || rom->romSize() > (int)maxTmssRomSize) {
		// ROM is too big.
		// Or, ROM size is invalid...
		return -4;
	}

	// Allocate the buffer.
	m_tmssRom_size_real = rom->romSize();
	m_tmssRom_size = next_pow2u(m_tmssRom_size_real);
	if (m_tmssRom_size < minTmssRomSize)
		m_tmssRom_size = minTmssRomSize;
	m_tmssRom_mask = m_tmssRom_size - 1;
	m_tmssRom = (uint8_t*)malloc(m_tmssRom_size);
	if (!m_tmssRom) {
		// Error allocating memory.
		m_tmssRom_size_real = 0;
		m_tmssRom_size = 0;
		m_tmssRom_mask = 0;
		return -5;
	}

	// Load the ROM.
	int ret = rom->loadRom(m_tmssRom, m_tmssRom_size);
	if (ret != (int)m_tmssRom_size_real) {
		// Error loading the ROM.
		delete m_tmssRom;
		m_tmssRom = nullptr;
		m_tmssRom_size_real = 0;
		m_tmssRom_size = 0;
		m_tmssRom_mask = 0;
		return -6;
	}

	// Clear the empty part of the ROM buffer.
	if (m_tmssRom_size_real < m_tmssRom_size) {
		const uint32_t diff = m_tmssRom_size - m_tmssRom_size_real;
		memset(&m_tmssRom[m_tmssRom_size_real], 0xFF, diff);
	}

	// Byteswap the ROM image.
	// FIXME: Adjust ROM size if it's odd.
	// TODO: Get rid of the pointer cast?
	be16_to_cpu_array((uint16_t*)m_tmssRom, m_tmssRom_size_real);

	// TMSS ROM is loaded.
	return 0;
}

/**
 * Clear the TMSS ROM and disable TMSS.
 * M68K memory map must be updated afterwards.
 */
void TmssReg::clearTmssRom(void)
{
	delete m_tmssRom;
	m_tmssRom = nullptr;
	m_tmssRom_size = 0;
	m_tmssRom_mask = 0;
}

/**
 * Read a byte from the TMSS ROM.
 * @param address Address.
 * @return Byte from the TMSS ROM.
 */
uint8_t TmssReg::readByte(uint32_t address) const
{
	if (!m_tmssRom)
		return 0xFF;
	address &= m_tmssRom_mask;
	return m_tmssRom[address ^ BYTE_ADDR_INVERT];
}

/**
 * Read a word from the TMSS ROM.
 * @param address Address.
 * @return Word from the TMSS ROM.
 */
uint16_t TmssReg::readWord(uint32_t address) const
{
	if (!m_tmssRom)
		return 0xFFFF;
	address &= m_tmssRom_mask;
	// TODO: Eliminate this cast?
	return ((uint16_t*)m_tmssRom)[address>>1];
}

/**
 * Update M68K CPU program access structs for bankswitching purposes.
 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
 * @param banks Maximum number of banks to update.
 * @return Number of banks updated.
 */
int TmssReg::updateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks)
{
	((void)banks);	// unused
#ifdef GENS_ENABLE_EMULATION
	M68K_Fetch->lowaddr = 0x000000;
	M68K_Fetch->highaddr = m_tmssRom_mask;
	M68K_Fetch->offset = (uint32_t)m_tmssRom;
	M68K_Fetch++;
	return 1;
#else /* !GENS_ENABLE_EMULATION */
	// Emulation is disabled.
	return 0;
#endif /* GENS_ENABLE_EMULATION */
}

}
