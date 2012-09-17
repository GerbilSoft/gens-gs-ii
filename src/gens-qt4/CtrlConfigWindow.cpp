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

// EmuMD has the I/O devices.
#include "libgens/MD/EmuMD.hpp"

// Qt includes.
#include <QtGui/QActionGroup>
#include <QtGui/QKeyEvent>
#include <QtCore/QFile>

namespace GensQt4
{

// Static member initialization.
CtrlConfigWindow *CtrlConfigWindow::m_CtrlConfigWindow = NULL;

// Controller icon filenames.
const char *const CtrlConfigWindow::ms_CtrlIconFilenames[LibGens::IoManager::IOT_MAX] =
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


/**
 * CtrlConfigWindow(): Initialize the Controller Configuration window.
 */
CtrlConfigWindow::CtrlConfigWindow(QWidget *parent)
	: QMainWindow(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
{
	// Initialize the Qt4 UI.
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Initialize the cboDevice lock counter.
	m_cboDeviceLockCnt = 0;

	// Initialize the toolbar.
	m_actgrpSelPort = new QActionGroup(this);
	m_actgrpSelPort->setExclusive(true);

	// Base ports.
	m_actgrpSelPort->addAction(actionPort1);
	m_actgrpSelPort->addAction(actionPort2);
	m_actgrpSelPort->addAction(actionPortEXT);

	// TeamPlayer 1.
	m_actgrpSelPort->addAction(actionPortTP1A);
	m_actgrpSelPort->addAction(actionPortTP1B);
	m_actgrpSelPort->addAction(actionPortTP1C);
	m_actgrpSelPort->addAction(actionPortTP1D);

	// TeamPlayer 2.
	m_actgrpSelPort->addAction(actionPortTP2A);
	m_actgrpSelPort->addAction(actionPortTP2B);
	m_actgrpSelPort->addAction(actionPortTP2C);
	m_actgrpSelPort->addAction(actionPortTP2D);

	// EA 4-Way Play.
	m_actgrpSelPort->addAction(actionPort4WPA);
	m_actgrpSelPort->addAction(actionPort4WPB);
	m_actgrpSelPort->addAction(actionPort4WPC);
	m_actgrpSelPort->addAction(actionPort4WPD);

	// Signal mapper for toolbar buttons.
	m_mapperSelPort = new QSignalMapper(this);
	QObject::connect(m_mapperSelPort, SIGNAL(mapped(int)),
			 this, SLOT(toolbarPortSelected(int)));

	// Find the toolbar separators.
	// Qt Designer doesn't save them for some reason.
	// Also, add port buttons to the signal mapper.
	int portNum = 0;
	m_vecTbSep.reserve(CTRL_CFG_TBSEP_MAX);
	foreach(QAction *action, toolBar->actions())
	{
		if (action->isSeparator())
		{
			// Append to the vector of separators.
			m_vecTbSep.append(action);
		}
		else
		{
			// Port button. Add to signal mapper.
			QObject::connect(action, SIGNAL(toggled(bool)),
					 m_mapperSelPort, SLOT(map()));
			m_mapperSelPort->setMapping(action, portNum);
			portNum++;
		}
	}
	
	// Initialize the "Device" dropdown.
	// 4WP Master is added here.
	// on_cboDevice_currentIndexChanged() handles translation for Port 1.
	cboDevice_lock();
	for (int ioType = LibGens::IoManager::IOT_NONE;
	     ioType <= LibGens::IoManager::IOT_4WP_MASTER; ioType++) {
		cboDevice->addItem(GetCtrlIcon((LibGens::IoManager::IoType_t)ioType),
				GetShortDeviceName((LibGens::IoManager::IoType_t)ioType));
	}
	cboDevice_unlock();

	// Clear controller settings.
	memset(m_devType, 0x00, sizeof(m_devType));

	// Copy the current controller settings.
	// TODO: Button mapping.
	// TODO: Load from the configuration cache instead of the emulation context.
	// TODO: TeamPlayer / EA 4-Way Play support.
	if (gqt4_emuContext) {
		// Emulation is running.
		m_devType[0] = gqt4_emuContext->m_ioManager->devType(LibGens::IoManager::VIRTPORT_1);
		m_devType[1] = gqt4_emuContext->m_ioManager->devType(LibGens::IoManager::VIRTPORT_2);
	} else {
		// Emulation is not running.
		m_devType[0] = LibGens::IoManager::IOT_NONE;
		m_devType[1] = LibGens::IoManager::IOT_NONE;
	}

	// Initialize all of the port buttons.
	for (int virtPort = LibGens::IoManager::VIRTPORT_1;
	     virtPort < LibGens::IoManager::VIRTPORT_MAX; virtPort++) {
		updatePortButton((LibGens::IoManager::VirtPort_t)virtPort);
	}

	// Set Port 1 as active.
	actionPort1->setChecked(true);
}


/**
 * ~CtrlConfigWindow(): Shut down the Controller Configuration window.
 */
CtrlConfigWindow::~CtrlConfigWindow()
{
	// Clear the m_CtrlConfigWindow pointer.
	m_CtrlConfigWindow = NULL;
}


/**
 * ShowSingle(): Show a single instance of the Controller Configuration window.
 * @param parent Parent window.
 */
void CtrlConfigWindow::ShowSingle(QWidget *parent)
{
	if (m_CtrlConfigWindow != NULL)
	{
		// Controller Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(m_CtrlConfigWindow);
	}
	else
	{
		// Controller Configuration Window is not displayed.
		m_CtrlConfigWindow = new CtrlConfigWindow(parent);
		m_CtrlConfigWindow->show();
	}
}


/**
 * keyPressEvent(): Key press handler.
 * @param event Key event.
 */
void CtrlConfigWindow::keyPressEvent(QKeyEvent *event)
{
	// TODO: Handle Cmd-Period on Mac?
	// NOTE: Cmd-W triggers the "Close ROM" action...
	
	// Check for special dialog keys.
	// Adapted from QDialog::keyPressEvent().
	if (!event->modifiers() || ((event->modifiers() & Qt::KeypadModifier) && event->key() == Qt::Key_Enter))
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
	}
	else
	{
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
		for (int virtPort = LibGens::IoManager::VIRTPORT_1;
		     virtPort < LibGens::IoManager::VIRTPORT_MAX; virtPort++) {
			updatePortButton((LibGens::IoManager::VirtPort_t)virtPort);
		}

		// Update the selected port information.
		updatePortSettings(m_selPort);
	}

	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
}


/**
 * Get the short name of an I/O device.
 * @param ioType Device type.
 * @return Short device name.
 */
QString CtrlConfigWindow::GetShortDeviceName(LibGens::IoManager::IoType_t ioType)
{
	using LibGens::IoManager;

	switch (ioType) {
		case IoManager::IOT_NONE:
		default:
			return tr("None");

		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return tr("3-button");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return tr("6-button");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return tr("2-button");
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return tr("Mega Mouse");
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return tr("Team Player");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return tr("4-Way Play");
	}
}


/**
 * Get the long name of an I/O device.
 * @param ioType Device type.
 * @return Long device name.
 */
QString CtrlConfigWindow::GetLongDeviceName(LibGens::IoManager::IoType_t ioType)
{
	using LibGens::IoManager;

	switch (ioType) {
		case IoManager::IOT_NONE:
		default:
			return tr("No device connected.");
		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return tr("3-button gamepad");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return tr("6-button gamepad");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return tr("2-button gamepad (SMS)");
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return tr("Mega Mouse");
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return tr("Sega Team Player");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return tr("EA 4-Way Play");
	}
}


/**
 * Get the name of a given port.
 * @param virtPort Virtual port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigWindow::GetPortName(LibGens::IoManager::VirtPort_t virtPort)
{
	using LibGens::IoManager;

	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:
		case IoManager::VIRTPORT_2:
			return tr("Port %1")
				.arg(virtPort - IoManager::VIRTPORT_1 + 1);

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			return tr("Team Player %1, Port %2").arg(1)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP1A)));

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			return tr("Team Player %1, Port %2").arg(2)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP2A)));

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			return tr("4-Way Play, Port %1")
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
QIcon CtrlConfigWindow::GetCtrlIcon(LibGens::IoManager::IoType_t ioType)
{
	if (ioType < LibGens::IoManager::IOT_NONE ||
	    ioType >= LibGens::IoManager::IOT_MAX) {
		// Invalid I/O type.
		return QIcon();
	}
	
	// Create the icon.
	QIcon ctrlIcon;
	static const int iconSizes[5] = {64, 48, 32, 22, 16};
	for (size_t i = 0; i < sizeof(iconSizes)/sizeof(iconSizes[0]); i++) {
		QString iconFilename = QLatin1String(":/gens/") +
			QString::number(iconSizes[i]) + QChar(L'x') +
			QString::number(iconSizes[i]) +
			QLatin1String("/controllers/") +
			QLatin1String(ms_CtrlIconFilenames[ioType]);
		
		if (QFile::exists(iconFilename)) {
			// File exists.
			ctrlIcon.addFile(iconFilename, QSize(iconSizes[i], iconSizes[i]));
		}
	}
	
	// Return the controller icon.
	return ctrlIcon;
}


/**
 * updatePortButton(): Update a port button.
 * @param virtPort Virtual port number.
 */
void CtrlConfigWindow::updatePortButton(LibGens::IoManager::VirtPort_t virtPort)
{
	using LibGens::IoManager;

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
	if (m_devType[virtPort] < IoManager::IOT_NONE ||
	    m_devType[virtPort] >= IoManager::IOT_MAX)
	{
		// Invalid device type.
		return;
	}

	// Update the port icon and tooltip.
	actionPort->setIcon(GetCtrlIcon(m_devType[virtPort]));
	actionPort->setToolTip(GetPortName(virtPort) +
				QLatin1String(": ") +
				GetLongDeviceName(m_devType[virtPort]));

	// Disable the EXT port for now.
	actionPortEXT->setVisible(false);

	if (virtPort == LibGens::IoManager::VIRTPORT_1) {
		// Port 1. Update TeamPlayer 1 button state.
		const bool isTP = (m_devType[virtPort] == IoManager::IOT_TEAMPLAYER);
		m_vecTbSep[CTRL_CFG_TBSEP_TP1]->setVisible(isTP);
		actionPortTP1A->setVisible(isTP);
		actionPortTP1B->setVisible(isTP);
		actionPortTP1C->setVisible(isTP);
		actionPortTP1D->setVisible(isTP);
	} else if (virtPort == LibGens::IoManager::VIRTPORT_2) {
		// Port 2. Update TeamPlayer 2 button state.
		const bool isTP = (m_devType[virtPort] == IoManager::IOT_TEAMPLAYER);
		m_vecTbSep[CTRL_CFG_TBSEP_TP2]->setVisible(isTP);
		actionPortTP2A->setVisible(isTP);
		actionPortTP2B->setVisible(isTP);
		actionPortTP2C->setVisible(isTP);
		actionPortTP2D->setVisible(isTP);
	}

	if (virtPort == IoManager::VIRTPORT_1 ||
	    virtPort == IoManager::VIRTPORT_2) {
		// Port 1 or 2. Update EA 4-Way Play button state.
		const bool is4WP =
			(m_devType[IoManager::VIRTPORT_1] == IoManager::IOT_4WP_SLAVE &&
			 m_devType[IoManager::VIRTPORT_2] == IoManager::IOT_4WP_MASTER);

		m_vecTbSep[CTRL_CFG_TBSEP_4WP]->setVisible(is4WP);
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
void CtrlConfigWindow::updatePortSettings(LibGens::IoManager::VirtPort_t virtPort)
{
	if (virtPort < LibGens::IoManager::VIRTPORT_1 ||
	    virtPort >= LibGens::IoManager::VIRTPORT_MAX)
		return;

	// Update the port settings.
	// TODO: Controller keymap.

	// Make sure the device type is in bounds.
	if (m_devType[virtPort] < LibGens::IoManager::IOT_NONE ||
	    m_devType[virtPort] >= LibGens::IoManager::IOT_MAX)
	{
		// Invalid device type.
		return;
	}

	// Set the "Port Settings" text.
	// TODO: Port names for when e.g. EXT, J-Cart, etc. are added.
	grpPortSettings->setTitle(GetPortName(virtPort));

	// Set the device type in the dropdown.
	int devIndex = m_devType[virtPort];
	if (devIndex >= LibGens::IoManager::IOT_4WP_SLAVE)
		devIndex--;	// avoid having two 4WP devices in the dropdown
	cboDevice->setCurrentIndex(devIndex);

	// Set the device type in the CtrlCfgWidget.
	// TODO: Load the configuration, and save the previous configuration.
	ctrlCfgWidget->setIoType((LibGens::IoManager::IoType_t)devIndex);
}


/**
 * Select a port to display.
 * @param virtPort Virtual port to display.
 */
void CtrlConfigWindow::selectPort(LibGens::IoManager::VirtPort_t virtPort)
{
	using LibGens::IoManager;

	if (virtPort < IoManager::VIRTPORT_1 ||
	    virtPort >= IoManager::VIRTPORT_MAX)
		return;

	// Check if this is a Team Player port.
	bool isTP = false;
	switch (virtPort) {
		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			if (m_devType[IoManager::VIRTPORT_1] == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			if (m_devType[IoManager::VIRTPORT_2] == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			if (m_devType[IoManager::VIRTPORT_1] == IoManager::IOT_4WP_SLAVE &&
			    m_devType[IoManager::VIRTPORT_2] == IoManager::IOT_4WP_MASTER)
				isTP = true;
			break;

		default:
			// Other port.
			break;
	}

	// Device setting is valid.

	// Make sure the dropdown index is set properly to reduce flicker.
	cboDevice_lock();
	const int idx = (int)m_devType[virtPort];
	if (idx < cboDevice->count())
		cboDevice->setCurrentIndex(idx);
	cboDevice_unlock();

	// Update the port settings.
	m_selPort = virtPort;
	cboDevice_setTP(isTP);		// Update Team Player device availability.
	updatePortSettings(virtPort);	// Update the port settings.
}


/**
 * Set cboDevice's Team Player device availability.
 * @param isTP If true, don't show devices that can't be connected to a TeamPlayer/4WP/etc device.
 */
void CtrlConfigWindow::cboDevice_setTP(bool isTP)
{
	const int devCount = (isTP ? LibGens::IoManager::IOT_6BTN : LibGens::IoManager::IOT_4WP_MASTER) + 1;
	if (cboDevice->count() == devCount)
		return;

	// Dropdown needs to be updated.
	cboDevice_lock();
	if (cboDevice->count() > devCount) {
		// Remove the extra items.
		for (int ioType = LibGens::IoManager::IOT_4WP_MASTER;
		     ioType > LibGens::IoManager::IOT_6BTN; ioType--) {
			cboDevice->removeItem(ioType);
		}
	} else {
		// Add the extra items.
		for (int ioType = LibGens::IoManager::IOT_2BTN;
		     ioType <= LibGens::IoManager::IOT_4WP_MASTER; ioType++) {
			cboDevice->addItem(GetCtrlIcon((LibGens::IoManager::IoType_t)ioType),
					GetShortDeviceName((LibGens::IoManager::IoType_t)ioType));
		}
	}
	cboDevice_unlock();
}


/**
 * cboDevice_lock() / cboDevice_unlock(): Lock or unlock cboDevice.
 * This turns off signal handling for cboDevice.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfigWindow::cboDevice_lock(void)
	{ m_cboDeviceLockCnt++; return 0; }

int CtrlConfigWindow::cboDevice_unlock(void)
{
	assert(m_cboDeviceLockCnt >= 0);
	if (m_cboDeviceLockCnt <= 0)
		return -1;
	
	m_cboDeviceLockCnt--;
	return 0;
}


/** TODO **/

void CtrlConfigWindow::accept(void)
	{ close(); }
void CtrlConfigWindow::reject(void)
	{ close(); }

void CtrlConfigWindow::reload(void) { }
void CtrlConfigWindow::apply(void) { }


/** Widget slots. **/


/**
 * The selected port in the toolbar has been changed.
 * @param virtPort Port number.
 */
void CtrlConfigWindow::toolbarPortSelected(int virtPort)
{
	QAction *action = qobject_cast<QAction*>(m_mapperSelPort->mapping(virtPort));
	if (action && action->isChecked())
		selectPort((LibGens::IoManager::VirtPort_t)virtPort);
}


/**
 * Device type has been changed.
 * @param index Device type index.
 */
void CtrlConfigWindow::on_cboDevice_currentIndexChanged(int index)
{
	if (isCboDeviceLocked())
		return;

	if (m_selPort < LibGens::IoManager::VIRTPORT_1 ||
	    m_selPort >= LibGens::IoManager::VIRTPORT_MAX)
		return;

	// Check if the device type has been changed.
	if (index < 0)
		return;
	if (m_devType[m_selPort] == index)
		return;
	if (m_devType[m_selPort] < LibGens::IoManager::IOT_NONE ||
	    m_devType[m_selPort] >= LibGens::IoManager::IOT_MAX)
	{
		// Invalid device type.
		return;
	}

	// Check for 4WP.
	if (m_selPort == LibGens::IoManager::VIRTPORT_1) {
		// Port 1.
		if (index == LibGens::IoManager::IOT_4WP_MASTER) {
			// 4WP SLAVE set for Port 1.
			// (4WP_MASTER index used for dropdown.)
			// TODO: Maybe use the combo box item's data parameter?
			m_devType[m_selPort] = LibGens::IoManager::IOT_4WP_SLAVE;

			// Make sure Port 2 is set to 4WP MASTER.
			if (m_devType[LibGens::IoManager::VIRTPORT_2] != LibGens::IoManager::IOT_4WP_MASTER) {
				m_devType[LibGens::IoManager::VIRTPORT_2] = LibGens::IoManager::IOT_4WP_MASTER;
				updatePortButton(LibGens::IoManager::VIRTPORT_2);
			}
		} else {
			// 4WP SLAVE not set for Port 1.
			m_devType[m_selPort] = (LibGens::IoManager::IoType_t)index;
			
			// Check if Port 2 is set to 4WP MASTER. (or 4WP SLAVE for sanity check)
			if (m_devType[LibGens::IoManager::VIRTPORT_2] == LibGens::IoManager::IOT_4WP_MASTER ||
			    m_devType[LibGens::IoManager::VIRTPORT_2] == LibGens::IoManager::IOT_4WP_SLAVE)
			{
				// Port 2 is set to 4WP MASTER. Unset it.
				m_devType[LibGens::IoManager::VIRTPORT_2] = LibGens::IoManager::IOT_NONE;
				updatePortButton(LibGens::IoManager::VIRTPORT_2);
			}
		}
	} else if (m_selPort == LibGens::IoManager::VIRTPORT_2) {
		// Port 2.
		if (index == LibGens::IoManager::IOT_4WP_MASTER) {
			// 4WP MASTER set for Port 2.
			m_devType[m_selPort] = LibGens::IoManager::IOT_4WP_MASTER;
			
			// Make sure Port 1 is set to 4WP SLAVE.
			if (m_devType[LibGens::IoManager::VIRTPORT_1] != LibGens::IoManager::IOT_4WP_SLAVE) {
				m_devType[LibGens::IoManager::VIRTPORT_1] = LibGens::IoManager::IOT_4WP_SLAVE;
				updatePortButton(LibGens::IoManager::VIRTPORT_1);
			}
		} else {
			// 4WP MASTER not set for Port 2.
			m_devType[m_selPort] = (LibGens::IoManager::IoType_t)index;
			
			// Check if Port 1 is set to 4WP SLAVE. (or 4WP MASTER for sanity check)
			if (m_devType[LibGens::IoManager::VIRTPORT_1] == LibGens::IoManager::IOT_4WP_SLAVE ||
			    m_devType[LibGens::IoManager::VIRTPORT_1] == LibGens::IoManager::IOT_4WP_MASTER)
			{
				// Port 1 is set to 4WP SLAVE. Unset it.
				m_devType[LibGens::IoManager::VIRTPORT_1] = LibGens::IoManager::IOT_NONE;
				updatePortButton(LibGens::IoManager::VIRTPORT_1);
			}
		}
	} else {
		// Other port.
		m_devType[m_selPort] = (LibGens::IoManager::IoType_t)index;
	}

	// Update the port information.
	updatePortButton(m_selPort);
	updatePortSettings(m_selPort);
}

}
