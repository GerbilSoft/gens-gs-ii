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

/**
 * QICON_FROMTHEME(): Icon loading function.
 * Qt 4.6 supports FreeDesktop.org icon themes.
 * Older versions do not, unfortunately.
 * TODO: Combine with GensMenuBar's QICON_FROMTHEME()?
 */
#if QT_VERSION >= 0x040600
#define QICON_FROMTHEME(name, fallback) \
	(QIcon::hasThemeIcon(name) ? QIcon::fromTheme(name) : QIcon(fallback))
#define QICON_FROMTHEME_NF(name) \
	(QIcon::hasThemeIcon(name) ? QIcon::fromTheme(name) : QIcon())
#else
#define QICON_FROMTHEME(name, fallback) \
	QIcon(fallback)
#define QICON_FROMTHEME_NF(name)
#endif

namespace GensQt4
{

/**
 * GetDriveType(): Get the drive name from a given drive specifier.
 * @param discs_supported Supported discs.
 * @return DriveType.
 */
FindCdromBase::DriveType FindCdromBase::GetDriveType(uint32_t discs_supported)
{
	// TODO: Find various permutations like DVD/CD-RW.
	// Also, check for multi-format DVD±RW drives.
	// For now, just get the maximum disc type.
	if (discs_supported & DISC_TYPE_HDDVD_RW)
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
		case DRIVE_TYPE_CDROM:		return "CD-ROM";
		case DRIVE_TYPE_CD_R:		return "CD-R";
		case DRIVE_TYPE_CD_RW:		return "CD-RW";
		case DRIVE_TYPE_DVD:		return "DVD-ROM";
		case DRIVE_TYPE_DVD_CD_RW:	return "DVD/CD-RW";
		case DRIVE_TYPE_DVD_R:		return "DVD-R";
		case DRIVE_TYPE_DVD_RW:		return "DVD-RW";
		case DRIVE_TYPE_DVD_RAM:	return "DVD-RAM";
		case DRIVE_TYPE_DVD_PLUS_RW:	return "DVD+RW";
		case DRIVE_TYPE_DVD_PLUS_R_DL:	return "DVD+R DL";
		case DRIVE_TYPE_DVD_PLUS_RW_DL:	return "DVD+RW DL";
		case DRIVE_TYPE_BDROM:		return "Blu-ray";
		case DRIVE_TYPE_BDROM_DVD_RW:	return "BD/DVD±RW";
		case DRIVE_TYPE_BD_R:		return "BD-R";
		case DRIVE_TYPE_BD_RE:		return "BD-RE";
		case DRIVE_TYPE_HDDVD:		return "HD-DVD";
		case DRIVE_TYPE_HDDVD_DVD_RW:	return "HD-DVD/DVD±RW";
		case DRIVE_TYPE_HDDVD_R:	return "HD-DVD-R";
		case DRIVE_TYPE_HDDVD_RW:	return "HD-DVD-RW";
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
		case DISC_TYPE_CDROM:		return "CD-ROM";
		case DISC_TYPE_CD_R:		return "CD-R";
		case DISC_TYPE_CD_RW:		return "CD-RW";
		case DISC_TYPE_DVD:		return "DVD-ROM";
		case DISC_TYPE_DVD_R:		return "DVD-R";
		case DISC_TYPE_DVD_RW:		return "DVD-RW";
		case DISC_TYPE_DVD_RAM:		return "DVD-RAM";
		case DISC_TYPE_DVD_PLUS_R:	return "DVD+R";
		case DISC_TYPE_DVD_PLUS_RW:	return "DVD+RW";
		case DISC_TYPE_DVD_PLUS_R_DL:	return "DVD+R DL";
		case DISC_TYPE_DVD_PLUS_RW_DL:	return "DVD+RW DL";
		case DISC_TYPE_BDROM:		return "Blu-ray";
		case DISC_TYPE_BD_R:		return "BD-R";
		case DISC_TYPE_BD_RE:		return "BD-RE";
		case DISC_TYPE_HDDVD:		return "HD-DVD";
		case DISC_TYPE_HDDVD_R:		return "HD-DVD-R";
		case DISC_TYPE_HDDVD_RW:	return "HD-DVD-RW";
		
		// TODO: Do we really need to handle these?
		case DISC_TYPE_MO:		return "MO";
		case DISC_TYPE_MRW:		return "MRW";
		case DISC_TYPE_MRW_W:		return "MRW-W";
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
	return QICON_FROMTHEME_NF("drive-optical");
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
	switch (disc_type)
	{
		case DISC_TYPE_NONE:
		default:			return QIcon();
		case DISC_TYPE_CDROM:		return QICON_FROMTHEME_NF("media-optical");
		case DISC_TYPE_CD_R:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_CD_RW:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD:		return QICON_FROMTHEME_NF("media-optical-dvd");
		case DISC_TYPE_DVD_R:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_RW:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_RAM:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_PLUS_R:	return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_PLUS_RW:	return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_PLUS_R_DL:	return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_DVD_PLUS_RW_DL:	return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_BDROM:		return QICON_FROMTHEME_NF("media-optical-blu-ray");
		case DISC_TYPE_BD_R:		return QICON_FROMTHEME_NF("media-optical-blu-ray");
		case DISC_TYPE_BD_RE:		return QICON_FROMTHEME_NF("media-optical-blu-ray");
		case DISC_TYPE_HDDVD:		return QICON_FROMTHEME_NF("media-optical-dvd");;
		case DISC_TYPE_HDDVD_R:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_HDDVD_RW:	return QICON_FROMTHEME_NF("media-optical-recordable");
		
		// TODO: Do we really need to handle these?
		case DISC_TYPE_MO:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_MRW:		return QICON_FROMTHEME_NF("media-optical-recordable");
		case DISC_TYPE_MRW_W:		return QICON_FROMTHEME_NF("media-optical-recordable");
	}
}

}
