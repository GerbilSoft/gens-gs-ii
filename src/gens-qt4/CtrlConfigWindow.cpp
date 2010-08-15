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

// Text translation macro.
#define TR(text) \
	QApplication::translate("CtrlConfigWindow", (text), NULL, QApplication::UnicodeUTF8)

// EmuMD has the I/O devices.
#include "libgens/MD/EmuMD.hpp"

namespace GensQt4
{

// Static member initialization.
CtrlConfigWindow *CtrlConfigWindow::m_CtrlConfigWindow = NULL;

// Controller icon filenames.
const char *CtrlConfigWindow::ms_CtrlIconFilenames[LibGens::IoBase::IOT_MAX] =
{
	":/gens/controller-none",	// IOT_NONE
	":/gens/controller-3btn",	// IOT_3BTN
	":/gens/controller-6btn",	// IOT_6BTN
	":/gens/controller-2btn",	// IOT_2BTN
	":/gens/controller-mega-mouse"	// IOT_MEGA_MOUSE (TODO)
	":/gens/controller-teamplayer"	// IOT_TEAMPLAYER (TODO)
};


/**
 * CtrlConfigWindow(): Initialize the About window.
 */
CtrlConfigWindow::CtrlConfigWindow(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
	// Copy the current controller settings.
	// TODO: Button mapping.
	m_devType[0] = LibGens::EmuMD::m_port1->devType();
	m_devType[1] = LibGens::EmuMD::m_port2->devType();
	
	// Initialize all the settings.
	updatePortButton(0);
	updatePortButton(1);
	
	// Initialize the "Device" dropdown.
	// TODO: Make a list of device names and icons.
	cboDevice->addItem(QIcon(":/gens/controller-none"),
			GetShortDeviceName(LibGens::IoBase::IOT_NONE));
	cboDevice->addItem(QIcon(":/gens/controller-3btn"),
			GetShortDeviceName(LibGens::IoBase::IOT_3BTN));
	cboDevice->addItem(QIcon(":/gens/controller-6btn"),
			GetShortDeviceName(LibGens::IoBase::IOT_6BTN));
	cboDevice->addItem(QIcon(":/gens/controller-2btn"),
			GetShortDeviceName(LibGens::IoBase::IOT_2BTN));
	cboDevice->addItem(QIcon(":/gens/controller-mouse"),
			GetShortDeviceName(LibGens::IoBase::IOT_MEGA_MOUSE));
	cboDevice->addItem(QIcon(":/gens/controller-teamplayer"),
			GetShortDeviceName(LibGens::IoBase::IOT_TEAMPLAYER));
	
	// Update the port settings.
	updatePortSettings();
}


/**
 * ~CtrlConfigWindow(): Shut down the About window.
 */
CtrlConfigWindow::~CtrlConfigWindow()
{
	// Clear the m_CtrlConfigWindow pointer.
	m_CtrlConfigWindow = NULL;
}


/**
 * ShowSingle(): Show a single instance of the About window.
 * @param parent Parent window.
 */
void CtrlConfigWindow::ShowSingle(QWidget *parent)
{
	if (m_CtrlConfigWindow != NULL)
	{
		// About Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(m_CtrlConfigWindow);
	}
	else
	{
		// About Window is not displayed.
		m_CtrlConfigWindow = new CtrlConfigWindow(parent);
		m_CtrlConfigWindow->show();
	}
}


/**
 * GetShortDeviceName(): Get the short name of an I/O device.
 * TODO: Use IoBase::devName()?
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
			return TR("None");
		
		case IoBase::IOT_3BTN:		return TR("3-button");
		case IoBase::IOT_6BTN:		return TR("6-button");
		case IoBase::IOT_2BTN:		return TR("2-button");
		case IoBase::IOT_MEGA_MOUSE:	return TR("Mega Mouse");
		case IoBase::IOT_TEAMPLAYER:	return TR("Teamplayer");
	}
}


/**
 * GetLongDeviceName(): Get the long name of an I/O device.
 * TODO: Use IoBase::devName()?
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
			return TR("No device connected.");
		
		case IoBase::IOT_3BTN:		return TR("3-button gamepad");
		case IoBase::IOT_6BTN:		return TR("6-button gamepad");
		case IoBase::IOT_2BTN:		return TR("2-button gamepad (SMS)");
		case IoBase::IOT_MEGA_MOUSE:	return TR("Mega Mouse");
		case IoBase::IOT_TEAMPLAYER:	return TR("Sega Teamplayer");
	}
}


/**
 * updatePortButton(): Update a port button.
 * @param port Port ID.
 */
void CtrlConfigWindow::updatePortButton(int port)
{
	QPushButton *btnPort;
	switch (port)
	{
		case 0:
			btnPort = this->btnPort1;
			break;
		case 1:
			btnPort = this->btnPort2;
			break;
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
	btnPort->setIcon(QIcon(ms_CtrlIconFilenames[m_devType[port]]));
	btnPort->setToolTip(GetLongDeviceName(m_devType[port]));
}


/**
 * updatePortSettings(): Update port settings.
 */
void CtrlConfigWindow::updatePortSettings(void)
{
	int port = selectedPort();
	if (port < 0)
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
	grpPortSettings->setTitle(TR("Controller Settings") + ": Port " + QString::number(port + 1));
	
	// Set the device type in the dropdown.
	cboDevice->setCurrentIndex(m_devType[port]);
}

}
