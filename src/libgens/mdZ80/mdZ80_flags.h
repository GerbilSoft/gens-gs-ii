/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_flags.h: Z80 flag constants.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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


#ifndef _MDZ80_FLAGS_H
#define _MDZ80_FLAGS_H


/* Z80 flags. */
#define Z80_FLAG_C	(1 << 0)
#define Z80_FLAG_N	(1 << 1)
#define Z80_FLAG_P	(1 << 2)
#define Z80_FLAG_X	(1 << 3)
#define Z80_FLAG_H	(1 << 4)
#define Z80_FLAG_Y	(1 << 5)
#define Z80_FLAG_Z	(1 << 6)
#define Z80_FLAG_S	(1 << 7)


/* Z80 states. */
#define Z80_STATE_RUNNING	0x01
#define Z80_STATE_HALTED	0x02
#define Z80_STATE_FAULTED	0x10


#endif /* _MDZ80_FLAGS_H */
