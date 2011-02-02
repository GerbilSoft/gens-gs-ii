/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromUnix.cpp: Find CD-ROM drives. (UNIX fallback)                  *
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

#include "FindCdromUnix.hpp"

// C includes.
#include <stdio.h>

// stat(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// open(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// ioctl(2)
#include <sys/ioctl.h>

// errno
#include <errno.h>

// Qt includes.
#include <QtCore/qglobal.h>
#include <QtCore/QList>
#include <QtCore/QScopedPointer>
#include <QtCore/QDir>
#include <QtCore/QStringList>

// Text translation macro.
#include <QtCore/QCoreApplication>
#define TR(text) \
	QCoreApplication::translate("FindCdromUnix", (text), NULL, QCoreApplication::UnicodeUTF8)

#ifdef Q_OS_LINUX
// HDIO_GET_IDENTITY
#include <linux/hdreg.h>
// CDROM_GET_CAPABILITY
#include <linux/cdrom.h>
#endif

namespace GensQt4
{

const char *FindCdromUnix::ms_Unix_DevNames[] =
{
#ifdef Q_OS_LINUX
	"cdrom*", "cdrw*", "dvd*", "dvdrw*", "sr*", "scd*", "hd*",
#endif
	
	NULL
};

/**
 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
 * The driveUpdated() signal will be emitted once for each detected drive.
 * @return 0 on success; non-zero on error.
 */
int FindCdromUnix::query_int(void)
{
	// Find all CD-ROM devices.
	
	// Open the /dev/ directory.
	QDir dir("/dev/");
	if (!dir.exists())
		return -1;
	
	// Create the filters for the templates.
	QStringList nameFilters;
	for (const char **devName = &ms_Unix_DevNames[0];
	     *devName != NULL; devName++)
	{
		nameFilters.append(QString::fromLatin1(*devName));
	}
	
	// Get a list of filenames.
	QFileInfoList devFiles = dir.entryInfoList(nameFilters, (QDir::NoSymLinks | QDir::System), QDir::Name);
	
	// Search for block devices that are readable by the user.
	QFileInfo fileInfo;
	QStringList devicesUsable;
	foreach(fileInfo, devFiles)
	{
		if (fileInfo.isDir() || !fileInfo.isReadable())
			continue;
		
		// Check if this is a block device file.
		QString q_filename = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
		const char *c_filename = q_filename.toLocal8Bit().constData();
		
		struct stat st_devFile;
		if (stat(c_filename, &st_devFile))
			continue;
		if (!S_ISBLK(st_devFile.st_mode))
			continue;
		
		// This file is a block device.
		devicesUsable.append(q_filename);
	}
	
	// Get drive information.
	// TODO: We're just getting the pathname right now.
	QString devFilename;
	foreach(devFilename, devicesUsable)
	{
		// Construct the CdromDriveEntry.
		CdromDriveEntry drive;
		drive.path = devFilename;
		
		// Get various properties.
		// TODO: Get the properties.
		drive.drive_type = DRIVE_TYPE_CDROM;
		drive.discs_supported = DISC_TYPE_CDROM;
		drive.disc_type = DISC_TYPE_CDROM;
		
		// Open the device file.
		const char *c_devFilename = devFilename.toLocal8Bit().constData();
		int fd_drive = open(c_devFilename, (O_RDONLY | O_NONBLOCK));
		if (fd_drive >= 0)
		{
			// Device file descriptor is open.
			// Get the device information.
			os_GetDevIdentity(fd_drive, drive);
			
			// Get the drive type.
			drive.drive_type = os_GetDriveType(fd_drive);
			
			// TODO: Get disc label and other properties.
			close(fd_drive);
		}
		
		// TODO
		drive.disc_label	= "Disc Label";
		drive.disc_blank	= false;
		
		// TODO: Check if a disc is in the drive.
		
		// If the disc is blank, set the disc label to "Blank [disc_type]".
		// TODO: Make this a common FindCdromBase function?
		if (drive.disc_type != DISC_TYPE_NONE && drive.disc_blank)
			drive.disc_label = TR("Blank %1").arg(GetDiscTypeName(drive.disc_type));
		
		// Emit the driveUpdated() signal for this drive.
		// TODO: If scanning all drives, notify when all drives are scanned.
		emit driveUpdated(drive);
	}
	
	// Devices queried.
	return 0;
}


/**
 * os_GetDevVendor(): Get device identity.
 * This includes vendor, model, and firmware revision.
 * @param fd	[in] File descriptor.
 * @param entry	[out] CD-ROM device entry.
 * @return 0 on success; non-zero on error.
 */
int FindCdromUnix::os_GetDevIdentity(int fd, CdromDriveEntry &entry)
{
#if defined(Q_OS_LINUX) && defined(HDIO_GET_IDENTITY)
	struct hd_driveid drive_id;
	if (ioctl(fd, HDIO_GET_IDENTITY, &drive_id) != 0)
	{
		// Error calling ioctl().
		return -errno;
	}
	
	// Get the device identity from the struct hd_driveid.
	entry.drive_vendor   = QString::fromLatin1((const char*)drive_id.model, 8).trimmed();
	entry.drive_model    = QString::fromLatin1((const char*)&drive_id.model[8],
						   (sizeof(drive_id.model) - 8)).trimmed();
	entry.drive_firmware = QString::fromLatin1((const char*)drive_id.fw_rev,
						   sizeof(drive_id.fw_rev)).trimmed();
	return 0;
#else
	// Other Unix system.
	// TODO
	return -EIMPL;
#endif
}


/**
 * os_GetDriveType(): Get drive type.
 * @param fd	[in] File descriptor.
 * @return Drive type, or DRIVE_TYPE_NONE on error.
 */
DriveType FindCdromUnix::os_GetDriveType(int fd)
{
#if defined(Q_OS_LINUX) && defined(CDROM_GET_CAPABILITY)
	int caps = ioctl(fd, CDROM_GET_CAPABILITY, 0);
	if (caps < 0)
		return DRIVE_TYPE_NONE;
	
	// Determine the drive type based on capabilities.
	// TODO: Blu-ray / HD-DVD?
	// TODO: DVD+RW?
	// TODO: Is there such a thing as DVD/CD-R, or just DVD/CD-RW?
	if (caps & CDC_MO_DRIVE)
		return DRIVE_TYPE_MO;
	else if (caps & CDC_DVD_RAM)
		return DRIVE_TYPE_DVD_RAM;
	else if (caps & CDC_DVD_R)
		return DRIVE_TYPE_DVD_R;
	else if (caps & (CDC_DVD | CDC_CD_R | CDC_CD_RW))
		return DRIVE_TYPE_DVD_CD_RW;
	else if (caps & CDC_DVD)
		return DRIVE_TYPE_DVD;
	else if (caps & CDC_CD_RW)
		return DRIVE_TYPE_CD_RW;
	else if (caps & CDC_CD_R)
		return DRIVE_TYPE_CD_R;
	else
		return DRIVE_TYPE_CDROM;
#else
	// Other Unix system.
	// TODO
	return DRIVE_TYPE_NONE;
#endif
}

}
