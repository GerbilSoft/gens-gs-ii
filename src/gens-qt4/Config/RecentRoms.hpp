/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRoms.hpp: Recent ROMs List.                                       *
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

#ifndef __GENS_QT4_CONFIG_RECENTROMS_HPP__
#define __GENS_QT4_CONFIG_RECENTROMS_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
#include <QtCore/QList>
class QSettings;

// LibGens includes.
#include "libgens/Rom.hpp"

namespace GensQt4
{

struct RecentRom_t
{
	QString filename;	// ROM filename.
	QString z_filename;	// Internal filename for multi-file archives.
	LibGens::Rom::MDP_SYSTEM_ID sysId;	// System ID.
};

class RecentRoms : public QObject
{
	Q_OBJECT
	
	public:
		RecentRoms(QObject *parent = 0);
		
		// Maximum number of ROMs allowed in Recent ROMs.
		static const int MAX_ROMS;
		
		/**
		 * update(): Update the recent ROMs list.
		 * @param filename Filename of the new ROM.
		 * @param z_filename Internal filename for multi-file archives.
		 * @param sysId System ID.
		 */
		void update(QString filename, QString z_filename,
				LibGens::Rom::MDP_SYSTEM_ID sysId);
		
		/**
		 * load(): Load the recent ROMs list from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param settings Settings file.
		 * @return Size of Recent ROMs list on success; negative on error.
		 */
		int load(const QSettings& settings);
		
		/**
		 * save(): Save the recent ROMs list to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param settings Settings file.
		 * @return Size of Recent ROMs list on success; negative on error.
		 */
		int save(QSettings& settings);
	
	signals:
		// Recent ROMs list has been updated.
		void updated(void);
	
	protected:
		QList<RecentRom_t> m_lstRoms;
	
	private:
		Q_DISABLE_COPY(RecentRoms)
};

}

#endif /* __GENS_QT4_CONFIG_RECENTROMS_HPP__ */
