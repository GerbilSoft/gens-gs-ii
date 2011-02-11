/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromUnix.hpp: Find CD-ROM drives. (UNIX fallback)                  *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMUNIX_HPP__
#define __GENS_QT4_CDROM_FINDCDROMUNIX_HPP__

#include "FindCdrom.hpp"

namespace GensQt4
{

class FindCdromUnix : public FindCdrom
{
	Q_OBJECT
	
	protected:
		/**
		 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 */
		int query_int(void);
	
	private:
		static const char *ms_Unix_DevNames[];
		
		/** OS-specific functions. **/
		static int os_GetDevIdentity(int fd, CdromDriveEntry &entry);
		static DriveType os_GetDriveType(int fd);
		static uint32_t os_GetDiscType(int fd);
		static QString os_GetDiscLabel(int fd);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMUNIX_HPP__ */
