/***************************************************************************
 * libcompat/tests: Compatibility Library. (Test Suite)                    *
 * ByteswapTest.hpp: Byteswapping tests. (Common header)                   *
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

#ifndef __LIBCOMPAT_TESTS_BYTESWAPTEST_HPP__
#define __LIBCOMPAT_TESTS_BYTESWAPTEST_HPP__

// C includes.
#include <stdint.h>

namespace LibCompat { namespace Tests {

struct ByteswapTest_flags {
	uint32_t cpuFlags;
	uint32_t cpuFlags_slow;

	ByteswapTest_flags(uint32_t cpuFlags, uint32_t cpuFlags_slow)
	{
		this->cpuFlags = cpuFlags;
		this->cpuFlags_slow = cpuFlags_slow;
	}
};

} }

#endif /* __LIBCOMPAT_TESTS_BYTESWAPTEST_HPP__ */
