/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContextFactory.cpp: EmuContext factory class.                        *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "EmuContextFactory.hpp"

#include "EmuContext.hpp"
#include "Rom.hpp"

// System-specific EmuContext classes.
#include "EmuMD.hpp"
#include "EmuPico.hpp"

namespace LibGens {

/**
 * Check if a ROM's format is supported.
 * @param rom ROM to check.
 * @return True if supported; false if not.
 */
bool EmuContextFactory::isRomFormatSupported(const Rom *rom)
{
	switch (rom->romFormat()) {
		case Rom::RFMT_BINARY:
		case Rom::RFMT_SMD:
		case Rom::RFMT_SMD_SPLIT:
			// NOTE: Split SMD isn't fully supported.
			// Only the first segment will be loaded.
			return true;

		default:
			break;
	}

	// ROM format is not supported.
	return false;
}

/**
 * Check if a ROM's system is supported.
 * @param rom ROM to check.
 * @return True if supported; false if not.
 */
bool EmuContextFactory::isRomSystemSupported(const Rom *rom)
{
	switch (rom->sysId()) {
		case Rom::MDP_SYSTEM_MD:
		case Rom::MDP_SYSTEM_PICO:
			return true;

		default:
			break;
	}

	// System is not supported.
	return false;
}

/**
 * Create an EmuContext for the given ROM.
 * @param rom ROM for the EmuContext.
 * @return EmuContext, or nullptr on error.
 */
EmuContext *EmuContextFactory::createContext(Rom *rom)
{
	switch (rom->sysId()) {
		case Rom::MDP_SYSTEM_MD:
			return new EmuMD(rom);
		case Rom::MDP_SYSTEM_PICO:
			return new EmuPico(rom);

		default:
			break;
	}

	// System is not supported.
	return false;
}

}
