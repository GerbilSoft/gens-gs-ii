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

// NTDDSCSI is required for SPTI.
// TODO: Create a separate SPTI class for Win32.
#include <ntddscsi.h>
#include <devioctl.h>
#include "gens_scsi_win32.h"

namespace GensQt4
{

// TODO: Create a separate SPTI class for Win32.
/**
 * ScsiSendCdb(): Send a SCSI command buffer.
 * @return 0 on error; non-zero on success. (Win32-style)
 */
int FindCdromWin32::ScsiSendCdb(HANDLE device, void *cdb, unsigned char cdb_length,
					void *buffer, unsigned int buffer_length,
					int data_in)
{
	// Based on http://www.codeproject.com/KB/system/mydvdregion.aspx
	DWORD returned;
	int ret;
	
	// Size of SCSI_PASS_THROUGH + 96 bytes for sense data.
	uint8_t cmd[sizeof(SCSI_PASS_THROUGH_DIRECT) + 96];
	memset(cmd, 0x00, sizeof(cmd));
	
	// Shortcut to the buffer.
	SCSI_PASS_THROUGH_DIRECT *pcmd = (SCSI_PASS_THROUGH_DIRECT*)cmd;
	
	// Copy the CDB to the SCSI_PASS_THROUGH structure.
	memcpy(pcmd->Cdb, cdb, cdb_length);
	
	// Initialize the other SCSI command variables.
	pcmd->DataBuffer = buffer;
	pcmd->DataTransferLength = buffer_length;
	pcmd->DataIn = data_in;
	pcmd->CdbLength = cdb_length;
	pcmd->Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	pcmd->SenseInfoLength = (sizeof(cmd) - sizeof(SCSI_PASS_THROUGH_DIRECT));
	pcmd->SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);
	pcmd->TimeOutValue = 6000; // TODO: 6000?
	
	ret = DeviceIoControl(device, IOCTL_SCSI_PASS_THROUGH_DIRECT,
				(LPVOID)&cmd, sizeof(cmd),
				(LPVOID)&cmd, sizeof(cmd),
				&returned, NULL);
	
	// TODO: Check for errors.
	DWORD err = GetLastError();
	if (err != ERROR_SUCCESS)
		fprintf(stderr, "%s(): Error: 0x%08X\n", __func__, (unsigned int)err);
	return ret;
}


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
	char spti_drive_path[7] = {'\\', '\\', '.', '\\', 0, ':', 0};
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
		// TODO: Move SPTI-specific code to McdReader later.
		spti_drive_path[4] = drive_path[0];
		HANDLE hDrive = CreateFileA(spti_drive_path,			// lpFileName
					GENERIC_READ | GENERIC_WRITE,		// dwDesiredAccess
					FILE_SHARE_READ | FILE_SHARE_WRITE,	// dwShareMode
					NULL,					// lpSecurityAttributes
					OPEN_EXISTING,				// dwCreationDisposition,
					FILE_ATTRIBUTE_NORMAL,			// dwFlagsAndAttributes
					NULL);					// hTemplateFile
		if (hDrive != INVALID_HANDLE_VALUE)
		{
			// Drive opened for SPTI.
			// Do a drive inquiry.
			CDB_INQUIRY6 inquiry;
			SCSI_INQUIRY_STD_DATA data;
			
			memset(&inquiry, 0x00, sizeof(inquiry));
			memset(&data, 0x00, sizeof(data));
			
			// Set SCSI operation type.
			inquiry.AllocationLength = sizeof(data);
			inquiry.OperationCode6 = SCSI_INQUIRY;
			
			if (ScsiSendCdb(hDrive, &inquiry, sizeof(inquiry), &data, sizeof(data)))
			{
				// Drive inquiry successful.
				// Get the information.
				drive.drive_vendor   = QString::fromLatin1(data.vendor_id, sizeof(data.vendor_id)).trimmed();
				drive.drive_model    = QString::fromLatin1(data.product_id, sizeof(data.product_id)).trimmed();
				drive.drive_firmware = QString::fromLatin1(data.product_revision_level, sizeof(data.product_revision_level)).trimmed();
			}
		}
		
		// Close the drive handle.
		CloseHandle(hDrive);
		
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
