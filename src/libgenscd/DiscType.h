/***************************************************************************
 * libgenscd: Gens/GS CD-ROM accesslibrary.                                *
 * DiscType.h: Disc and drive type definitions.                            *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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

#ifndef __LIBGENSCD_DISCTYPE_H__
#define __LIBGENSCD_DISCTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	DISC_TYPE_NONE			= 0,
	DISC_TYPE_MO			= (1 << 0),
	DISC_TYPE_CDROM			= (1 << 1),
	DISC_TYPE_CD_R			= (1 << 2),
	DISC_TYPE_CD_RW			= (1 << 3),
	DISC_TYPE_DVDROM		= (1 << 4),
	DISC_TYPE_DVD_R			= (1 << 5),
	DISC_TYPE_DVD_RAM		= (1 << 6),
	DISC_TYPE_DVD_RW		= (1 << 7),
	DISC_TYPE_DVD_R_DL		= (1 << 8),
	DISC_TYPE_DVD_RW_DL		= (1 << 9),
	DISC_TYPE_DVD_PLUS_RW		= (1 << 10),
	DISC_TYPE_DVD_PLUS_R		= (1 << 11),
	DISC_TYPE_DDCDROM		= (1 << 12),
	DISC_TYPE_DDCD_R		= (1 << 13),
	DISC_TYPE_DDCD_RW		= (1 << 14),
	DISC_TYPE_DVD_PLUS_RW_DL	= (1 << 15),
	DISC_TYPE_DVD_PLUS_R_DL		= (1 << 16),
	DISC_TYPE_BDROM			= (1 << 17),
	DISC_TYPE_BD_R			= (1 << 18),
	DISC_TYPE_BD_RE			= (1 << 19),
	DISC_TYPE_HDDVD			= (1 << 20),
	DISC_TYPE_HDDVD_R		= (1 << 21),
	DISC_TYPE_HDDVD_RAM		= (1 << 22),
	DISC_TYPE_HDDVD_RW		= (1 << 23),
	DISC_TYPE_HDDVD_R_DL		= (1 << 24),
	DISC_TYPE_HDDVD_RW_DL		= (1 << 25),
} CD_DiscType_t;

typedef enum
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
	DRIVE_TYPE_DDCDROM,
	DRIVE_TYPE_DDCD_R,
	DRIVE_TYPE_DDCD_RW,
	DRIVE_TYPE_DVD_PLUS_R_DL,
	DRIVE_TYPE_DVD_PLUS_RW_DL,
	DRIVE_TYPE_BDROM,
	DRIVE_TYPE_BDROM_DVD_RW,
	DRIVE_TYPE_BD_R,
	DRIVE_TYPE_BD_RE,
	DRIVE_TYPE_HDDVD,
	DRIVE_TYPE_HDDVD_DVD_RW,
	DRIVE_TYPE_HDDVD_R,
	DRIVE_TYPE_HDDVD_RW,
	DRIVE_TYPE_MO
} CD_DriveType_t;

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENSCD_DISCTYPE_H__ */
