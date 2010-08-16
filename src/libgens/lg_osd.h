/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * lg_osd.h: LibGens OSD callback handler.                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_LG_OSD_H__
#define __LIBGENS_LG_OSD_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	OSD_UNKNOWN = 0,
	OSD_SRAM_LOAD,		// param: Number of bytes loaded.
	OSD_SRAM_SAVE,		// param: Number of bytes saved.
	OSD_SRAM_AUTOSAVE,	// param: Number of bytes saved.
	OSD_EEPROM_LOAD,	// param: Number of bytes loaded.
	OSD_EEPROM_SAVE,	// param: Number of bytes saved.
	OSD_EEPROM_AUTOSAVE,	// param: Number of bytes saved.
	
	OSD_MAX
} OsdType;

/**
 * lg_osd_fn(): Onscreen display function.
 * @param osd_type: OSD type.
 * @param param: Integer parameter.
 * The OSD handles message formatting.
 */
typedef void (*lg_osd_fn)(OsdType osd_type, int param);

void lg_set_osd_fn(lg_osd_fn fn);
extern lg_osd_fn lg_osd;

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_LG_OSD_H__ */
