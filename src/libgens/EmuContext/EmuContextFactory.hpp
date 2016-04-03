/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContextFactory.hpp: EmuContext factory class.                        *
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

#ifndef __LIBGENS_EMUCONTEXT_EMUCONTEXTFACTORY_HPP__
#define __LIBGENS_EMUCONTEXT_EMUCONTEXTFACTORY_HPP__

// Region code.
// TODO: Make the region code non-console-specific.
#include "SysVersion.hpp"

namespace LibGens {

class EmuContext;
class Rom;

class EmuContextFactory
{
	private:
		// Static class.
		EmuContextFactory() { }
		~EmuContextFactory() { }

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		EmuContextFactory(const EmuContextFactory &);
		EmuContextFactory &operator=(const EmuContextFactory &);

	public:
		/**
		 * Check if a ROM's format is supported.
		 * @param rom ROM to check.
		 * @return True if supported; false if not.
		 */
		static bool isRomFormatSupported(const Rom *rom);

		/**
		 * Check if a ROM's system is supported.
		 * @param rom ROM to check.
		 * @return True if supported; false if not.
		 */
		static bool isRomSystemSupported(const Rom *rom);

		/**
		 * Create an EmuContext for the given ROM.
		 * @param rom ROM for the EmuContext.
		 * @param region Region code. (TODO: Default to REGION_AUTO.)
		 * @return EmuContext, or nullptr on error.
		 */
		static EmuContext *createContext(Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
};

}

#endif /* __LIBGENS_EMUCONTEXT_EMUCONTEXTFACTORY_HPP__ */
