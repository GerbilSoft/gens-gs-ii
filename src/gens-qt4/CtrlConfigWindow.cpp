/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfigWindow.cpp: Controller Configuration Window.                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "CtrlConfigWindow.hpp"
#include "gqt4_main.hpp"

// C includes.
#include <assert.h>

// Controller I/O manager.
#include "libgens/IoManager.hpp"
using LibGens::IoManager;

// EmuMD has the I/O devices.
#include "libgens/MD/EmuMD.hpp"

// Qt includes.
#include <QtGui/QActionGroup>
#include <QtGui/QKeyEvent>
#include <QtGui/QPushButton>
#include <QtCore/QFile>

namespace GensQt4
{

class CtrlConfigWindowPrivate
{
	public:
		CtrlConfigWindowPrivate(CtrlConfigWindow *q);
		~CtrlConfigWindowPrivate();

	private:
		CtrlConfigWindow *const q;
		Q_DISABLE_COPY(CtrlConfigWindowPrivate)

	public:
		// Single window instance.
		static CtrlConfigWindow *ms_Window;

		// Controller data.
		static const char *const CtrlIconFilenames[LibGens::IoManager::IOT_MAX];
		QString getShortDeviceName(LibGens::IoManager::IoType_t ioType) const;
		QString getLongDeviceName(LibGens::IoManager::IoType_t ioType) const;
		QString getPortName(LibGens::IoManager::VirtPort_t virtPort) const;
		QIcon getCtrlIcon(LibGens::IoManager::IoType_t ioType) const;

		// Internal CtrlConfig instance.
		CtrlConfig *ctrlConfig;

		// Selected port.
		LibGens::IoManager::VirtPort_t selPort;
		QActionGroup *actgrpSelPort;
		QSignalMapper *mapperSelPort;

		// Toolbar separators.
		QVector<QAction*> vecTbSep;

	private:
		// Dropdown device lock.
		// Used when rebuilding cboDevice.
		int m_cboDeviceLockCnt;

	public:
		/**
		 * Lock cboDevice.
		 * This turns off signal handling for cboDevice.
		 */
		inline void cboDevice_lock(void)
			{ m_cboDeviceLockCnt++; }

		/**
		 * Unlock cboDevice.
		 * This turns on signal handling for cboDevice once m_cboDeviceLockCnt reaches 0.
		 */
		inline void cboDevice_unlock(void)
		{
			assert(m_cboDeviceLockCnt >= 0);
			if (m_cboDeviceLockCnt > 0)
				m_cboDeviceLockCnt--;
		}

		/**
		 * Check if cboDevice is locked.
		 * @return True if locked; false if not.
		 */
		inline bool isCboDeviceLocked(void) const
			{ return (m_cboDeviceLockCnt > 0); }
};

/**************************************
 * CtrlConfigWindowPrivate functions. *
 **************************************/

// Single window instance.
CtrlConfigWindow *CtrlConfigWindowPrivate::ms_Window = NULL;

// Controller icon filenames.
const char *const CtrlConfigWindowPrivate::CtrlIconFilenames[IoManager::IOT_MAX] =
{
	"controller-none.png",		// IOT_NONE
	"controller-3btn.png",		// IOT_3BTN
	"controller-6btn.png",		// IOT_6BTN
	"controller-2btn.png",		// IOT_2BTN
	"controller-mega-mouse.png",	// IOT_MEGA_MOUSE (TODO)
	"controller-teamplayer.png",	// IOT_TEAMPLAYER (TODO)
	"controller-4wp.png",		// IOT_4WP_MASTER (TODO)
	"controller-4wp.png",		// IOT_4WP_SLAVE (TODO)
};

CtrlConfigWindowPrivate::CtrlConfigWindowPrivate(CtrlConfigWindow *q)
	: q(q)
	, ctrlConfig(new CtrlConfig(q))
	, selPort(IoManager::VIRTPORT_1)
	, actgrpSelPort(new QActionGroup(q))
	, mapperSelPort(new QSignalMapper(q))
	, m_cboDeviceLockCnt(0)
{
	// Set the single window instance pointer.
	ms_Window = q;

	// Signal mapper for toolbar buttons.
	QObject::connect(mapperSelPort, SIGNAL(mapped(int)),
			  q, SLOT(toolbarPortSelected(int)));
}

CtrlConfigWindowPrivate::~CtrlConfigWindowPrivate()
{
	// Clear the single window instance pointer.
	ms_Window = NULL;
}

/**
 * Get the short name of an I/O device.
 * @param ioType Device type.
 * @return Short device name.
 */
QString CtrlConfigWindowPrivate::getShortDeviceName(IoManager::IoType_t ioType) const
{
	switch (ioType) {
		case IoManager::IOT_NONE:
		default:
			return q->tr("None");
		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return q->tr("3-button");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return q->tr("6-button");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return q->tr("2-button");
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return q->tr("Mega Mouse");
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return q->tr("Team Player");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return q->tr("4-Way Play");
	}
}

/**
 * Get the long name of an I/O device.
 * @param ioType Device type.
 * @return Long device name.
 */
QString CtrlConfigWindowPrivate::getLongDeviceName(IoManager::IoType_t ioType) const
{
	switch (ioType) {
		case IoManager::IOT_NONE:
		default:
			return q->tr("No device connected.");
		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return q->tr("3-button gamepad");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return q->tr("6-button gamepad");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return q->tr("2-button gamepad (SMS)");
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return q->tr("Mega Mouse");
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return q->tr("Sega Team Player");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return q->tr("EA 4-Way Play");
	}
}

/**
 * Get the name of a given port.
 * @param virtPort Virtual port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigWindowPrivate::getPortName(IoManager::VirtPort_t virtPort) const
{
	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:
		case IoManager::VIRTPORT_2:
			return q->tr("Port %1")
				.arg(virtPort - IoManager::VIRTPORT_1 + 1);

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			return q->tr("Team Player %1, Port %2").arg(1)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP1A)));

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			return q->tr("Team Player %1, Port %2").arg(2)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP2A)));

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			return q->tr("4-Way Play, Port %1")
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_4WPA)));

		default:
			// Unknown port number.
			return QString();
	}

	// Should not get here...
	return QString();
}

/**
 * Get an icon for the specified controller type.
 * @param ioType Controller type.
 * @return Icon for the specified controller type.
 */
QIcon CtrlConfigWindowPrivate::getCtrlIcon(IoManager::IoType_t ioType) const
{
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	
	// Create the icon.
	QIcon ctrlIcon;
	static const int iconSizes[5] = {64, 48, 32, 22, 16};
	for (size_t i = 0; i < NUM_ELEMENTS(iconSizes); i++) {
		QString iconFilename = QLatin1String(":/gens/") +
			QString::number(iconSizes[i]) + QChar(L'x') +
			QString::number(iconSizes[i]) +
			QLatin1String("/controllers/") +
			QLatin1String(CtrlIconFilenames[ioType]);
		
		if (QFile::exists(iconFilename)) {
			// File exists.
			ctrlIcon.addFile(iconFilename, QSize(iconSizes[i], iconSizes[i]));
		}
	}
	
	// Return the controller icon.
	return ctrlIcon;
}


/*******************************
 * CtrlConfigWindow functions. *
 *******************************/

/**
 * Initialize the Controller Configuration window.
 */
CtrlConfigWindow::CtrlConfigWindow(QWidget *parent)
	: QMainWindow(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
	, d(new CtrlConfigWindowPrivate(this))
{
	// Initialize the Qt4 UI.
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// "OK" and "Cancel" are automatically connected by uic.
	// Connect a signal for the "Apply" button.
	QPushButton *btnApply = buttonBox->button(QDialogButtonBox::Apply);
	connect(btnApply, SIGNAL(clicked(bool)), this, SLOT(apply()));

	// Initialize the toolbar.
	d->actgrpSelPort->setExclusive(true);

	// Base ports.
	d->actgrpSelPort->addAction(actionPort1);
	d->actgrpSelPort->addAction(actionPort2);
	d->actgrpSelPort->addAction(actionPortEXT);

	// TeamPlayer 1.
	d->actgrpSelPort->addAction(actionPortTP1A);
	d->actgrpSelPort->addAction(actionPortTP1B);
	d->actgrpSelPort->addAction(actionPortTP1C);
	d->actgrpSelPort->addAction(actionPortTP1D);

	// TeamPlayer 2.
	d->actgrpSelPort->addAction(actionPortTP2A);
	d->actgrpSelPort->addAction(actionPortTP2B);
	d->actgrpSelPort->addAction(actionPortTP2C);
	d->actgrpSelPort->addAction(actionPortTP2D);

	// EA 4-Way Play.
	d->actgrpSelPort->addAction(actionPort4WPA);
	d->actgrpSelPort->addAction(actionPort4WPB);
	d->actgrpSelPort->addAction(actionPort4WPC);
	d->actgrpSelPort->addAction(actionPort4WPD);

	// Find the toolbar separators.
	// Qt Designer doesn't save them for some reason.
	// Also, add port buttons to the signal mapper.
	int portNum = 0;
	d->vecTbSep.reserve(CTRL_CFG_TBSEP_MAX);
	foreach(QAction *action, toolBar->actions()) {
		if (action->isSeparator()) {
			// Append to the vector of separators.
			d->vecTbSep.append(action);
		} else {
			// Port button. Add to signal mapper.
			QObject::connect(action, SIGNAL(toggled(bool)),
					  d->mapperSelPort, SLOT(map()));
			d->mapperSelPort->setMapping(action, portNum);
			portNum++;
		}
	}

	// Initialize the "Device" dropdown.
	// 4WP Master is added here.
	// on_cboDevice_currentIndexChanged() handles translation for Port 1.
	d->cboDevice_lock();
	for (int ioType = IoManager::IOT_NONE;
	     ioType <= IoManager::IOT_4WP_MASTER; ioType++)
	{
		cboDevice->addItem(d->getCtrlIcon((IoManager::IoType_t)ioType),
				d->getShortDeviceName((IoManager::IoType_t)ioType));
	}
	d->cboDevice_unlock();

	// Load the controller configuration settings.
	reload();

	// Set Port 1 as active.
	actionPort1->setChecked(true);
}


/**
 * Shut down the Controller Configuration window.
 */
CtrlConfigWindow::~CtrlConfigWindow()
{
	delete d;
}


/**
 * Show a single instance of the Controller Configuration window.
 * @param parent Parent window.
 */
void CtrlConfigWindow::ShowSingle(QWidget *parent)
{
	if (CtrlConfigWindowPrivate::ms_Window != NULL) {
		// Controller Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(CtrlConfigWindowPrivate::ms_Window);
	} else {
		// Controller Configuration Window is not displayed.
		// NOTE: CtrlConfigWindowPrivate's constructor sets ms_Window.
		(new CtrlConfigWindow(parent))->show();
	}
}


/**
 * Key press handler.
 * @param event Key event.
 */
void CtrlConfigWindow::keyPressEvent(QKeyEvent *event)
{
	// TODO: Handle Cmd-Period on Mac?
	// NOTE: Cmd-W triggers the "Close ROM" action...
	
	// Check for special dialog keys.
	// Adapted from QDialog::keyPressEvent().
	if (!event->modifiers() || ((event->modifiers() & Qt::KeypadModifier)
	    && event->key() == Qt::Key_Enter))
	{
		switch (event->key())
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:
				// Accept the dialog changes.
				accept();
				break;
			
			case Qt::Key_Escape:
				// Reject the dialog changes.
				reject();
				break;
			
			default:
				// Pass the event to the base class.
				this->QMainWindow::keyPressEvent(event);
				return;
		}
	} else {
		// Pass the event to the base class.
		this->QMainWindow::keyPressEvent(event);
	}
}


/**
 * Widget state has changed.
 * @param event State change event.
 */
void CtrlConfigWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		retranslateUi(this);

		// Update the port buttons.
		for (int virtPort = IoManager::VIRTPORT_1;
		     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
			updatePortButton((IoManager::VirtPort_t)virtPort);
		}

		// Update the selected port information.
		updatePortSettings(d->selPort);
	}

	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
}


/**
 * Update a port button.
 * @param virtPort Virtual port number.
 */
void CtrlConfigWindow::updatePortButton(IoManager::VirtPort_t virtPort)
{
	QAction *actionPort;
	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:		actionPort = actionPort1; break;
		case IoManager::VIRTPORT_2:		actionPort = actionPort2; break;

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:		actionPort = actionPortTP1A; break;
		case IoManager::VIRTPORT_TP1B:		actionPort = actionPortTP1B; break;
		case IoManager::VIRTPORT_TP1C:		actionPort = actionPortTP1C; break;
		case IoManager::VIRTPORT_TP1D:		actionPort = actionPortTP1D; break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:		actionPort = actionPortTP2A; break;
		case IoManager::VIRTPORT_TP2B:		actionPort = actionPortTP2B; break;
		case IoManager::VIRTPORT_TP2C:		actionPort = actionPortTP2C; break;
		case IoManager::VIRTPORT_TP2D:		actionPort = actionPortTP2D; break;

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:		actionPort = actionPort4WPA; break;
		case IoManager::VIRTPORT_4WPB:		actionPort = actionPort4WPB; break;
		case IoManager::VIRTPORT_4WPC:		actionPort = actionPort4WPC; break;
		case IoManager::VIRTPORT_4WPD:		actionPort = actionPort4WPD; break;

		default:
			// Unknown port.
			return;
	}

	// Make sure the device type is in bounds.
	const IoManager::IoType_t ioType = d->ctrlConfig->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);

	// Update the port icon and tooltip.
	actionPort->setIcon(d->getCtrlIcon(ioType));
	actionPort->setToolTip(d->getPortName(virtPort) +
				QLatin1String(": ") +
				d->getLongDeviceName(ioType));

	// Hide the EXT port for now.
	actionPortEXT->setVisible(false);

	if (virtPort == IoManager::VIRTPORT_1) {
		// Port 1. Update TeamPlayer 1 button state.
		const bool isTP = (ioType == IoManager::IOT_TEAMPLAYER);
		d->vecTbSep[CTRL_CFG_TBSEP_TP1]->setVisible(isTP);
		actionPortTP1A->setVisible(isTP);
		actionPortTP1B->setVisible(isTP);
		actionPortTP1C->setVisible(isTP);
		actionPortTP1D->setVisible(isTP);
	} else if (virtPort == IoManager::VIRTPORT_2) {
		// Port 2. Update TeamPlayer 2 button state.
		const bool isTP = (ioType == IoManager::IOT_TEAMPLAYER);
		d->vecTbSep[CTRL_CFG_TBSEP_TP2]->setVisible(isTP);
		actionPortTP2A->setVisible(isTP);
		actionPortTP2B->setVisible(isTP);
		actionPortTP2C->setVisible(isTP);
		actionPortTP2D->setVisible(isTP);
	}

	if (virtPort == IoManager::VIRTPORT_1 ||
	    virtPort == IoManager::VIRTPORT_2) {
		// Port 1 or 2. Update EA 4-Way Play button state.
		const bool is4WP =
			(d->ctrlConfig->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE &&
			 d->ctrlConfig->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER);

		d->vecTbSep[CTRL_CFG_TBSEP_4WP]->setVisible(is4WP);
		actionPort4WPA->setVisible(is4WP);
		actionPort4WPB->setVisible(is4WP);
		actionPort4WPC->setVisible(is4WP);
		actionPort4WPD->setVisible(is4WP);
	}
}


/**
 * Update port settings.
 * @param virtPort Virtual port to display.
 */
void CtrlConfigWindow::updatePortSettings(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	const int ioType = d->ctrlConfig->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);

	// Update the port settings.
	// TODO: Controller keymap.

	// Set the "Port Settings" text.
	// TODO: Port names for when e.g. EXT, J-Cart, etc. are added.
	grpPortSettings->setTitle(d->getPortName(virtPort));

	// Set the device type in the dropdown.
	int devIndex = ioType;
	if (devIndex >= IoManager::IOT_4WP_SLAVE)
		devIndex--;	// avoid having two 4WP devices in the dropdown
	cboDevice->setCurrentIndex(devIndex);

	// Set the device information in the GensCtrlCfgWidget.
	ctrlCfgWidget->setIoType((IoManager::IoType_t)devIndex);
	ctrlCfgWidget->setKeyMap(d->ctrlConfig->keyMap(virtPort));
}


/**
 * Select a port to display.
 * @param virtPort Virtual port to display.
 */
void CtrlConfigWindow::selectPort(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	// Check if this is a Team Player port.
	bool isTP = false;
	switch (virtPort) {
		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE &&
			    d->ctrlConfig->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER)
				isTP = true;
			break;

		default:
			// Other port.
			break;
	}

	// Device setting is valid.

	// Make sure the dropdown index is set properly to reduce flicker.
	d->cboDevice_lock();
	const int ioType = (int)d->ctrlConfig->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	if (ioType < cboDevice->count())
		cboDevice->setCurrentIndex(ioType);
	d->cboDevice_unlock();

	// Update the port settings.
	d->selPort = virtPort;
	cboDevice_setTP(isTP);		// Update Team Player device availability.
	updatePortSettings(virtPort);	// Update the port settings.
}


/**
 * Set cboDevice's Team Player device availability.
 * @param isTP If true, don't show devices that can't be connected to a TeamPlayer/4WP/etc device.
 */
void CtrlConfigWindow::cboDevice_setTP(bool isTP)
{
	const int devCount = (isTP ? IoManager::IOT_6BTN : IoManager::IOT_4WP_MASTER) + 1;
	if (cboDevice->count() == devCount)
		return;

	// Dropdown needs to be updated.
	d->cboDevice_lock();
	if (cboDevice->count() > devCount) {
		// Remove the extra items.
		for (int ioType = IoManager::IOT_4WP_MASTER;
		     ioType > IoManager::IOT_6BTN; ioType--) {
			cboDevice->removeItem(ioType);
		}
	} else {
		// Add the extra items.
		for (int ioType = IoManager::IOT_2BTN;
		     ioType <= IoManager::IOT_4WP_MASTER; ioType++) {
			cboDevice->addItem(d->getCtrlIcon((IoManager::IoType_t)ioType),
					d->getShortDeviceName((IoManager::IoType_t)ioType));
		}
	}
	d->cboDevice_unlock();
}


/** TODO **/

/**
 * Save the configuration.
 */
void CtrlConfigWindow::accept(void)
{
	apply();
	close();
}

/**
 * Discard the configuration changes.
 */
void CtrlConfigWindow::reject(void)
	{ close(); }

/**
 * Reload the configuration, discarding all changes.
 */
void CtrlConfigWindow::reload(void)
{
	// Copy the current controller settings.
	d->ctrlConfig->copyFrom(gqt4_cfg->m_ctrlConfig);

	// Initialize all of the port buttons.
	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++)
	{
		updatePortButton((IoManager::VirtPort_t)virtPort);
	}

	// Set to true if the port settings have been updated.
	bool hasUpdatedPortSettings = false;

	// TODO: If a TP or 4WP port was selected,
	// but is no longer available, switch to the base port.
	switch (d->selPort) {
		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			// Make sure port 1 is still Team Player.
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_TEAMPLAYER) {
				// Port 1 is no longer Team Player.
				// Switch to Port 1 instead of the TP1 port.
				actionPort1->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			// Make sure port 2 is still Team Player.
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_TEAMPLAYER) {
				// Port 2 is no longer Team Player.
				// Switch to Port 2 instead of the TP2 port.
				actionPort2->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			// Make sure port 1 is still 4WP slave,
			// and port 2 is still 4WP master.
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_4WP_SLAVE ||
			    d->ctrlConfig->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_4WP_MASTER) {
				// 4WP is no longer set. Switch to Port 1.
				actionPort1->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		default:
			break;
	}

	// If we didn't update the port settings due to an invalid TP/4WP setting,
	// update them now, since they may have changed due to the reload.
	if (!hasUpdatedPortSettings)
		updatePortSettings(d->selPort);
}

/**
 * Apply the configuration changes.
 */
void CtrlConfigWindow::apply(void)
{
	// Copy the controller configuration settings to gqt4_cfg.
	gqt4_cfg->m_ctrlConfig->copyFrom(d->ctrlConfig);

	// Update the I/O manager if emulation is running.
	// TODO: Send something to the emulation queue instead?
	if (gqt4_emuContext)
		gqt4_cfg->m_ctrlConfig->updateIoManager(gqt4_emuContext->m_ioManager);
}


/** Widget slots. **/


/**
 * The selected port in the toolbar has been changed.
 * @param virtPort Port number.
 */
void CtrlConfigWindow::toolbarPortSelected(int virtPort)
{
	QAction *action = qobject_cast<QAction*>(d->mapperSelPort->mapping(virtPort));
	if (action && action->isChecked())
		selectPort((IoManager::VirtPort_t)virtPort);
}


/**
 * Device type has been changed.
 * @param index Device type index.
 */
void CtrlConfigWindow::on_cboDevice_currentIndexChanged(int index)
{
	if (d->isCboDeviceLocked())
		return;

	const IoManager::VirtPort_t virtPort = d->selPort;
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	// Check if the device type has been changed.
	if (index < 0)
		return;
	const IoManager::IoType_t ioType = d->ctrlConfig->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	if (ioType == index)
		return;

	// Check for 4WP.
	if (d->selPort == IoManager::VIRTPORT_1) {
		// Port 1.
		if (index == IoManager::IOT_4WP_MASTER) {
			// 4WP SLAVE set for Port 1.
			// (4WP_MASTER index used for dropdown.)
			// TODO: Maybe use the combo box item's data parameter?
			d->ctrlConfig->setIoType(d->selPort, IoManager::IOT_4WP_SLAVE);

			// Make sure Port 2 is set to 4WP MASTER.
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_4WP_MASTER) {
				d->ctrlConfig->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_4WP_MASTER);
				updatePortButton(IoManager::VIRTPORT_2);
			}
		} else {
			// 4WP SLAVE not set for Port 1.
			d->ctrlConfig->setIoType(d->selPort, (IoManager::IoType_t)index);
			
			// Check if Port 2 is set to 4WP MASTER. (or 4WP SLAVE for sanity check)
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER ||
			    d->ctrlConfig->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_SLAVE)
			{
				// Port 2 is set to 4WP MASTER. Unset it.
				d->ctrlConfig->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
				updatePortButton(IoManager::VIRTPORT_2);
			}
		}
	} else if (d->selPort == IoManager::VIRTPORT_2) {
		// Port 2.
		if (index == IoManager::IOT_4WP_MASTER) {
			// 4WP MASTER set for Port 2.
			d->ctrlConfig->setIoType(d->selPort, IoManager::IOT_4WP_MASTER);
			
			// Make sure Port 1 is set to 4WP SLAVE.
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_4WP_SLAVE) {
				d->ctrlConfig->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_4WP_SLAVE);
				updatePortButton(IoManager::VIRTPORT_1);
			}
		} else {
			// 4WP MASTER not set for Port 2.
			d->ctrlConfig->setIoType(d->selPort, (IoManager::IoType_t)index);
			
			// Check if Port 1 is set to 4WP SLAVE. (or 4WP MASTER for sanity check)
			if (d->ctrlConfig->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE ||
			    d->ctrlConfig->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_MASTER)
			{
				// Port 1 is set to 4WP SLAVE. Unset it.
				d->ctrlConfig->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_NONE);
				updatePortButton(IoManager::VIRTPORT_1);
			}
		}
	} else {
		// Other port.
		d->ctrlConfig->setIoType(d->selPort, (IoManager::IoType_t)index);
	}

	// Update the port information.
	updatePortButton(d->selPort);
	updatePortSettings(d->selPort);
}

/**
 * A key's configuration has been changed.
 * @param idx Button index.
 * @param gensKey New GensKey_t value.
 */
void CtrlConfigWindow::on_ctrlCfgWidget_keyChanged(int idx, GensKey_t gensKey)
{
	// TODO: Only save the specific key that was changed.
	// For now, we're going to save everything.
	printf("KEY CHANGED: idx == %d, gensKey == %08X\n", idx, gensKey);
	d->ctrlConfig->setKeyMap(d->selPort, ctrlCfgWidget->keyMap());
}

}
