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

#include "lg_osd.h"

/**
 * lg_osd_default(): Default OSD function.
 * It does nothing!
 * @param osd_type OSD type. (ignored)
 * @param param Integer parameter. (ignored)
 */
static void lg_osd_default(OsdType osd_type, int param)
{
	// Do nothing!
	((void)osd_type);
	((void)param);
}


lg_osd_fn lg_osd = lg_osd_default;


/**
 * lg_set_osd_fn(): Set an OSD function.
 * @param fn OSD function, or NULL to disable.
 */
void lg_set_osd_fn(lg_osd_fn fn)
{
	if (!fn)
		lg_osd = lg_osd_default;
	else
		lg_osd = fn;
}
