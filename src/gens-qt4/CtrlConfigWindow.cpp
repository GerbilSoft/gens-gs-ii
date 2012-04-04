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
const char *const CtrlConfigWindow::ms_CtrlIconFilenames[LibGens::IoBase::IOT_MAX] =
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
	size_t tbSepCnt = 0;
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
	for (int i = LibGens::IoBase::IOT_NONE; i <= LibGens::IoBase::IOT_4WP_MASTER; i++)
	{
		cboDevice->addItem(GetCtrlIcon((LibGens::IoBase::IoType)i),
				GetShortDeviceName((LibGens::IoBase::IoType)i));
	}
	cboDevice_unlock();
	
	// Clear controller settings.
	memset(m_devType, 0x00, sizeof(m_devType));
	
	// Copy the current controller settings.
	// TODO: Button mapping.
	// TODO: Load from the configuration cache instead of the emulation context.
	// TODO: TeamPlayer / EA 4-Way Play support.
	if (gqt4_emuContext)
	{
		// Emulation is running.
		m_devType[0] = gqt4_emuContext->m_port1->devType();
		m_devType[1] = gqt4_emuContext->m_port2->devType();
	}
	else
	{
		// Emulation is not running.
		m_devType[0] = LibGens::IoBase::IOT_NONE;
		m_devType[1] = LibGens::IoBase::IOT_NONE;
	}
	
	// Initialize all the port buttons.
	for (int i = 0; i < CtrlConfig::PORT_MAX; i++)
		updatePortButton(i);
	
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
 * changeEvent(): Widget state has changed.
 * @param event State change event.
 */
void CtrlConfigWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		// Retranslate the UI.
		retranslateUi(this);
		
		// Update the port buttons.
		for (int i = 0; i < CtrlConfig::PORT_MAX; i++)
			updatePortButton(i);
		
		// Update the selected port information.
		updatePortSettings(m_selPort);
	}
	
	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
}


/**
 * GetShortDeviceName(): Get the short name of an I/O device.
 * @param devType Device type.
 * @return Short device name.
 */
QString CtrlConfigWindow::GetShortDeviceName(LibGens::IoBase::IoType devType)
{
	using LibGens::IoBase;
	
	switch (devType)
	{
		case IoBase::IOT_NONE:
		default:
			return tr("None");
		
		case IoBase::IOT_3BTN:
			//: Standard 3-button control pad.
			return tr("3-button");
		case IoBase::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return tr("6-button");
		case IoBase::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			// return tr("2-button");
		case IoBase::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return tr("Mega Mouse");
		case IoBase::IOT_TEAMPLAYER:
			//: Sega Team Player.
			//: NOTE: This is a specific brand name.
			//: Only change it if the Sega Team Player was released
			//: using a different name in your region,
			//: e.g. "Multiplayer" in Europe.
			return tr("Team Player");
		case IoBase::IOT_4WP_MASTER:	/* fallthrough */
		case IoBase::IOT_4WP_SLAVE:
			//: EA 4-Way Play.
			//: NOTE: This is a specific brand name.
			//: Only change it if the EA 4-Way Play was released
			//: using a different name in your region.
			return tr("4-Way Play");
	}
}


/**
 * GetLongDeviceName(): Get the long name of an I/O device.
 * @param devType Device type.
 * @return Long device name.
 */
QString CtrlConfigWindow::GetLongDeviceName(LibGens::IoBase::IoType devType)
{
	using LibGens::IoBase;
	
	switch (devType)
	{
		case IoBase::IOT_NONE:
		default:
			return tr("No device connected.");
		
		case IoBase::IOT_3BTN:		return tr("3-button gamepad");
		case IoBase::IOT_6BTN:		return tr("6-button gamepad");
		case IoBase::IOT_2BTN:		return tr("2-button gamepad (SMS)");
		case IoBase::IOT_MEGA_MOUSE:	return tr("Mega Mouse");
		case IoBase::IOT_TEAMPLAYER:	return tr("Sega Team Player");
		case IoBase::IOT_4WP_MASTER:	/* see below */
		case IoBase::IOT_4WP_SLAVE:	return tr("EA 4-Way Play");
	}
}


/**
 * GetPortName(): Get the name of a given port.
 * @param port Port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigWindow::GetPortName(int port)
{
	switch (port)
	{
		// System controller ports.
		case CtrlConfig::PORT_1:
		case CtrlConfig::PORT_2:
			return tr("Port %1").arg((port - CtrlConfig::PORT_1) + 1);
		
		// Team Player, Port 1.
		case CtrlConfig::PORT_TP1A:
		case CtrlConfig::PORT_TP1B:
		case CtrlConfig::PORT_TP1C:
		case CtrlConfig::PORT_TP1D:
			return tr("Team Player %1, Port %2").arg(1)
				.arg(QChar(L'A' + (port - CtrlConfig::PORT_TP1A)));
		
		// Team Player, Port 2.
		case CtrlConfig::PORT_TP2A:
		case CtrlConfig::PORT_TP2B:
		case CtrlConfig::PORT_TP2C:
		case CtrlConfig::PORT_TP2D:
			return tr("Team Player %1, Port %2").arg(2)
				.arg(QChar(L'A' + (port - CtrlConfig::PORT_TP2A)));
		
		// 4-Way Play.
		case CtrlConfig::PORT_4WPA:
		case CtrlConfig::PORT_4WPB:
		case CtrlConfig::PORT_4WPC:
		case CtrlConfig::PORT_4WPD:
			return tr("4-Way Play, Port %1")
				.arg(QChar(L'A' + (port - CtrlConfig::PORT_4WPA)));
		
		default:
			// Unknown port number.
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * GetCtrlIcon(): Get an icon for the specified controller type.
 * @param ioType Controller type.
 * @return Icon for the specified controller type.
 */
QIcon CtrlConfigWindow::GetCtrlIcon(LibGens::IoBase::IoType ioType)
{
	if (ioType < LibGens::IoBase::IOT_NONE ||
	    ioType >= LibGens::IoBase::IOT_MAX)
	{
		// Invalid I/O type.
		return QIcon();
	}
	
	// Create the icon.
	QIcon ctrlIcon;
	static const int iconSizes[5] = {64, 48, 32, 22, 16};
	for (size_t i = 0; i < sizeof(iconSizes)/sizeof(iconSizes[0]); i++)
	{
		QString iconFilename = QLatin1String(":/gens/") +
			QString::number(iconSizes[i]) + QChar(L'x') +
			QString::number(iconSizes[i]) +
			QLatin1String("/controllers/") +
			QLatin1String(ms_CtrlIconFilenames[ioType]);
		
		if (QFile::exists(iconFilename))
		{
			// File exists.
			ctrlIcon.addFile(iconFilename, QSize(iconSizes[i], iconSizes[i]));
		}
	}
	
	// Return the controller icon.
	return ctrlIcon;
}


/**
 * updatePortButton(): Update a port button.
 * @param port Port ID.
 */
void CtrlConfigWindow::updatePortButton(int port)
{
	QAction *actionPort;
	switch (port)
	{
		// System controller ports.
		case CtrlConfig::PORT_1:	actionPort = actionPort1; break;
		case CtrlConfig::PORT_2:	actionPort = actionPort2; break;
		
		// Team Player, Port 1.
		case CtrlConfig::PORT_TP1A:	actionPort = actionPortTP1A; break;
		case CtrlConfig::PORT_TP1B:	actionPort = actionPortTP1B; break;
		case CtrlConfig::PORT_TP1C:	actionPort = actionPortTP1C; break;
		case CtrlConfig::PORT_TP1D:	actionPort = actionPortTP1D; break;

		// Team Player, Port 2.
		case CtrlConfig::PORT_TP2A:	actionPort = actionPortTP2A; break;
		case CtrlConfig::PORT_TP2B:	actionPort = actionPortTP2B; break;
		case CtrlConfig::PORT_TP2C:	actionPort = actionPortTP2C; break;
		case CtrlConfig::PORT_TP2D:	actionPort = actionPortTP2D; break;
		
		// 4-Way Play.
		case CtrlConfig::PORT_4WPA:	actionPort = actionPort4WPA; break;
		case CtrlConfig::PORT_4WPB:	actionPort = actionPort4WPB; break;
		case CtrlConfig::PORT_4WPC:	actionPort = actionPort4WPC; break;
		case CtrlConfig::PORT_4WPD:	actionPort = actionPort4WPD; break;
		
		default:
			// Unknown port.
			return;
	}
	
	// Make sure the device type is in bounds.
	if (m_devType[port] < LibGens::IoBase::IOT_NONE ||
	    m_devType[port] >= LibGens::IoBase::IOT_MAX)
	{
		// Invalid device type.
		return;
	}
	
	// Update the port icon and tooltip.
	actionPort->setIcon(GetCtrlIcon(m_devType[port]));
	actionPort->setToolTip(GetPortName(port) +
				QLatin1String(": ") +
				GetLongDeviceName(m_devType[port]));
	
	if (port == CtrlConfig::PORT_1)
	{
		// Port 1. Update TeamPlayer 1 button state.
		const bool isTP = (m_devType[port] == LibGens::IoBase::IOT_TEAMPLAYER);
		m_vecTbSep[CTRL_CFG_TBSEP_TP1]->setVisible(isTP);
		actionPortTP1A->setVisible(isTP);
		actionPortTP1B->setVisible(isTP);
		actionPortTP1C->setVisible(isTP);
		actionPortTP1D->setVisible(isTP);
	}
	else if (port == CtrlConfig::PORT_2)
	{
		// Port 2. Update TeamPlayer 2 button state.
		const bool isTP = (m_devType[port] == LibGens::IoBase::IOT_TEAMPLAYER);
		m_vecTbSep[CTRL_CFG_TBSEP_TP2]->setVisible(isTP);
		actionPortTP2A->setVisible(isTP);
		actionPortTP2B->setVisible(isTP);
		actionPortTP2C->setVisible(isTP);
		actionPortTP2D->setVisible(isTP);
	}
	
	if (port == CtrlConfig::PORT_1 || port == CtrlConfig::PORT_2)
	{
		// Port 1 or 2. Update EA 4-Way Play button state.
		const bool is4WP = (m_devType[0] == LibGens::IoBase::IOT_4WP_SLAVE &&
				    m_devType[1] == LibGens::IoBase::IOT_4WP_MASTER);
		
		m_vecTbSep[CTRL_CFG_TBSEP_4WP]->setVisible(is4WP);
		actionPort4WPA->setVisible(is4WP);
		actionPort4WPB->setVisible(is4WP);
		actionPort4WPC->setVisible(is4WP);
		actionPort4WPD->setVisible(is4WP);
	}
}


/**
 * updatePortSettings(): Update port settings.
 * @param port Port to display.
 */
void CtrlConfigWindow::updatePortSettings(int port)
{
	if (port < CtrlConfig::PORT_1 || port >= CtrlConfig::PORT_MAX)
		return;
	
	// Update the port settings.
	// TODO: Controller keymap.
	
	// Make sure the device type is in bounds.
	if (m_devType[port] < LibGens::IoBase::IOT_NONE ||
	    m_devType[port] >= LibGens::IoBase::IOT_MAX)
	{
		// Invalid device type.
		return;
	}
	
	// Set the "Port Settings" text.
	// TODO: Port names for when e.g. EXT, J-Cart, etc. are added.
	grpPortSettings->setTitle(GetPortName(port));
	
	// Set the device type in the dropdown.
	int devIndex = m_devType[port];
	if (devIndex >= LibGens::IoBase::IOT_4WP_SLAVE)
		devIndex--;	// avoid having two 4WP devices in the dropdown
	cboDevice->setCurrentIndex(devIndex);
	
	// Set the device type in the CtrlCfgWidget.
	// TODO: Load the configuration, and save the previous configuration.
	ctrlCfgWidget->setIoType((LibGens::IoBase::IoType)devIndex);
}


void CtrlConfigWindow::selectPort(int port)
{
	if (port < CtrlConfig::PORT_1 || port >= CtrlConfig::PORT_MAX)
		return;
	
	// Check if this is a Team Player port.
	bool isTP = false;
	switch (port)
	{
		// Team Player, Port 1.
		case CtrlConfig::PORT_TP1A:
		case CtrlConfig::PORT_TP1B:
		case CtrlConfig::PORT_TP1C:
		case CtrlConfig::PORT_TP1D:
			if (m_devType[0] == LibGens::IoBase::IOT_TEAMPLAYER)
				isTP = true;
			break;
		
		// Team Player, Port 2.
		case CtrlConfig::PORT_TP2A:
		case CtrlConfig::PORT_TP2B:
		case CtrlConfig::PORT_TP2C:
		case CtrlConfig::PORT_TP2D:
			if (m_devType[1] == LibGens::IoBase::IOT_TEAMPLAYER)
				isTP = true;
			break;
		
		// 4-Way Play.
		case CtrlConfig::PORT_4WPA:
		case CtrlConfig::PORT_4WPB:
		case CtrlConfig::PORT_4WPC:
		case CtrlConfig::PORT_4WPD:
			if (m_devType[0] == LibGens::IoBase::IOT_4WP_SLAVE &&
			    m_devType[1] == LibGens::IoBase::IOT_4WP_MASTER)
			{
				isTP = true;
			}
			break;
		
		default:
			// Other port.
			break;
	}

	// Device setting is valid.
	
	// Make sure the dropdown index is set properly to reduce flicker.
	cboDevice_lock();
	const int idx = (int)m_devType[port];
	if (idx < cboDevice->count())
		cboDevice->setCurrentIndex(idx);
	cboDevice_unlock();
	
	// Update the port settings.
	m_selPort = port;
	cboDevice_setTP(isTP);		// Update Team Player device availability.
	updatePortSettings(port);	// Update the port settings.
}


/**
 * cboDevice_setTP(): Set cboDevice's Team Player device availability.
 * @param isTP If true, don't show devices that can't be connected to a TeamPlayer/4WP/etc device.
 */
void CtrlConfigWindow::cboDevice_setTP(bool isTP)
{
	const int devCount = (isTP ? LibGens::IoBase::IOT_6BTN : LibGens::IoBase::IOT_4WP_MASTER) + 1;
	if (cboDevice->count() == devCount)
		return;
	
	// Dropdown needs to be updated.
	cboDevice_lock();
	if (cboDevice->count() > devCount)
	{
		// Remove the extra items.
		for (int i = LibGens::IoBase::IOT_4WP_MASTER;
		     i > LibGens::IoBase::IOT_6BTN; i--)
		{
			cboDevice->removeItem(i);
		}
	}
	else
	{
		// Add the extra items.
		for (int i = LibGens::IoBase::IOT_2BTN;
		     i <= LibGens::IoBase::IOT_4WP_MASTER; i++)
		{
			cboDevice->addItem(GetCtrlIcon((LibGens::IoBase::IoType)i),
					GetShortDeviceName((LibGens::IoBase::IoType)i));
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
 * toolbarPortSelected(): The selected port in the toolbar has been changed.
 * @param i Port number.
 */
void CtrlConfigWindow::toolbarPortSelected(int i)
{
	QAction *action = qobject_cast<QAction*>(m_mapperSelPort->mapping(i));
	if (action && action->isChecked())
		selectPort(i);
}


/**
 * on_cboDevice_currentIndexChanged(): Device type has been changed.
 * @param index Device type index.
 */
void CtrlConfigWindow::on_cboDevice_currentIndexChanged(int index)
{
	if (isCboDeviceLocked())
		return;
	
	if (m_selPort < CtrlConfig::PORT_1 || m_selPort >= CtrlConfig::PORT_MAX)
		return;
	
	// Check if the device type has been changed.
	if (index < 0)
		return;
	if (m_devType[m_selPort] == index)
		return;
	if (m_devType[m_selPort] < LibGens::IoBase::IOT_NONE ||
	    m_devType[m_selPort] >= LibGens::IoBase::IOT_MAX)
	{
		// Invalid device type.
		return;
	}
	
	// Check for 4WP.
	if (m_selPort == CtrlConfig::PORT_1)
	{
		// Port 1.
		if (index == LibGens::IoBase::IOT_4WP_MASTER)
		{
			// 4WP SLAVE set for Port 1.
			// (4WP_MASTER index used for dropdown.)
			// TODO: Maybe use the combo box item's data parameter?
			m_devType[m_selPort] = LibGens::IoBase::IOT_4WP_SLAVE;
			
			// Make sure Port 2 is set to 4WP MASTER.
			if (m_devType[CtrlConfig::PORT_2] != LibGens::IoBase::IOT_4WP_MASTER)
			{
				m_devType[CtrlConfig::PORT_2] = LibGens::IoBase::IOT_4WP_MASTER;
				updatePortButton(CtrlConfig::PORT_2);
			}
		}
		else
		{
			// 4WP SLAVE not set for Port 1.
			m_devType[m_selPort] = (LibGens::IoBase::IoType)index;
			
			// Check if Port 2 is set to 4WP MASTER. (or 4WP SLAVE for sanity check)
			if (m_devType[CtrlConfig::PORT_2] == LibGens::IoBase::IOT_4WP_MASTER ||
			    m_devType[CtrlConfig::PORT_2] == LibGens::IoBase::IOT_4WP_SLAVE)
			{
				// Port 2 is set to 4WP MASTER. Unset it.
				m_devType[CtrlConfig::PORT_2] = LibGens::IoBase::IOT_NONE;
				updatePortButton(CtrlConfig::PORT_2);
			}
		}
	}
	else if (m_selPort == CtrlConfig::PORT_2)
	{
		// Port 2.
		if (index == LibGens::IoBase::IOT_4WP_MASTER)
		{
			// 4WP MASTER set for Port 2.
			m_devType[m_selPort] = LibGens::IoBase::IOT_4WP_MASTER;
			
			// Make sure Port 1 is set to 4WP SLAVE.
			if (m_devType[CtrlConfig::PORT_1] != LibGens::IoBase::IOT_4WP_SLAVE)
			{
				m_devType[CtrlConfig::PORT_1] = LibGens::IoBase::IOT_4WP_SLAVE;
				updatePortButton(CtrlConfig::PORT_1);
			}
		}
		else
		{
			// 4WP MASTER not set for Port 2.
			m_devType[m_selPort] = (LibGens::IoBase::IoType)index;
			
			// Check if Port 1 is set to 4WP SLAVE. (or 4WP MASTER for sanity check)
			if (m_devType[CtrlConfig::PORT_1] == LibGens::IoBase::IOT_4WP_SLAVE ||
			    m_devType[CtrlConfig::PORT_1] == LibGens::IoBase::IOT_4WP_MASTER)
			{
				// Port 1 is set to 4WP SLAVE. Unset it.
				m_devType[CtrlConfig::PORT_1] = LibGens::IoBase::IOT_NONE;
				updatePortButton(CtrlConfig::PORT_1);
			}
		}
	}
	else
	{
		// Other port.
		m_devType[m_selPort] = (LibGens::IoBase::IoType)index;
	}
	
	// Update the port information.
	updatePortButton(m_selPort);
	updatePortSettings(m_selPort);
}

}
