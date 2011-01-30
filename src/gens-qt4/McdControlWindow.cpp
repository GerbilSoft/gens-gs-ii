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

// Text translation macro.
#define TR(text) \
	QApplication::translate("McdControlWindow", (text), NULL, QApplication::UnicodeUTF8)

/**
 * QICON_FROMTHEME(): Icon loading function.
 * Qt 4.6 supports FreeDesktop.org icon themes.
 * Older versions do not, unfortunately.
 * TODO: Combine with GensMenuBar's QICON_FROMTHEME()?
 */
#if QT_VERSION >= 0x040600
#define QICON_FROMTHEME(name, fallback) \
	(QIcon::hasThemeIcon(name) ? QIcon::fromTheme(name) : QIcon(fallback))
#else
#define QICON_FROMTHEME(name, fallback) \
	QIcon(fallback)
#endif

// C includes.
#include <stdint.h>

// Find CD-ROM drives.
// TODO: Add Win32, Mac OS X, and non-UDisks classes.
#ifdef QT_QTDBUS_FOUND
#include "cdrom/FindCdromUDisks.hpp"
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
	
	// Create the "Refresh" button.
	// TODO: Don't load an icon on systems that don't use icons on buttons.
	btnRefresh = new QPushButton(QICON_FROMTHEME("view-refresh", ":/oxygen-16x16/view-refresh.png"),
					TR("&Refresh"), this);
	connect(btnRefresh, SIGNAL(clicked()), this, SLOT(query()));
	// NOTE: "ResetRole" isn't exactly the right thing, but it works.
	// On KDE, the button's on the left side of the dialog, whereas "Close" is on the right.
	buttonBox->addButton(btnRefresh, QDialogButtonBox::ResetRole);
	
	// Initialize the FindCdromBase class.
#ifdef QT_QTDBUS_FOUND
	m_drives = new FindCdromUDisks();
#else
	// TODO: Implement versions for Win32, Mac OS X, and non-UDisks.
	m_drives = NULL;
#endif
	
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
 * query(): Query CD-ROM drives.
 */
void McdControlWindow::query(void)
{
	// Clear the dropdown.
	cboCdDrives->clear();
	
	if (!m_drives)
		return;
	
	m_drives->query();
	
	// Add the drives tothe list.
	// TODO: Asynchronous scanning.
	FindCdromBase::drive_entry_t drive_entry;
	foreach(drive_entry, m_drives->getDriveList())
	{
		QString item_desc = drive_entry.drive_vendor + " " +
					drive_entry.drive_model + " " +
					drive_entry.drive_firmware + "\n" +
					drive_entry.path + ": ";
		
		// Add the disc label if a disc is present.
		if (drive_entry.disc_type == 0)
			item_desc += TR("No medium found.");
		else
			item_desc += drive_entry.disc_label;
		
		cboCdDrives->addItem(drive_entry.icon, item_desc, drive_entry.path);
	}
	
	// Set cboCdDrive's selected index to 0 so that the
	// first CD-ROM drive is displayed.
	cboCdDrives->setCurrentIndex(0);
}

}
