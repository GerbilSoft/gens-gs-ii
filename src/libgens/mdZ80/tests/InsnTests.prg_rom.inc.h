/***************************************************************************
 * mdZ80/tests: Gens Z80 Emulator. (Test Suite)                            *
 * InsnTests.prg_rom.inc.h: Program ROM for instruction tests.             *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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
 * Program ROM. (Mapped to $8000)
 * All tests are followed by HALT.
 */
const uint8_t InsnTests::prg_rom[0x8000] = {
	// $8000: NOP
	0x00, 0x76,
};
