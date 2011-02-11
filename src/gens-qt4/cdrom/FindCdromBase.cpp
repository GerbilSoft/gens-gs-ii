/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdrom.cpp: Find CD-ROM drives. (Base Class)                         *
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

#include "FindCdromBase.hpp"
#include "../GensQApplication.hpp"

namespace GensQt4
{

// Qt metatype ID for CdromDriveEntry.
int FindCdromBase::ms_MetaType_CdromDriveEntry = 0;

FindCdromBase::FindCdromBase()
{
	if (!ms_MetaType_CdromDriveEntry)
		ms_MetaType_CdromDriveEntry = qRegisterMetaType<CdromDriveEntry>("CdromDriveEntry");
	
	m_thread = new FindCdromThread(this);
}

FindCdromBase::~FindCdromBase()
{
	if (m_thread->isRunning())
		m_thread->terminate();
	
	delete m_thread;
}


/**
 * query(): Asynchronously query for CD-ROM drives.
 * The driveUpdated() signal will be emitted once for each detected drive.
 * @return 0 on success; non-zero on error.
 */
int FindCdromBase::query(void)
{
	if (!isUsable())
		return -1;
	
	m_thread->start();
	return 0;
}


/**
 * GetDriveType(): Get the drive name from a given drive specifier.
 * @param discs_supported Supported discs.
 * @return DriveType.
 */
DriveType FindCdromBase::GetDriveType(uint32_t discs_supported)
{
	// TODO: Find various permutations like DVD/CD-RW.
	// Also, check for multi-format DVD±RW drives.
	// For now, just get the maximum disc type.
	if (discs_supported & DISC_TYPE_MO)
		return DRIVE_TYPE_MO;
	else if (discs_supported & DISC_TYPE_HDDVD_RW)
		return DRIVE_TYPE_HDDVD_RW;
	else if (discs_supported & DISC_TYPE_HDDVD_R)
		return DRIVE_TYPE_HDDVD_R;
	else if (discs_supported & DISC_TYPE_HDDVD)
	{
		// TODO: Check for CD/DVD writing capabilities.
		return DRIVE_TYPE_HDDVD;
	}
	else if (discs_supported & DISC_TYPE_BD_RE)
		return DRIVE_TYPE_BD_RE;
	else if (discs_supported & DISC_TYPE_BD_R)
		return DRIVE_TYPE_BD_R;
	else if (discs_supported & DISC_TYPE_BDROM)
	{
		// TODO: Check for CD/DVD writing capabilities.
		return DRIVE_TYPE_BDROM;
	}
	else if (discs_supported & DISC_TYPE_DVD_PLUS_RW_DL)
		return DRIVE_TYPE_DVD_PLUS_RW_DL;
	else if (discs_supported & DISC_TYPE_DVD_PLUS_R_DL)
		return DRIVE_TYPE_DVD_PLUS_R_DL;
	else if (discs_supported & (DISC_TYPE_DVD_PLUS_RW | DISC_TYPE_DVD_PLUS_R))
	{
		// DVD+RW was released before DVD+R.
		// Hence, there's no such thing as a DVD+R-only drive.
		return DRIVE_TYPE_DVD_PLUS_RW;
	}
	else if (discs_supported & DISC_TYPE_DVD_RAM)
		return DRIVE_TYPE_DVD_RAM;
	else if (discs_supported & DISC_TYPE_DVD_RW)
		return DRIVE_TYPE_DVD_RW;
	else if (discs_supported & DISC_TYPE_DVD_R)
		return DRIVE_TYPE_DVD_R;
	else if (discs_supported & DISC_TYPE_DVD)
	{
		if (discs_supported & (DISC_TYPE_CD_R | DISC_TYPE_CD_RW))
			return DRIVE_TYPE_DVD_CD_RW;
		else
			return DRIVE_TYPE_DVD;
	}
	else if (discs_supported & DISC_TYPE_CD_RW)
		return DRIVE_TYPE_CD_RW;
	else if (discs_supported & DISC_TYPE_CD_R)
		return DRIVE_TYPE_CD_R;
	else if (discs_supported & DISC_TYPE_CDROM)
		return DRIVE_TYPE_CDROM;
	else
		return DRIVE_TYPE_NONE;
}


/**
 * GetDriveTypeName(): Get the name of a given drive type.
 * @param drive_type Drive type.
 * @return Drive type name, or empty string if the drive type is DRIVE_TYPE_NONE or invalid.
 */
QString FindCdromBase::GetDriveTypeName(DriveType drive_type)
{
	switch (drive_type)
	{
		case DRIVE_TYPE_NONE:
		default:			return QString();
		case DRIVE_TYPE_CDROM:		return QLatin1String("CD-ROM");
		case DRIVE_TYPE_CD_R:		return QLatin1String("CD-R");
		case DRIVE_TYPE_CD_RW:		return QLatin1String("CD-RW");
		case DRIVE_TYPE_DVD:		return QLatin1String("DVD-ROM");
		case DRIVE_TYPE_DVD_CD_RW:	return QLatin1String("DVD/CD-RW");
		case DRIVE_TYPE_DVD_R:		return QLatin1String("DVD-R");
		case DRIVE_TYPE_DVD_RW:		return QLatin1String("DVD-RW");
		case DRIVE_TYPE_DVD_RAM:	return QLatin1String("DVD-RAM");
		case DRIVE_TYPE_DVD_PLUS_RW:	return QLatin1String("DVD+RW");
		case DRIVE_TYPE_DVD_PLUS_R_DL:	return QLatin1String("DVD+R DL");
		case DRIVE_TYPE_DVD_PLUS_RW_DL:	return QLatin1String("DVD+RW DL");
		case DRIVE_TYPE_BDROM:		return QLatin1String("Blu-ray");
		case DRIVE_TYPE_BDROM_DVD_RW:	return QString::fromUtf8("BD/DVD±RW");
		case DRIVE_TYPE_BD_R:		return QLatin1String("BD-R");
		case DRIVE_TYPE_BD_RE:		return QLatin1String("BD-RE");
		case DRIVE_TYPE_HDDVD:		return QLatin1String("HD-DVD");
		case DRIVE_TYPE_HDDVD_DVD_RW:	return QString::fromUtf8("HD-DVD/DVD±RW");
		case DRIVE_TYPE_HDDVD_R:	return QLatin1String("HD-DVD-R");
		case DRIVE_TYPE_HDDVD_RW:	return QLatin1String("HD-DVD-RW");
		case DRIVE_TYPE_MO:		return QLatin1String("MO");
	}
}


/**
 * GetDiscTypeName(): Get the name of a given disc type.
 * @param disc_type Disc type.
 * @return Disc type name, or empty string if the disc type is DISC_TYPE_NONE or invalid.
 * The disc type MUST be a single bit! (Combinations are treated as invalid.)
 */
QString FindCdromBase::GetDiscTypeName(uint32_t disc_type)
{
	switch (disc_type)
	{
		case DISC_TYPE_NONE:
		default:			return QString();
		case DISC_TYPE_CDROM:		return QLatin1String("CD-ROM");
		case DISC_TYPE_CD_R:		return QLatin1String("CD-R");
		case DISC_TYPE_CD_RW:		return QLatin1String("CD-RW");
		case DISC_TYPE_DVD:		return QLatin1String("DVD-ROM");
		case DISC_TYPE_DVD_R:		return QLatin1String("DVD-R");
		case DISC_TYPE_DVD_RW:		return QLatin1String("DVD-RW");
		case DISC_TYPE_DVD_RAM:		return QLatin1String("DVD-RAM");
		case DISC_TYPE_DVD_PLUS_R:	return QLatin1String("DVD+R");
		case DISC_TYPE_DVD_PLUS_RW:	return QLatin1String("DVD+RW");
		case DISC_TYPE_DVD_PLUS_R_DL:	return QLatin1String("DVD+R DL");
		case DISC_TYPE_DVD_PLUS_RW_DL:	return QLatin1String("DVD+RW DL");
		case DISC_TYPE_BDROM:		return QLatin1String("Blu-ray");
		case DISC_TYPE_BD_R:		return QLatin1String("BD-R");
		case DISC_TYPE_BD_RE:		return QLatin1String("BD-RE");
		case DISC_TYPE_HDDVD:		return QLatin1String("HD-DVD");
		case DISC_TYPE_HDDVD_R:		return QLatin1String("HD-DVD-R");
		case DISC_TYPE_HDDVD_RW:	return QLatin1String("HD-DVD-RW");
		
		// TODO: Do we really need to handle these?
		case DISC_TYPE_MO:		return QLatin1String("MO");
		case DISC_TYPE_MRW:		return QLatin1String("MRW");
		case DISC_TYPE_MRW_W:		return QLatin1String("MRW-W");
	}
}


/**
 * GetDriveTypeName(): Get the icon for a given drive type.
 * @param drive_type Drive type.
 * @return Drive type icon, or invalid icon if the drive type is DRIVE_TYPE_NONE or invalid.
 */
QIcon FindCdromBase::GetDriveTypeIcon(DriveType drive_type)
{
	// TODO: Fallback icons.
	// Possibly use Windows or Mac system icons.
	// TODO: Figure out what QStyle::SP_DriveCDIcon and QStyle::SP_DriveDVDIcon are.
	
	// TODO: Add icons for different types of drives.
	((void)drive_type);
	return GensQApplication::IconFromTheme(QLatin1String("drive-optical"));
}


/**
 * GetDiscTypeIcon(): Get the icon for a given disc type.
 * @param disc_type Disc type.
 * @return Disc type icon, or invalid icon if the disc type is DISC_TYPE_NONE or invalid.
 * The disc type MUST be a single bit! (Combinations are treated as invalid.)
 */
QIcon FindCdromBase::GetDiscTypeIcon(uint32_t disc_type)
{
	// TODO: Fallback icons.
	// Possibly use Windows or Mac system icons.
	// TODO: Figure out what QStyle::SP_DriveCDIcon and QStyle::SP_DriveDVDIcon are.
	
	// TODO: Add more unique icons.
	QString iconFdo;
	switch (disc_type)
	{
		case DISC_TYPE_NONE:
		default:
			return QIcon();
		
		case DISC_TYPE_CDROM:
			iconFdo = QLatin1String("media-optical");
			break;
		
		case DISC_TYPE_CD_R:
		case DISC_TYPE_CD_RW:
			iconFdo = QLatin1String("media-optical-recordable");
			break;
		
		case DISC_TYPE_DVD:
			iconFdo = QLatin1String("media-optical-dvd");
			break;
		
		case DISC_TYPE_DVD_R:
		case DISC_TYPE_DVD_RW:
		case DISC_TYPE_DVD_RAM:
		case DISC_TYPE_DVD_PLUS_R:
		case DISC_TYPE_DVD_PLUS_RW:
		case DISC_TYPE_DVD_PLUS_R_DL:
		case DISC_TYPE_DVD_PLUS_RW_DL:
			iconFdo = QLatin1String("media-optical-recordable");
			break;
		
		case DISC_TYPE_BDROM:
		case DISC_TYPE_BD_R:
		case DISC_TYPE_BD_RE:
			iconFdo = QLatin1String("media-optical-blu-ray");
			break;
		
		case DISC_TYPE_HDDVD:
			iconFdo = QLatin1String("media-optical-dvd");
			break;
		
		case DISC_TYPE_HDDVD_R:	
		case DISC_TYPE_HDDVD_RW:
			iconFdo = QLatin1String("media-optical-recordable");
			break;
		
		// TODO: Do we really need to handle these?
		case DISC_TYPE_MO:
		case DISC_TYPE_MRW:
		case DISC_TYPE_MRW_W:
			iconFdo = QLatin1String("media-optical-recordable");
			break;
	}
	
	return GensQApplication::IconFromTheme(iconFdo);
}

}

