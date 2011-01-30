/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdrom.hpp: Find CD-ROM drives. (Base Class)                         *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMBASE_HPP__
#define __GENS_QT4_CDROM_FINDCDROMBASE_HPP__

#include <config.h>

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QIcon>

namespace GensQt4
{

class FindCdromBase : public QObject
{
	Q_OBJECT
	
	public:
		virtual void query(void) = 0;
		
		enum DiscType
		{
			DISC_TYPE_NONE			= 0,
			DISC_TYPE_CDROM			= (1 << 0),
			DISC_TYPE_CD_R			= (1 << 1),
			DISC_TYPE_CD_RW			= (1 << 2),
			DISC_TYPE_DVD			= (1 << 3),
			DISC_TYPE_DVD_R			= (1 << 4),
			DISC_TYPE_DVD_RW		= (1 << 5),
			DISC_TYPE_DVD_RAM		= (1 << 6),
			DISC_TYPE_DVD_PLUS_R		= (1 << 7),
			DISC_TYPE_DVD_PLUS_RW		= (1 << 8),
			DISC_TYPE_DVD_PLUS_R_DL		= (1 << 9),
			DISC_TYPE_DVD_PLUS_RW_DL	= (1 << 10),
			DISC_TYPE_BDROM			= (1 << 11),
			DISC_TYPE_BD_R			= (1 << 12),
			DISC_TYPE_BD_RE			= (1 << 13),
			DISC_TYPE_HDDVD			= (1 << 14),
			DISC_TYPE_HDDVD_R		= (1 << 15),
			DISC_TYPE_HDDVD_RW		= (1 << 16),
			DISC_TYPE_MO			= (1 << 17),
			DISC_TYPE_MRW			= (1 << 18),
			DISC_TYPE_MRW_W			= (1 << 19),
		};
		
		enum DriveType
		{
			DRIVE_TYPE_NONE,
			DRIVE_TYPE_CDROM,
			DRIVE_TYPE_CD_R,
			DRIVE_TYPE_CD_RW,
			DRIVE_TYPE_DVD,
			DRIVE_TYPE_DVD_CD_RW,
			DRIVE_TYPE_DVD_R,
			DRIVE_TYPE_DVD_RW,
			DRIVE_TYPE_DVD_RAM,
			DRIVE_TYPE_DVD_PLUS_RW,
			DRIVE_TYPE_DVD_PLUS_R_DL,
			DRIVE_TYPE_DVD_PLUS_RW_DL,
			DRIVE_TYPE_BDROM,
			DRIVE_TYPE_BDROM_DVD_RW,
			DRIVE_TYPE_BD_R,
			DRIVE_TYPE_BD_RE,
			DRIVE_TYPE_HDDVD,
			DRIVE_TYPE_HDDVD_DVD_RW,
			DRIVE_TYPE_HDDVD_R,
			DRIVE_TYPE_HDDVD_RW
		};
		
		// Get drive type and disc type names.
		static QString GetDriveTypeName(DriveType drive_type);
		static QString GetDiscTypeName(uint32_t disc_type);
		
		struct drive_entry_t
		{
			QString path;
			
			// Device icon.
			// Usually has the drive icon if no disc is detected,
			// or the disc icon if a disc was found.
			QIcon icon;
			
			// Drive information.
			uint32_t discs_supported;	// Bitfield indicating supported media.
			DriveType drive_type;		// Drive type. (Maximum disc supported.)
			QString drive_vendor;		// Drive vendor.
			QString drive_model;		// Drive model.
			QString drive_firmware;		// Drive firmware revision.
			
			// Disc information.
			uint32_t disc_type;		// Single bit indicating current media.
			QString disc_label;		// Disc label.
		};
		
		const QList<drive_entry_t>& getDriveList(void) { return m_drives; }
	
	protected:
		static DriveType GetDriveType(uint32_t discs_supported);
		QList<drive_entry_t> m_drives;
		
		// Get drive type and disc type icons.
		static QIcon GetDriveTypeIcon(DriveType drive_type);
		static QIcon GetDiscTypeIcon(uint32_t disc_type);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMBASE_HPP__ */
