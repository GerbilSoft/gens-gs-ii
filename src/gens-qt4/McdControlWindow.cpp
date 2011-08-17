/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * McdControl.hpp: Sega CD Control Panel.                                  *
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

#include <config.h>

#include "McdControlWindow.hpp"
#include "gqt4_main.hpp"
#include "GensQApplication.hpp"

// Qt includes.
#include <QtGui/QPushButton>

// C includes.
#include <stdint.h>

// Find CD-ROM drives.
// TODO: Add Mac OS X, and non-UDisks classes.
#if defined(Q_OS_WIN)
#include "cdrom/FindCdromWin32.hpp"
#elif defined(QT_QTDBUS_FOUND)
#include "cdrom/FindCdromUDisks.hpp"
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include "cdrom/FindCdromUnix.hpp"
#endif

namespace GensQt4
{

// Static member initialization.
McdControlWindow *McdControlWindow::m_McdControlWindow = NULL;

/**
 * McdControlWindow(): Initialize the General Configuration window.
 */
McdControlWindow::McdControlWindow(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	// Initialize the Qt4 UI.
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif
	
	// Create the "Refresh" button.
	// TODO: Don't load an icon on systems that don't use icons on buttons.
	btnRefresh = new QPushButton(GensQApplication::IconFromTheme(QLatin1String("view-refresh")),
					tr("&Refresh"), this);
	connect(btnRefresh, SIGNAL(clicked()), this, SLOT(query()));
	// NOTE: "ResetRole" isn't exactly the right thing, but it works.
	// On KDE, the button's on the left side of the dialog, whereas "Close" is on the right.
	buttonBox->addButton(btnRefresh, QDialogButtonBox::ResetRole);
	
	// Initialize the FindCdromBase class.
#if defined(Q_OS_WIN)
	m_drives = new FindCdromWin32();
#elif defined(QT_QTDBUS_FOUND)
	m_drives = new FindCdromUDisks();
	if (!m_drives->isUsable())
	{
		delete m_drives;
		m_drives = NULL;
	}
#else
	// TODO: Implement FindCdromBase subclass for Mac OS X.
	m_drives = NULL;
#endif
	
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// UNIX fallback.
	if (!m_drives)
		m_drives = new FindCdromUnix();
#endif
	
	if (m_drives)
	{
		// Set up the FindCdromBase signals
		connect(m_drives, SIGNAL(driveUpdated(CdromDriveEntry)),
			this, SLOT(driveUpdated(CdromDriveEntry)));
		connect(m_drives, SIGNAL(driveQueryFinished(void)),
			this, SLOT(driveQueryFinished(void)));
		connect(m_drives, SIGNAL(driveRemoved(QString)),
			this, SLOT(driveRemoved(QString)));
	}
	
	// Query CD-ROM drives.
	query();
}


/**
 * ~McdControlWindow(): Shut down the Controller Configuration window.
 */
McdControlWindow::~McdControlWindow()
{
	// Clear the m_McdControlWindow pointer.
	m_McdControlWindow = NULL;
	
	// Delete m_drives.
	delete m_drives;
	m_drives = NULL;
}


/**
 * ShowSingle(): Show a single instance of the General Configuration window.
 * @param parent Parent window.
 */
void McdControlWindow::ShowSingle(QWidget *parent)
{
	if (m_McdControlWindow != NULL)
	{
		// General Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(m_McdControlWindow);
	}
	else
	{
		// General Configuration Window is not displayed.
		m_McdControlWindow = new McdControlWindow(parent);
		m_McdControlWindow->show();
	}
}


/**
 * changeEvent(): Widget state has changed.
 * @param event State change event.
 */
void McdControlWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		// Retranslate the UI.
		retranslateUi(this);
	}
	
	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}


/**
 * query(): Query CD-ROM drives.
 */
void McdControlWindow::query(void)
{
	// Clear the dropdown.
	cboCdDrives->clear();
	
	if (!m_drives)
	{
		// No CD-ROM drive handler is available.
		// TODO: Center-align the text by using QItemDelegate.
		cboCdDrives->setEnabled(false);
		cboCdDrives->addItem(tr("No CD-ROM drive handler is available."));
		cboCdDrives->setCurrentIndex(0);
		return;
	}
	
	// Set the mouse pointer.
	setCursor(Qt::WaitCursor);
	
	// Indicate that drives are being scanned.
	// TODO: Center-align the text by using QItemDelegate.
	// TODO: Add an animated "Searching..." icon.
	cboCdDrives->setEnabled(false);
	cboCdDrives->addItem(tr("Searching for CD-ROM drives..."));
	cboCdDrives->setCurrentIndex(0);
	
	// Query the drives.
	m_queryList.clear();
	m_isQuerying = true;
	m_drives->query();
}


/**
 * addDriveEntry(): Add a CdromDriveEntry to the dropdown box.
 * @param drive Drive entry.
 */
void McdControlWindow::addDriveEntry(const CdromDriveEntry& drive, int index)
{
	// Add the drive to the dropdown box.
	QString item_desc = drive.drive_vendor + QChar(L' ') +
				drive.drive_model + QChar(L' ') +
				drive.drive_firmware + QChar(L'\n') +
				drive.path + QString::fromLatin1(": ");
	
	// Add the disc label if a disc is present.
	if (drive.disc_type == 0)
		item_desc += tr("No medium found.");
	else
		item_desc += drive.disc_label;
	
	// Get the drive/disc icon.
	QIcon icon = m_drives->getDriveIcon(drive);
	
	// Remove the "No CD-ROM drives found." entry if it's there.
	if (!cboCdDrives->isEnabled())
	{
		cboCdDrives->clear();
		cboCdDrives->setEnabled(true);
	}
	
	// If index is >= 0, this is an existing item.
	if (index >= 0 && index < cboCdDrives->count())
	{
		cboCdDrives->setItemText(index, item_desc);
		cboCdDrives->setItemIcon(index, icon);
	}
	else
	{
		cboCdDrives->addItem(icon, item_desc, drive.path);
	}
	
	// Make sure a drive is selected.
	if (cboCdDrives->currentIndex() < 0)
		cboCdDrives->setCurrentIndex(0);
}


/**
 * driveUpdated(): A drive was updated.
 * @param drive CdromDriveEntry.
 */
void McdControlWindow::driveUpdated(const CdromDriveEntry& drive)
{
	if (m_isQuerying)
	{
		// Querying all drives.
		// Add the drive entry to m_queryList.
		m_queryList.append(drive);
	}
	else
	{
		// Not querying drives.
		// This is an actual drive update signal.
		int index = cboCdDrives->findData(drive.path, Qt::UserRole,
#if defined(Q_OS_WIN)
						(Qt::MatchFixedString | Qt::MatchCaseSensitive)
#else
						Qt::MatchFixedString
#endif
				       );
		
		addDriveEntry(drive, index);
	}
}


/**
 * driveQueryFinished(): A FindCdromBase::query() request has finished.
 */
void McdControlWindow::driveQueryFinished(void)
{
	// Unset the mouse pointer.
	unsetCursor();
	
	// Remove the "Searching" text from the dropdown box.
	cboCdDrives->clear();
	
	if (m_queryList.isEmpty())
	{
		// Query list is empty.
		// TODO: Center-align the text by using QItemDelegate.
		cboCdDrives->setEnabled(false);
		cboCdDrives->addItem(tr("No CD-ROM drives found."));
		cboCdDrives->setCurrentIndex(0);
		return;
	}
	
	foreach (const CdromDriveEntry& drive, m_queryList)
		addDriveEntry(drive);
	
	// Set cboCdDrive's selected index to 0 so that the
	// first CD-ROM drive is displayed.
	cboCdDrives->setCurrentIndex(0);
	cboCdDrives->setEnabled(true);
	
	// Clear the query list.
	m_queryList.clear();
	m_isQuerying = false;
}


/**
 * driveRemoved(): A drive was removed.
 * @param path Device path.
 */
void McdControlWindow::driveRemoved(const QString& path)
{
	if (m_isQuerying)
		return;
	if (!cboCdDrives->isEnabled())
		return;
	
	// If the drive exists in the dropdown, remove it.
	int index = cboCdDrives->findData(path, Qt::UserRole,
#if defined(Q_OS_WIN)
					(Qt::MatchFixedString | Qt::MatchCaseSensitive)
#else
					Qt::MatchFixedString
#endif
			       );
	
	if (index >= 0 && index < cboCdDrives->count())
	{
		cboCdDrives->removeItem(index);
		if (cboCdDrives->count() <= 0)
		{
			// No drives left.
			cboCdDrives->setEnabled(false);
			cboCdDrives->addItem(tr("No CD-ROM drives found."));
			cboCdDrives->setCurrentIndex(0);
		}
	}
}

}
