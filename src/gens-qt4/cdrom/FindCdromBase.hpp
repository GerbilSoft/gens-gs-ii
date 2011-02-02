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
#include <QtCore/QMetaType>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QIcon>

namespace GensQt4
{

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

struct CdromDriveEntry
{
	QString path;
	
	// Drive information.
	uint32_t discs_supported;	// Bitfield indicating supported media.
	DriveType drive_type;		// Drive type. (Maximum disc supported.)
	QString drive_vendor;		// Drive vendor.
	QString drive_model;		// Drive model.
	QString drive_firmware;		// Drive firmware revision.
	
	// Disc information.
	uint32_t disc_type;		// Single bit indicating current media.
	QString disc_label;		// Disc label.
	bool disc_blank;		// True if the disc is blank.
};

class FindCdromBase : public QObject
{
	Q_OBJECT
	
	public:
		FindCdromBase()
		{
			if (!ms_MetaType_CdromDriveEntry)
				ms_MetaType_CdromDriveEntry = qRegisterMetaType<CdromDriveEntry>("CdromDriveEntry");
			
			m_thread = new FindCdromThread(this);
		}
		~FindCdromBase()
		{
			if (m_thread->isRunning())
				m_thread->terminate();
			delete m_thread;
		}
		
		virtual bool isUsable(void) const
			{ return true; }
		
		/**
		 * query(): Asynchronously query for CD-ROM drives.
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 * TODO: Determine if the backend is usable. If not, return an error code.
		 */
		virtual int query(void)
		{
			m_thread->start();
			return 0;
		}
		
		// Get drive type and disc type names.
		static QString GetDriveTypeName(DriveType drive_type);
		static QString GetDiscTypeName(uint32_t disc_type);
		
		/**
		 * getDriveIcon(): Get the icon for a given CdromDriveEntry.
		 * If a disc type is set, gets the disc icon.
		 * Otherwise, gets the drive icon.
		 * @param drive CdromDriveEntry.
		 * @return Icon for either the drive or the disc.
		 */
		virtual QIcon getDriveIcon(const CdromDriveEntry& drive)
		{
			if (drive.disc_type == 0)
				return GetDriveTypeIcon(drive.drive_type);
			else
				return GetDiscTypeIcon(drive.disc_type);
		}
	
	protected:
		// Qt metatype ID for CdromDriveEntry.
		static int ms_MetaType_CdromDriveEntry;
		
		class FindCdromThread : public QThread
		{
			public:
				FindCdromThread(FindCdromBase *base)
					{ m_cdromBase = base; }
				
				void run(void) { m_cdromBase->query_int(); }
			
			protected:
				FindCdromBase *m_cdromBase;
		};
		FindCdromThread *m_thread;
		
		/**
		 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int query_int(void) = 0;
		
		static DriveType GetDriveType(uint32_t discs_supported);
		
		// Get drive type and disc type icons.
		static QIcon GetDriveTypeIcon(DriveType drive_type);
		static QIcon GetDiscTypeIcon(uint32_t disc_type);
	
	signals:
		void driveUpdated(const CdromDriveEntry& drive);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMBASE_HPP__ */
