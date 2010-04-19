/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_INC_DEC.c: Increment / Decrement.                                 *
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

#ifndef __MDZ80_INC_DEC_H
#define __MDZ80_INC_DEC_H

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char mdZ80_INC_Flags_Table[256];
extern const unsigned char mdZ80_DEC_Flags_Table[256];

#ifdef __cplusplus
}
#endif

#endif /* __MDZ80_INC_DEC_H */
