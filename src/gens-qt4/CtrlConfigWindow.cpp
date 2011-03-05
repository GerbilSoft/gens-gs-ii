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

namespace GensQt4
{

// Static member initialization.
CtrlConfigWindow *CtrlConfigWindow::m_CtrlConfigWindow = NULL;

// Controller icon filenames.
const char *CtrlConfigWindow::ms_CtrlIconFilenames[LibGens::IoBase::IOT_MAX] =
{
	":/gens/controller-none.png",		// IOT_NONE
	":/gens/controller-3btn.png",		// IOT_3BTN
	":/gens/controller-6btn.png",		// IOT_6BTN
	":/gens/controller-2btn.png",		// IOT_2BTN
	":/gens/controller-mega-mouse.png",	// IOT_MEGA_MOUSE (TODO)
	":/gens/controller-teamplayer.png",	// IOT_TEAMPLAYER (TODO)
	":/gens/controller-4wp.png",		// IOT_4WP_MASTER (TODO)
	":/gens/controller-4wp.png",		// IOT_4WP_SLAVE (TODO)
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
	
	// Find the toolbar separators.
	// Qt Designer doesn't save them for some reason.
	size_t tbSepCnt = 0;
	foreach(QAction *action, toolBar->actions())
	{
		if (action->isSeparator())
		{
			m_tbSep[tbSepCnt] = action;
			tbSepCnt++;
			if (tbSepCnt >= (sizeof(m_tbSep) / sizeof(m_tbSep[0])))
				break;
		}
	}
	
	// Initialize the "Device" dropdown.
	// TODO: Handle Master/Slave devices.
	cboDevice_lock();
	for (int i = LibGens::IoBase::IOT_NONE; i <= LibGens::IoBase::IOT_TEAMPLAYER; i++)
	{
		cboDevice->addItem(QIcon(QLatin1String(ms_CtrlIconFilenames[i])),
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
	for (int i = 0; i < CTRL_CFG_MAX_PORTS; i++)
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
 * GetShortDeviceName(): Get the short name of an I/O device.
 * @param devType Device type.
 * @return Short device name.
 */
const QString CtrlConfigWindow::GetShortDeviceName(LibGens::IoBase::IoType devType)
{
	using LibGens::IoBase;
	
	switch (devType)
	{
		case IoBase::IOT_NONE:
		default:
			return tr("None");
		
		case IoBase::IOT_3BTN:		return tr("3-button");
		case IoBase::IOT_6BTN:		return tr("6-button");
		case IoBase::IOT_2BTN:		return tr("2-button");
		case IoBase::IOT_MEGA_MOUSE:	return tr("Mega Mouse");
		case IoBase::IOT_TEAMPLAYER:	return tr("Team Player");
		case IoBase::IOT_4WP_MASTER:	/* see below */
		case IoBase::IOT_4WP_SLAVE:	return tr("4-Way Play");
	}
}


/**
 * GetLongDeviceName(): Get the long name of an I/O device.
 * @param devType Device type.
 * @return Long device name.
 */
const QString CtrlConfigWindow::GetLongDeviceName(LibGens::IoBase::IoType devType)
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
const QString CtrlConfigWindow::GetPortName(int port)
{
	if (port == 0 || port == 1)
		return tr("Port %1").arg(port+1);
	else if (port >= CTRL_CFG_PORT_TP1A && port < (CTRL_CFG_PORT_TP1A+4))
	{
		return tr("Team Player %1, Port %2").arg(1)
			.arg(QChar(L'A' + (port - CTRL_CFG_PORT_TP1A)));
	}
	else if (port >= CTRL_CFG_PORT_TP2A && port < (CTRL_CFG_PORT_TP2A+4))
	{
		return tr("Team Player %1, Port %2").arg(2)
			.arg(QChar(L'A' + (port - CTRL_CFG_PORT_TP2A)));
	}
	else if (port >= CTRL_CFG_PORT_4WPA && port < (CTRL_CFG_PORT_4WPA+4))
	{
		return tr("EA 4-Way Play, Port %1")
			.arg(QChar(L'A' + (port - CTRL_CFG_PORT_4WPA)));
	}
	
	// Unknown port number.
	return QString();
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
		// Base ports.
		case 0:		actionPort = actionPort1; break;
		case 1:		actionPort = actionPort2; break;
		
		// TeamPlayer 1.
		case CTRL_CFG_PORT_TP1A:	actionPort = actionPortTP1A; break;
		case CTRL_CFG_PORT_TP1A+1:	actionPort = actionPortTP1B; break;
		case CTRL_CFG_PORT_TP1A+2:	actionPort = actionPortTP1C; break;
		case CTRL_CFG_PORT_TP1A+3:	actionPort = actionPortTP1D; break;

		// TeamPlayer 2.
		case CTRL_CFG_PORT_TP2A:	actionPort = actionPortTP2A; break;
		case CTRL_CFG_PORT_TP2A+1:	actionPort = actionPortTP2B; break;
		case CTRL_CFG_PORT_TP2A+2:	actionPort = actionPortTP2C; break;
		case CTRL_CFG_PORT_TP2A+3:	actionPort = actionPortTP2D; break;
		
		// EA 4-Way Play.
		case CTRL_CFG_PORT_4WPA:	actionPort = actionPort4WPA; break;
		case CTRL_CFG_PORT_4WPA+1:	actionPort = actionPort4WPB; break;
		case CTRL_CFG_PORT_4WPA+2:	actionPort = actionPort4WPC; break;
		case CTRL_CFG_PORT_4WPA+3:	actionPort = actionPort4WPD; break;
		
		default:
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
	actionPort->setIcon(QIcon(QLatin1String(ms_CtrlIconFilenames[m_devType[port]])));
	actionPort->setToolTip(GetLongDeviceName(m_devType[port]));
	
	if (port == 0)
	{
		// Port 1. Update TeamPlayer 1 button state.
		const bool isTP = (m_devType[port] == LibGens::IoBase::IOT_TEAMPLAYER);
		m_tbSep[CTRL_CFG_TBSEP_TP1]->setVisible(isTP);
		actionPortTP1A->setVisible(isTP);
		actionPortTP1B->setVisible(isTP);
		actionPortTP1C->setVisible(isTP);
		actionPortTP1D->setVisible(isTP);
	}
	else if (port == 1)
	{
		// Port 2. Update TeamPlayer 2 button state.
		const bool isTP = (m_devType[port] == LibGens::IoBase::IOT_TEAMPLAYER);
		m_tbSep[CTRL_CFG_TBSEP_TP2]->setVisible(isTP);
		actionPortTP2A->setVisible(isTP);
		actionPortTP2B->setVisible(isTP);
		actionPortTP2C->setVisible(isTP);
		actionPortTP2D->setVisible(isTP);
	}
	
	if (port == 0 || port == 1)
	{
		// Port 1 or 2. Update EA 4-Way Play button state.
		const bool is4WP = (m_devType[0] == LibGens::IoBase::IOT_4WP_SLAVE &&
				    m_devType[1] == LibGens::IoBase::IOT_4WP_MASTER);
		
		m_tbSep[CTRL_CFG_TBSEP_4WP]->setVisible(is4WP);
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
	if (port < 0 || port >= CTRL_CFG_MAX_PORTS)
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
	if (port < 0 || port >= CTRL_CFG_MAX_PORTS)
		return;
	
	// Check if this is a TeamPlayer port.
	bool isTP = false;
	if (port >= CTRL_CFG_PORT_TP1A && port < (CTRL_CFG_PORT_TP1A+4))
	{
		// TeamPlayer on Port 1.
		if (m_devType[0] == LibGens::IoBase::IOT_TEAMPLAYER)
			isTP = true;
	}
	else if (port >= CTRL_CFG_PORT_TP2A && port < (CTRL_CFG_PORT_TP2A+4))
	{
		// TeamPlayer on Port 2.
		if (m_devType[1] == LibGens::IoBase::IOT_TEAMPLAYER)
			isTP = true;
	}
	else if (port >= CTRL_CFG_PORT_4WPA && port < (CTRL_CFG_PORT_4WPA+4))
	{
		// EA 4-Way Play.
		if (m_devType[0] == LibGens::IoBase::IOT_4WP_SLAVE &&
		    m_devType[1] == LibGens::IoBase::IOT_4WP_MASTER)
		{
			isTP = true;
		}
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
	const int devCount = (isTP ? LibGens::IoBase::IOT_6BTN : LibGens::IoBase::IOT_TEAMPLAYER) + 1;
	if (cboDevice->count() == devCount)
		return;
	
	// Dropdown needs to be updated.
	cboDevice_lock();
	if (cboDevice->count() > devCount)
	{
		// Remove the extra items.
		for (int i = LibGens::IoBase::IOT_TEAMPLAYER;
		     i > LibGens::IoBase::IOT_6BTN; i--)
		{
			cboDevice->removeItem(i);
		}
	}
	else
	{
		// Add the extra items.
		for (int i = LibGens::IoBase::IOT_2BTN;
		     i <= LibGens::IoBase::IOT_TEAMPLAYER; i++)
		{
			cboDevice->addItem(QIcon(QLatin1String(ms_CtrlIconFilenames[i])),
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

// TODO: Use a signal mapper.
void CtrlConfigWindow::on_actionPort1_toggled(bool checked)
	{ if (checked) selectPort(0); }
void CtrlConfigWindow::on_actionPort2_toggled(bool checked)
	{ if (checked) selectPort(1); }

void CtrlConfigWindow::on_actionPortTP1A_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP1A); }
void CtrlConfigWindow::on_actionPortTP1B_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP1A+1); }
void CtrlConfigWindow::on_actionPortTP1C_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP1A+2); }
void CtrlConfigWindow::on_actionPortTP1D_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP1A+3); }

void CtrlConfigWindow::on_actionPortTP2A_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP2A); }
void CtrlConfigWindow::on_actionPortTP2B_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP2A+1); }
void CtrlConfigWindow::on_actionPortTP2C_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP2A+2); }
void CtrlConfigWindow::on_actionPortTP2D_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_TP2A+3); }

void CtrlConfigWindow::on_actionPort4WPA_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_4WPA); }
void CtrlConfigWindow::on_actionPort4WPB_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_4WPA+1); }
void CtrlConfigWindow::on_actionPort4WPC_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_4WPA+2); }
void CtrlConfigWindow::on_actionPort4WPD_toggled(bool checked)
	{ if (checked) selectPort(CTRL_CFG_PORT_4WPA+3); }


/**
 * on_cboDevice_currentIndexChanged(): Device type has been changed.
 * @param index Device type index.
 */
void CtrlConfigWindow::on_cboDevice_currentIndexChanged(int index)
{
	if (isCboDeviceLocked())
		return;
	
	if (m_selPort < 0 || m_selPort >= CTRL_CFG_MAX_PORTS)
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
	
	// Device type has been changed.
	m_devType[m_selPort] = (LibGens::IoBase::IoType)index;
	updatePortButton(m_selPort);
	updatePortSettings(m_selPort);
}

}
