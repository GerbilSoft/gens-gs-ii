/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * McdControl.hpp: Sega CD Control Panel.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include <gens-qt4/config.gens-qt4.h>

#include "McdControlWindow.hpp"
#include "gqt4_main.hpp"
#include "GensQApplication.hpp"

// Qt includes.
#include <QtGui/QPushButton>

// C includes.
#include <stdint.h>

// Find CD-ROM drives.
#include "cdrom/FindCdromDrives.hpp"

#include "ui_McdControlWindow.h"
namespace GensQt4 {

class McdControlWindowPrivate
{
	public:
		McdControlWindowPrivate(McdControlWindow *q);
		~McdControlWindowPrivate();

	private:
		McdControlWindow *const q_ptr;
		Q_DECLARE_PUBLIC(McdControlWindow)
	private:
		Q_DISABLE_COPY(McdControlWindowPrivate)

	public:
		// Single window instance.
		static McdControlWindow *ms_McdControlWindow;
		Ui::McdControlWindow ui;

		FindCdromDrives *findCdromDrives;
		//bool m_isQuerying;
		//QList<CdromDriveEntry> m_queryList;
		void addDriveEntry(const QString &deviceName, int index = -1);

		// Refresh button.
		QPushButton *btnRefresh;
};

/** McdControlWindowPrivate **/

// Static member initialization.
McdControlWindow *McdControlWindowPrivate::ms_McdControlWindow = nullptr;

McdControlWindowPrivate::McdControlWindowPrivate(McdControlWindow *q)
	: q_ptr(q)
	, findCdromDrives(new FindCdromDrives(q))
	, btnRefresh(nullptr)
{
	// Set up the FindCdromDrives signals.
	// TODO
	/*
	connect(findCdromDrives, SIGNAL(driveUpdated(CdromDriveEntry)),
		this, SLOT(driveUpdated(CdromDriveEntry)));
	connect(m_drives, SIGNAL(driveQueryFinished(void)),
		this, SLOT(driveQueryFinished(void)));
	connect(m_drives, SIGNAL(driveRemoved(QString)),
		this, SLOT(driveRemoved(QString)));
	*/
}

McdControlWindowPrivate::~McdControlWindowPrivate()
{ }

/**
 * Add a CD-ROM drive to the dropdown box.
 * TODO: Replace with MVC so the dropdown box automatically populates itself.
 * @param QString deviceName Device name.
 * @param index Dropdown index, or -1 for a new entry.
 */
void McdControlWindowPrivate::addDriveEntry(const QString &deviceName, int index)
{
	// Get the drive information.
	LibGensCD::CdDrive *cdDrive = findCdromDrives->getCdDrive(deviceName);
	if (!cdDrive)
		return;

	QString item_desc =
			QString::fromStdString(cdDrive->dev_vendor()) + QChar(L' ') +
			QString::fromStdString(cdDrive->dev_model()) + QChar(L' ') +
			QString::fromStdString(cdDrive->dev_firmware()) + QChar(L'\n') +
			deviceName + QLatin1String(": ");

	// TODO: Move the disc info query to a separate thread.

	// Get the disc label.
	if (!cdDrive->isDiscPresent())
		item_desc += McdControlWindow::tr("No medium found.");
	else
		item_desc += QString::fromUtf8(cdDrive->getDiscLabel().c_str());

	// Get the drive icon.
	QIcon icon = findCdromDrives->getDriveIcon(cdDrive);

	// Remove the "No CD-ROM drives found." entry if it's there.
	if (!ui.cboCdDrives->isEnabled()) {
		ui.cboCdDrives->clear();
		ui.cboCdDrives->setEnabled(true);
	}

	// If index is >= 0, this is an existing item.
	if (index >= 0 && index < ui.cboCdDrives->count()) {
		ui.cboCdDrives->setItemText(index, item_desc);
		ui.cboCdDrives->setItemIcon(index, icon);
	} else {
		ui.cboCdDrives->addItem(icon, item_desc, deviceName);
	}

	// Make sure a drive is selected.
	if (ui.cboCdDrives->currentIndex() < 0)
		ui.cboCdDrives->setCurrentIndex(0);
}

/** McdControlWindow **/

/**
 * Initialize the General Configuration window.
 */
McdControlWindow::McdControlWindow(QWidget *parent)
	: super(parent,
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint)
	, d_ptr(new McdControlWindowPrivate(this))
{
	Q_D(McdControlWindow);
	d->ui.setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Create the "Refresh" button.
	// TODO: Don't load an icon on systems that don't use icons on buttons.
	d->btnRefresh = new QPushButton(GensQApplication::IconFromTheme(QLatin1String("view-refresh")),
					tr("&Refresh"), this);
	connect(d->btnRefresh, SIGNAL(clicked()), this, SLOT(query()));
	// NOTE: "ResetRole" isn't exactly the right thing, but it works.
	// On KDE, the button's on the left side of the dialog, whereas "Close" is on the right.
	d->ui.buttonBox->addButton(d->btnRefresh, QDialogButtonBox::ResetRole);

	// Query CD-ROM drives.
	query();
}

/**
 * Shut down the Controller Configuration window.
 */
McdControlWindow::~McdControlWindow()
{
	delete d_ptr;

	// Clear the m_McdControlWindow pointer.
	McdControlWindowPrivate::ms_McdControlWindow = nullptr;
}

/**
 * Show a single instance of the General Configuration window.
 * @param parent Parent window.
 */
void McdControlWindow::ShowSingle(QWidget *parent)
{
	if (McdControlWindowPrivate::ms_McdControlWindow != nullptr) {
		// General Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(McdControlWindowPrivate::ms_McdControlWindow);
	} else {
		// General Configuration Window is not displayed.
		McdControlWindowPrivate::ms_McdControlWindow = new McdControlWindow(parent);
		McdControlWindowPrivate::ms_McdControlWindow->show();
	}
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void McdControlWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(McdControlWindow);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/**
 * Query CD-ROM drives.
 */
void McdControlWindow::query(void)
{
	Q_D(McdControlWindow);

	// Clear the dropdown.
	d->ui.cboCdDrives->clear();

	if (!d->findCdromDrives->isSupported()) {
		// No CD-ROM drive handler is available.
		// TODO: Center-align the text by using QItemDelegate.
		d->ui.cboCdDrives->setEnabled(false);
		d->ui.cboCdDrives->addItem(tr("No CD-ROM drive handler is available."));
		d->ui.cboCdDrives->setCurrentIndex(0);
		return;
	}

	// Set the mouse pointer.
	setCursor(Qt::WaitCursor);

	// Indicate that drives are being scanned.
	// TODO: Center-align the text by using QItemDelegate.
	// TODO: Add an animated "Searching..." icon.
	d->ui.cboCdDrives->setEnabled(false);
	d->ui.cboCdDrives->addItem(tr("Searching for CD-ROM drives..."));
	d->ui.cboCdDrives->setCurrentIndex(0);

	// Retrieve the list of CD-ROM device names.
	QStringList cdromDeviceNames = d->findCdromDrives->getDriveNames();
	foreach (QString deviceName, cdromDeviceNames) {
		d->addDriveEntry(deviceName);
	}

	unsetCursor();
}

// TODO: Replace with FindCdromDrives slots.
#if 0
/**
 * A drive was updated.
 * @param drive CdromDriveEntry.
 */
void McdControlWindow::driveUpdated(const CdromDriveEntry& drive)
{
	if (m_isQuerying) {
		// Querying all drives.
		// Add the drive entry to m_queryList.
		m_queryList.append(drive);
	} else {
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
 * A FindCdromBase::query() request has finished.
 */
void McdControlWindow::driveQueryFinished(void)
{
	// Unset the mouse pointer.
	unsetCursor();

	// Remove the "Searching" text from the dropdown box.
	cboCdDrives->clear();

	if (m_queryList.isEmpty()) {
		// Query list is empty.
		// TODO: Center-align the text by using QItemDelegate.
		cboCdDrives->setEnabled(false);
		cboCdDrives->addItem(tr("No CD-ROM drives found."));
		cboCdDrives->setCurrentIndex(0);
		return;
	}

	foreach (const CdromDriveEntry& drive, m_queryList) {
		addDriveEntry(drive);
	}

	// Set cboCdDrive's selected index to 0 so that the
	// first CD-ROM drive is displayed.
	cboCdDrives->setCurrentIndex(0);
	cboCdDrives->setEnabled(true);

	// Clear the query list.
	m_queryList.clear();
	m_isQuerying = false;
}

/**
 * A drive was removed.
 * @param path Device path.
 */
void McdControlWindow::driveRemoved(QString path)
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

	if (index >= 0 && index < cboCdDrives->count()) {
		cboCdDrives->removeItem(index);
		if (cboCdDrives->count() <= 0) {
			// No drives left.
			cboCdDrives->setEnabled(false);
			cboCdDrives->addItem(tr("No CD-ROM drives found."));
			cboCdDrives->setCurrentIndex(0);
		}
	}
}
#endif

}
