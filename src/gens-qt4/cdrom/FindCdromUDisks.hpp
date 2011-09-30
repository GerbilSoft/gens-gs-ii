/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromUDisks.hpp: Find CD-ROM drives using UDisks.                   *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMUDISKS_HPP__
#define __GENS_QT4_CDROM_FINDCDROMUDISKS_HPP__

#include "FindCdrom.hpp"

// QtDBus includes.
#include <QtDBus/QDBusObjectPath>

// QtDBus forward declarations.
class QDBusInterface;

namespace GensQt4
{

class FindCdromUDisks : public FindCdrom
{
	Q_OBJECT
	
	public:
		FindCdromUDisks();
		~FindCdromUDisks();
		
		inline bool isUsable(void) const
			{ return (m_ifUDisks != NULL); }
		
		/**
		 * query(): Asynchronously query for CD-ROM drives.
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 */
		int query(void);
	
	protected:
		/**
		 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 */
		int query_int(void);
	
	private:
		static const char *const ms_UDisks_DriveID[20];
		
		static QString GetStringProperty(QDBusInterface *dbus_if, const char *prop);
		static bool GetBoolProperty(QDBusInterface *dbus_if, const char *prop);
		
		// D-BUS interface to UDisks.
		QDBusInterface *m_ifUDisks;
		int queryUDisksDevice(const QDBusObjectPath& objectPath);
	
	private slots:
		void deviceChanged(const QDBusObjectPath& objectPath);
		void deviceRemoved(const QDBusObjectPath& objectPath);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMUDISKS_HPP__ */
