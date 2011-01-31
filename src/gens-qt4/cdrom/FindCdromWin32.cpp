/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromWin32.cpp: Find CD-ROM drives using Win32 API.                 *
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

#include "FindCdromWin32.hpp"

// C includes.
#include <stdio.h>

// Qt includes.
#include <QtCore/QList>

// Text translation macro.
#include <QtCore/QCoreApplication>
#define TR(text) \
	QCoreApplication::translate("FindCdromWin32", (text), NULL, QCoreApplication::UnicodeUTF8)

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// SPTI handler.
#include "Spti.hpp"

namespace GensQt4
{

/**
 * GetVolumeLabel(): Get the label of the given drive.
 * @param drive_letter Drive letter.
 * @return Volume label, or empty string if no device is present.
 */
QString FindCdromWin32::GetVolumeLabel(char drive_letter)
{
	// TODO: ANSI version.
	wchar_t fsNameBuf[MAX_PATH+1];
	wchar_t drive_path[4] = {0, L':', L'\\', 0};
	drive_path[0] = drive_letter;
	
	int ret = GetVolumeInformationW(drive_path, fsNameBuf, sizeof(fsNameBuf),
					NULL, NULL, NULL, NULL, 0);
	if (!ret)
		return QString();
	
	return QString::fromWCharArray(fsNameBuf);
}


/**
 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
 * The driveUpdated() signal will be emitted once for each detected drive.
 * @return 0 on success; non-zero on error.
 */
int FindCdromWin32::query_int(void)
{
	// Find all CD-ROM devices.
	// TODO: Make a base class and return standard values.
	
	// TODO: We're using GetLogicalDrives() to get drive letters.
	// There's probably a better way to get optical drive information,
	// e.g. via WMI, but this is the most compatible.
	DWORD logical_drives = GetLogicalDrives();
	if (!logical_drives)
		return 0;
	
	// Go through the 26 drive letters and find CD-ROM drives.
	// TODO: Get special information like supported disc types.
	char drive_path[4] = {0, ':', '\\', 0};
	unsigned int drive_type;
	for (int i = 0; i < 26; i++)
	{
		drive_path[0] = 'A' + i;
		drive_type = GetDriveTypeA(drive_path);
		if (drive_type != DRIVE_CDROM)
			continue;
		
		// This is a CD-ROM drive.
		// Construct the CdromDriveEntry.
		CdromDriveEntry drive;
		drive.discs_supported = 0;		// TODO
		drive.drive_type = DRIVE_TYPE_CDROM;	// TODO
		drive.disc_type = DISC_TYPE_CDROM;	// TODO
		drive.disc_blank = false;		// TODO
		
		// Save the drive path.
		drive.path = QString::fromLatin1(drive_path);
		
		// Get the disc label.
		drive.disc_label = GetVolumeLabel(drive_path[0]);
		
		// Open the drive using SPTI.
		Spti drive_spti(drive_path[0]);
		if (drive_spti.isOpen())
		{
			// Drive opened for SPTI.
			// Do a drive inquiry.
			if (!drive_spti.scsiInquiry())
			{
				// Drive inquiry successful.
				// Get the information.
				drive.drive_vendor   = QString::fromLatin1(drive_spti.inqVendor());
				drive.drive_model    = QString::fromLatin1(drive_spti.inqModel());
				drive.drive_firmware = QString::fromLatin1(drive_spti.inqFirmware());
			}
			
			// Check if a disc is inserted.
			// NOTE: Label may be empty, so this check isn't necessarily correct.
			if (drive.disc_label.isEmpty())
				drive.disc_type = DISC_TYPE_NONE;
			else
			{
				if (drive_spti.isMediumPresent())
					drive.disc_type = DISC_TYPE_CDROM;
				else
					drive.disc_type = DISC_TYPE_NONE;
			}
		}
		
		// Close the drive handle.
		drive_spti.close();
		
		// TODO: Get optical disc type and drive type information.
		drive.disc_blank = false;
		
		// If the disc is blank, set the disc label to "Blank [disc_type]".
		// TODO: Make this a common FindCdromBase function?
		if (drive.disc_type != DISC_TYPE_NONE && drive.disc_blank)
			drive.disc_label = TR("Blank %1").arg(GetDiscTypeName(drive.disc_type));
		
		// Emit the driveUpdated() signal for this drive.
		// TODO: If scanning all drives, notify when all drives are scanned.
		emit driveUpdated(drive);
		
		// Print drive information.
		printf("Drive: %s - drive is type %d, disc is 0x%08X\n",
		       drive.path.toLocal8Bit().constData(), (int)drive.drive_type, drive.disc_type);
	}
	
	// Devices queried.
	return 0;
}

}
