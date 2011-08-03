/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_datatypes.h: Various datatypes.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __GENS_QT4_GQT4_DATATYPES_H__
#define __GENS_QT4_GQT4_DATATYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _paused_t
{
	union
	{
		struct
		{
			uint8_t paused_manual : 1;
			uint8_t paused_auto : 1;
		};
		uint8_t data;
	};
} paused_t;

typedef enum
{
	STRETCH_NONE	= 0,
	STRETCH_H	= 1,
	STRETCH_V	= 2,
	STRETCH_FULL	= 3
} StretchMode_t;

#ifdef __cplusplus
}
#endif

#endif /* __GENS_QT4_GQT4_DATATYPES_H__ */
