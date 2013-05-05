/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * FindCdromUnix.cpp: Find CD-ROM drives: UNIX fallback.                      *
 *                                                                            *
 * Copyright (c) 2011-2013 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include "FindCdromUnix.hpp"

// C includes.
#include <stdio.h>
#include <paths.h>

// stat(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// errno
#include <errno.h>

// Qt includes.
#include <QtCore/qglobal.h>
#include <QtCore/QDir>
#include <QtCore/QStringList>


namespace GensQt4
{

class FindCdromUnixPrivate
{
	private:
		Q_DISABLE_COPY(FindCdromUnixPrivate)

	public:
		static const char *const UnixDevNames[];
};

/***********************************
 * FindCdromUnixPrivate functions. *
 ***********************************/

const char *const FindCdromUnixPrivate::UnixDevNames[] =
{
#ifdef Q_OS_LINUX
	"cdrom*", "cdrw*", "dvd*", "dvdrw*", "sr*", "scd*", "hd*",
#endif
	nullptr
};


/****************************
 * FindCdromUnix functions. *
 ****************************/

FindCdromUnix::FindCdromUnix(QObject *parent)
	: FindCdromBase(parent)
{ }

/**
 * Scan the system for CD-ROM devices.
 * @return QStringList with all detected CD-ROM device names.
 */
QStringList FindCdromUnix::scanDeviceNames(void)
{
	// Find all CD-ROM devices.

	// Open the /dev/ directory.
	QDir dir = QString::fromLatin1(_PATH_DEV);
	if (!dir.exists())
		return QStringList();

	// Create the filters for the templates.
	QStringList nameFilters;
	for (const char *const *devName = &FindCdromUnixPrivate::UnixDevNames[0];
	     *devName != NULL; devName++)
	{
		nameFilters.append(QLatin1String(*devName));
	}

	// Get a list of filenames.
	QFileInfoList devFiles = dir.entryInfoList(nameFilters,
					(QDir::NoSymLinks | QDir::System), QDir::Name);

	// Search for block devices that are readable by the user.
	QStringList cdromDeviceNames;
	foreach (const QFileInfo& fileInfo, devFiles) {
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
		cdromDeviceNames.append(q_filename);
	}

	return cdromDeviceNames;
}

}
