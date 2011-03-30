/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRoms.cpp: Recent ROMs List.                                       *
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

#include "RecentRoms.hpp"

// Qt includes.
#include <QtCore/QSettings>

// Filename case-sensitivity.
// TODO: Determine this from QAbstractFileEngine instead of hard-coding it!
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
#define FILENAME_CASE_SENSITIVE Qt::CaseInsensitive
#else
#define FILENAME_CASE_SENSITIVE Qt::CaseSensitive
#endif

namespace GensQt4
{

/** Static RecentRoms members. **/
const int RecentRoms::MAX_ROMS = 9;


RecentRoms::RecentRoms(QObject *parent)
	: QObject(parent)
{ }


/**
 * update(): Update the recent ROMs list.
 * @param filename Filename of the new ROM.
 * @param z_filename Internal filename for multi-file archives.
 * @param sysId System ID.
 */
void RecentRoms::update(QString filename, QString z_filename,
			LibGens::Rom::MDP_SYSTEM_ID sysId)
{
	// Make sure sysId is in range.
	if (sysId < LibGens::Rom::MDP_SYSTEM_UNKNOWN ||
	    sysId >= LibGens::Rom::MDP_SYSTEM_MAX)
	{
		// Out of range. Assume MDP_SYSTEM_UNKNOWN.
		sysId = LibGens::Rom::MDP_SYSTEM_UNKNOWN;
	}
	
	// Check if this ROM is already in the list.
	for (int i = 0; i < m_lstRoms.size(); i++)
	{
		const RecentRom_t& rom = m_lstRoms.at(i);
		if (!filename.compare(rom.filename, FILENAME_CASE_SENSITIVE) &&
		    !z_filename.compare(rom.z_filename, FILENAME_CASE_SENSITIVE))
		{
			// ROM already exists in Recent ROMs list.
			// Update the system ID.
			if (rom.sysId != sysId)
			{
				m_lstRoms[i].sysId = sysId;
				emit updated();
			}
			return;
		}
	}
	
	// ROM doesn't exist in the Recent ROMs list.
	// Add it to the front of the list.
	RecentRom_t rom;
	rom.filename = filename;
	rom.z_filename = z_filename;
	rom.sysId = sysId;
	m_lstRoms.prepend(rom);
	
	// Make sure we don't exceed MAX_ROMS.
	while (m_lstRoms.size() > MAX_ROMS)
		m_lstRoms.removeLast();
	
	// Recent ROM list is updated.
	emit updated();
}


/**
 * load(): Load the recent ROMs list from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return Size of Recent ROMs list on success; negative on error.
 */
int RecentRoms::load(const QSettings& settings)
{
	// Clear the Recent ROMs list.
	m_lstRoms.clear();
	
	// Read the recent ROMs from the settings file.
	for (int i = 0; i < MAX_ROMS; i++)
	{
		RecentRom_t rom;
		const QString romID = QLatin1String("rom") + QString::number(i+1);
		
		// Get the ROM filename.
		// If it's empty, skip this ROM.
		rom.filename = settings.value(romID).toString();
		if (rom.filename.isEmpty())
			continue;
		
		// Get the ROM z_filename.
		// May be empty for uncompressed ROMs or single-file archives.
		rom.z_filename = settings.value(romID + QLatin1String("_zf")).toString();
		
		// Get the ROM system ID.
		rom.sysId = (LibGens::Rom::MDP_SYSTEM_ID)
				(settings.value(romID + QLatin1String("_sys"), LibGens::Rom::MDP_SYSTEM_UNKNOWN).toInt());
		
		// Append the ROM to the list.
		m_lstRoms.append(rom);
	}
	
	// Recent ROMs list has been updated.
	emit updated();
	
	// Return the size of the Recent ROMs list.
	return m_lstRoms.size();
}


/**
 * save(): Save the recent ROMs list to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int RecentRoms::save(QSettings& settings)
{
	// Save the recent ROMs to the settings file.
	int i = 0;
	foreach(const RecentRom_t& rom, m_lstRoms)
	{
		// Save the ROM information.
		const QString romID = QLatin1String("rom") + QString::number(i+1);
		settings.setValue(romID, rom.filename);
		settings.setValue(romID + QLatin1String("_zf"), rom.z_filename);
		settings.setValue(romID + QLatin1String("_sys"), (int)rom.sysId);
		
		// Increment the ROM counter.
		i++;
	}
	
	// Delete extra entries.
	for (; i < MAX_ROMS; i++)
	{
		const QString romID = QLatin1String("rom") + QString::number(i+1);
		settings.remove(romID);
		settings.remove(romID + QLatin1String("_zf"));
		settings.remove(romID + QLatin1String("_sys"));
	}
	
	// Return the size of the Recent ROMs list.
	return m_lstRoms.size();
}

}
