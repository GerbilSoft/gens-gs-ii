/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * PausedEffectTest.hpp: Paused Effect test.                               *
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

#ifndef __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP
#define __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP

#include "EffectTest.hpp"

namespace LibGens { namespace Tests {

class PausedEffectTest : public EffectTest
{
	protected:
		PausedEffectTest()
			: EffectTest() { }
		virtual ~PausedEffectTest() { }

		/**
		 * Get the base filename for the output reference images.
		 * @return Base filename, e.g. "PausedEffect".
		 */
		virtual const char *baseFilename(void) override;
};

} }

#endif /* __LIBGENS_TESTS_EFFECTS_PAUSEDEFFECTTEST_HPP */
