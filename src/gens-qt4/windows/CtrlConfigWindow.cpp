/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfigWindow.cpp: Controller Configuration Window.                  *
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

#include "CtrlConfigWindow.hpp"
#include "gqt4_main.hpp"
#include "GensQApplication.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Controller I/O manager.
#include "libgens/IO/IoManager.hpp"
using LibGens::IoManager;

// LibGensKeys: Key Manager
#include "libgenskeys/KeyManager.hpp"
using LibGensKeys::KeyManager;

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtGui/QActionGroup>
#include <QtGui/QKeyEvent>
#include <QtGui/QPushButton>

#include "ui_CtrlConfigWindow.h"
namespace GensQt4 {

class CtrlConfigWindowPrivate
{
	public:
		CtrlConfigWindowPrivate(CtrlConfigWindow *q);
		~CtrlConfigWindowPrivate();

	private:
		CtrlConfigWindow *const q_ptr;
		Q_DECLARE_PUBLIC(CtrlConfigWindow)
	private:
		Q_DISABLE_COPY(CtrlConfigWindowPrivate)

	public:
		// Single window instance.
		static CtrlConfigWindow *ms_Window;
		Ui::CtrlConfigWindow ui;

		// Controller data.
		QString getShortDeviceName(LibGens::IoManager::IoType_t ioType) const;
		QString getLongDeviceName(LibGens::IoManager::IoType_t ioType) const;
		QString getPortName(LibGens::IoManager::VirtPort_t virtPort) const;

		// Controller icons.
		void loadCtrlIcons(void);
		QIcon ctrlIcons[LibGens::IoManager::IOT_MAX];

		// Internal KeyManager instance.
		KeyManager *keyManager;

		// Selected port.
		LibGens::IoManager::VirtPort_t selPort;
		QActionGroup *actgrpSelPort;
		QSignalMapper *mapperSelPort;

		// Toolbar separators.
		QVector<QAction*> vecTbSep;
		enum CtrlCfgTBSep_t {
			CTRL_CFG_TBSEP_TP1 = 0,
			CTRL_CFG_TBSEP_TP2,
			CTRL_CFG_TBSEP_4WP,
			CTRL_CFG_TBSEP_MTAP1,
			CTRL_CFG_TBSEP_MTAP2,

			CTRL_CFG_TBSEP_MAX
		};

	private:
		// Dropdown device lock.
		// Used when rebuilding cboDevice.
		int m_cboDeviceLockCnt;

		// Map of IoManager::IoType_t to cboDevice indexes and vice-versa.
		QMap<IoManager::IoType_t, int> map_ioTypeToIdx;
		QMap<int, IoManager::IoType_t> map_idxToIoType;

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

	private:
		/**
		 * Add an I/O device to cboDevice.
		 */
		void addIoTypeToCboDevice(IoManager::IoType_t ioType);

	public:
		/**
		 * Initialize cboDevice.
		 * @param isTP If true, only show devices that can be used on Team Player ports.
		 */
		void initCboDevice(bool isTP = false);

		/**
		 * Set the device type in cboDevice.
		 * @param ioType I/O device type.
		 */
		void setCboDeviceIoType(IoManager::IoType_t ioType);

		/**
		 * Get cboDevice's currently-selected device type.
		 * @return I/O device type.
		 */
		IoManager::IoType_t cboDeviceIoType(void);

		/**
		 * Get cboDevice's currently-selected device type.
		 * @param index Index. (Used in the currentIndexChanged slot.)
		 * @return I/O device type.
		 */
		IoManager::IoType_t cboDeviceIoType(int index);
};

/** CtrlConfigWindowPrivate **/

// Single window instance.
CtrlConfigWindow *CtrlConfigWindowPrivate::ms_Window = nullptr;

CtrlConfigWindowPrivate::CtrlConfigWindowPrivate(CtrlConfigWindow *q)
	: q_ptr(q)
	, keyManager(new KeyManager())
	, selPort(IoManager::VIRTPORT_1)
	, actgrpSelPort(new QActionGroup(q))
	, mapperSelPort(new QSignalMapper(q))
	, m_cboDeviceLockCnt(0)
{
	// Set the single window instance pointer.
	ms_Window = q;

	// Load controller icons.
	loadCtrlIcons();

	// Signal mapper for toolbar buttons.
	QObject::connect(mapperSelPort, SIGNAL(mapped(int)),
			 q, SLOT(toolbarPortSelected(int)));
}

CtrlConfigWindowPrivate::~CtrlConfigWindowPrivate()
{
	delete keyManager;

	// Clear the single window instance pointer.
	ms_Window = nullptr;
}

/**
 * Get the short name of an I/O device.
 * @param ioType Device type.
 * @return Short device name.
 */
QString CtrlConfigWindowPrivate::getShortDeviceName(IoManager::IoType_t ioType) const
{
	switch (ioType) {
		default:
			return QString::fromStdString(IoManager::IoTypeToString(ioType));
		case IoManager::IOT_NONE:
			return CtrlConfigWindow::tr("None", "shortDeviceName");
		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return CtrlConfigWindow::tr("3-button", "shortDeviceName");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return CtrlConfigWindow::tr("6-button", "shortDeviceName");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return CtrlConfigWindow::tr("2-button", "shortDeviceName");

		// Miscellaneous Master System peripherals.
		case IoManager::IOT_PADDLE:
			//: Paddle Control. (Sega Mark III)
			return CtrlConfigWindow::tr("Paddle Control", "shortDeviceName");
		case IoManager::IOT_SPORTS_PAD:
			//: Sega Sports Pad.
			return CtrlConfigWindow::tr("Sports Pad", "shortDeviceName");

		// Miscellaneous Mega Drive peripherals.
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return CtrlConfigWindow::tr("Mega Mouse", "shortDeviceName");
		case IoManager::IOT_XE_1AP:
			//: XE-1 AP.
			return CtrlConfigWindow::tr("XE-1 AP", "shortDeviceName");
		case IoManager::IOT_ACTIVATOR:
			//: Sega Activator.
			return CtrlConfigWindow::tr("Activator", "shortDeviceName");

		// Light guns.
		case IoManager::IOT_PHASER:
			//: Light Phaser.
			return CtrlConfigWindow::tr("Light Phaser", "shortDeviceName");
		case IoManager::IOT_MENACER:
			//: Sega Menacer.
			return CtrlConfigWindow::tr("Menacer", "shortDeviceName");
		case IoManager::IOT_JUSTIFIER:
			//: Konami Justifier.
			return CtrlConfigWindow::tr("Justifier", "shortDeviceName");

		// ColecoVision.
		case IoManager::IOT_COLECOVISION:
			//: ColecoVision.
			return CtrlConfigWindow::tr("ColecoVision", "shortDeviceName");

		// Multitaps.
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return CtrlConfigWindow::tr("Team Player", "shortDeviceName");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return CtrlConfigWindow::tr("4-Way Play", "shortDeviceName");
		case IoManager::IOT_MASTERTAP:
			//: Master Tap. (Homebrew multitap for Sega Master System.)
			return CtrlConfigWindow::tr("Master Tap", "shortDeviceName");
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
		default:
			return QString::fromStdString(IoManager::IoTypeToString(ioType));
		case IoManager::IOT_NONE:
			return CtrlConfigWindow::tr("No device connected.", "longDeviceName");
		case IoManager::IOT_3BTN:
			//: Standard 3-button control pad.
			return CtrlConfigWindow::tr("3-button gamepad", "longDeviceName");
		case IoManager::IOT_6BTN:
			//: Sega 6-button "arcade" control pad.
			return CtrlConfigWindow::tr("6-button gamepad", "longDeviceName");
		case IoManager::IOT_2BTN:
			//: Sega Master System 2-button control pad.
			return CtrlConfigWindow::tr("2-button gamepad (SMS)", "longDeviceName");

		// Miscellaneous Master System peripherals.
		case IoManager::IOT_PADDLE:
			//: Paddle Control. (Sega Mark III)
			return CtrlConfigWindow::tr("Paddle Control", "longDeviceName");
		case IoManager::IOT_SPORTS_PAD:
			//: Sega Sports Pad.
			return CtrlConfigWindow::tr("Sega Sports Pad", "longDeviceName");

		// Miscellaneous Mega Drive peripherals.
		case IoManager::IOT_MEGA_MOUSE:
			//: Sega Mega Mouse.
			return CtrlConfigWindow::tr("Mega Mouse", "longDeviceName");
		case IoManager::IOT_XE_1AP:
			//: XE-1 AP.
			return CtrlConfigWindow::tr("XE-1 AP", "longDeviceName");
		case IoManager::IOT_ACTIVATOR:
			//: Sega Activator.
			return CtrlConfigWindow::tr("Sega Activator", "longDeviceName");

		// Light guns.
		case IoManager::IOT_PHASER:
			//: Light Phaser.
			return CtrlConfigWindow::tr("Light Phaser", "longDeviceName");
		case IoManager::IOT_MENACER:
			//: Menacer.
			return CtrlConfigWindow::tr("Menacer", "longDeviceName");
		case IoManager::IOT_JUSTIFIER:
			//: Konami Justifier.
			return CtrlConfigWindow::tr("Justifier", "longDeviceName");

		// ColecoVision.
		case IoManager::IOT_COLECOVISION:
			//: ColecoVision.
			return CtrlConfigWindow::tr("ColecoVision", "longDeviceName");

		// Multitaps.
		case IoManager::IOT_TEAMPLAYER:
			//: Sega Team Player. (Specific brand name; only modify if it's different in your region!)
			return CtrlConfigWindow::tr("Sega Team Player", "longDeviceName");
		case IoManager::IOT_4WP_MASTER:	/* fallthrough */
		case IoManager::IOT_4WP_SLAVE:
			//: EA 4-Way Play. (Specific brand name; only modify if it's different in your region!)
			return CtrlConfigWindow::tr("EA 4-Way Play", "longDeviceName");
		case IoManager::IOT_MASTERTAP:
			//: Master Tap. (Homebrew multitap for Sega Master System.)
			return CtrlConfigWindow::tr("Master Tap", "longDeviceName");
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
			return CtrlConfigWindow::tr("Port %1")
				.arg(virtPort - IoManager::VIRTPORT_1 + 1);

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			return CtrlConfigWindow::tr("Team Player %1, Port %2").arg(1)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP1A)));

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			return CtrlConfigWindow::tr("Team Player %1, Port %2").arg(2)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_TP2A)));

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			return CtrlConfigWindow::tr("4-Way Play, Port %1")
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_4WPA)));

		// Master Tap, Port 1.
		case IoManager::VIRTPORT_MTAP1A:
		case IoManager::VIRTPORT_MTAP1B:
		case IoManager::VIRTPORT_MTAP1C:
		case IoManager::VIRTPORT_MTAP1D:
			return CtrlConfigWindow::tr("Master Tap %1, Port %2").arg(1)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_MTAP1A)));

		// Master Tap, Port 2.
		case IoManager::VIRTPORT_MTAP2A:
		case IoManager::VIRTPORT_MTAP2B:
		case IoManager::VIRTPORT_MTAP2C:
		case IoManager::VIRTPORT_MTAP2D:
			return CtrlConfigWindow::tr("Master Tap %1, Port %2").arg(2)
				.arg(QChar(L'A' + (virtPort - IoManager::VIRTPORT_MTAP2A)));

		default:
			// Unknown port number.
			return QString();
	}

	// Should not get here...
	return QString();
}

/**
 * Load all controller icons.
 */
void CtrlConfigWindowPrivate::loadCtrlIcons(void)
{
	const QString qsControllers = QLatin1String("controllers");

	for (int ioType = 0; ioType < LibGens::IoManager::IOT_MAX; ioType++) {
		// Controller icons are named using the FourCC.
		const QString qsFourCC = QString::fromStdString(
			IoManager::IoTypeToString((IoManager::IoType_t)ioType));

		// Get the icon.
		QIcon ctrlIcon = GensQApplication::IconFromProgram(qsFourCC, qsControllers);
		ctrlIcons[ioType] = ctrlIcon;
	}
}

/**
 * Add an I/O device to cboDevice.
 */
void CtrlConfigWindowPrivate::addIoTypeToCboDevice(IoManager::IoType_t ioType)
{
	ui.cboDevice->addItem(ctrlIcons[ioType], getShortDeviceName(ioType));
	const int idx = ui.cboDevice->count() - 1;
	map_ioTypeToIdx.insert(ioType, idx);
	map_idxToIoType.insert(idx, ioType);
}


/**
 * Initialize cboDevice.
 * @param isTP If true, only show devices that can be used on Team Player ports.
 */
void CtrlConfigWindowPrivate::initCboDevice(bool isTP)
{
	// on_cboDevice_currentIndexChanged() handles translation for Port 1.
	cboDevice_lock();

	// Clear all the maps.
	map_ioTypeToIdx.clear();
	map_idxToIoType.clear();
	ui.cboDevice->clear();

	// Determine how many devices should be added to the dropdown.
	// TODO: Use bitfields for each port instead of this mess.
	const int ioTypeMax = (isTP ? (IoManager::IOT_6BTN + 1) : IoManager::IOT_MAX);

	if (selPort >= IoManager::VIRTPORT_MTAP1A &&
	    selPort <= IoManager::VIRTPORT_MTAP2D)
	{
		// Master Tap. Only NONE and 2BTN are allowed.
		addIoTypeToCboDevice(IoManager::IOT_NONE);
		addIoTypeToCboDevice(IoManager::IOT_2BTN);
	} else {
		for (int ioType = IoManager::IOT_NONE;
		     ioType < ioTypeMax; ioType++)
		{
			if (!IoManager::IsDevTypeUsable((IoManager::IoType_t)ioType))
				continue;

			switch (ioType) {
				case IoManager::IOT_TEAMPLAYER:
				case IoManager::IOT_4WP_MASTER:
				case IoManager::IOT_4WP_SLAVE:
				case IoManager::IOT_MASTERTAP:
					// Multitaps are added at the end of the list.
					break;

				default:
					addIoTypeToCboDevice((IoManager::IoType_t)ioType);
					break;
			}
		}

		// Also add Mega Mouse to Team Player devices.
		// (NOT 4WP though...)
		if (selPort >= IoManager::VIRTPORT_TP1A &&
		    selPort <= IoManager::VIRTPORT_TP2D)
		{
			addIoTypeToCboDevice(IoManager::IOT_MEGA_MOUSE);
		}
	}

	// Add multitaps.
	if (!isTP) {
		addIoTypeToCboDevice(IoManager::IOT_TEAMPLAYER);
		addIoTypeToCboDevice(IoManager::IOT_4WP_MASTER);
		addIoTypeToCboDevice(IoManager::IOT_MASTERTAP);
	}

	cboDevice_unlock();
}


/**
 * Set the device type in cboDevice.
 * @param ioType I/O device type.
 */
void CtrlConfigWindowPrivate::setCboDeviceIoType(IoManager::IoType_t ioType)
{
	assert(ioType >= IoManager::IOT_NONE  && ioType < IoManager::IOT_MAX);
	const int idx = map_ioTypeToIdx.value(ioType, 0);
	if (idx < ui.cboDevice->count())
		ui.cboDevice->setCurrentIndex(idx);
}

/**
 * Get cboDevice's currently-selected device type.
 * @return I/O device type.
 */
IoManager::IoType_t CtrlConfigWindowPrivate::cboDeviceIoType(void)
{
	const int idx = ui.cboDevice->currentIndex();
	return map_idxToIoType.value(idx, IoManager::IOT_NONE);
}

/**
 * Get cboDevice's currently-selected device type.
 * @param index Index. (Used in the currentIndexChanged slot.)
 * @return I/O device type.
 */
IoManager::IoType_t CtrlConfigWindowPrivate::cboDeviceIoType(int index)
{
	return map_idxToIoType.value(index, IoManager::IOT_NONE);
}

/** CtrlConfigWindow **/

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
	, d_ptr(new CtrlConfigWindowPrivate(this))
{
	Q_D(CtrlConfigWindow);
	d->ui.setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// "OK" and "Cancel" are automatically connected by uic.
	// Connect a signal for the "Apply" button.
	QPushButton *btnApply = d->ui.buttonBox->button(QDialogButtonBox::Apply);
	connect(btnApply, SIGNAL(clicked(bool)), this, SLOT(apply()));

	// Initialize the toolbar.
	d->actgrpSelPort->setExclusive(true);

	// Base ports.
	d->actgrpSelPort->addAction(d->ui.actionPort1);
	d->actgrpSelPort->addAction(d->ui.actionPort2);
	d->actgrpSelPort->addAction(d->ui.actionPortEXT);

	// Team Player, Port 1.
	d->actgrpSelPort->addAction(d->ui.actionPortTP1A);
	d->actgrpSelPort->addAction(d->ui.actionPortTP1B);
	d->actgrpSelPort->addAction(d->ui.actionPortTP1C);
	d->actgrpSelPort->addAction(d->ui.actionPortTP1D);

	// Team Player, Port 2.
	d->actgrpSelPort->addAction(d->ui.actionPortTP2A);
	d->actgrpSelPort->addAction(d->ui.actionPortTP2B);
	d->actgrpSelPort->addAction(d->ui.actionPortTP2C);
	d->actgrpSelPort->addAction(d->ui.actionPortTP2D);

	// EA 4-Way Play.
	d->actgrpSelPort->addAction(d->ui.actionPort4WPA);
	d->actgrpSelPort->addAction(d->ui.actionPort4WPB);
	d->actgrpSelPort->addAction(d->ui.actionPort4WPC);
	d->actgrpSelPort->addAction(d->ui.actionPort4WPD);

	// Master Tap, Port 1.
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP1A);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP1B);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP1C);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP1D);

	// Master Tap, Port 2.
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP2A);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP2B);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP2C);
	d->actgrpSelPort->addAction(d->ui.actionPortMTAP2D);

	// TODO: J-Cart ports?

	// Find the toolbar separators.
	// Qt Designer doesn't save them for some reason.
	// Also, add port buttons to the signal mapper.
	int portNum = 0;
	d->vecTbSep.reserve(d->CTRL_CFG_TBSEP_MAX);
	foreach(QAction *action, d->ui.toolBar->actions()) {
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
	d->initCboDevice();

	// Load the controller configuration settings.
	reload();

	// Set Port 1 as active.
	d->ui.actionPort1->setChecked(true);
}

/**
 * Shut down the Controller Configuration window.
 */
CtrlConfigWindow::~CtrlConfigWindow()
{
	delete d_ptr;
}

/**
 * Show a single instance of the Controller Configuration window.
 * @param parent Parent window.
 */
void CtrlConfigWindow::ShowSingle(QWidget *parent)
{
	if (CtrlConfigWindowPrivate::ms_Window != nullptr) {
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
		switch (event->key()) {
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
		Q_D(CtrlConfigWindow);
		d->ui.retranslateUi(this);

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
	Q_D(CtrlConfigWindow);
	QAction *actionPort;
	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:		actionPort = d->ui.actionPort1; break;
		case IoManager::VIRTPORT_2:		actionPort = d->ui.actionPort2; break;

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:		actionPort = d->ui.actionPortTP1A; break;
		case IoManager::VIRTPORT_TP1B:		actionPort = d->ui.actionPortTP1B; break;
		case IoManager::VIRTPORT_TP1C:		actionPort = d->ui.actionPortTP1C; break;
		case IoManager::VIRTPORT_TP1D:		actionPort = d->ui.actionPortTP1D; break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:		actionPort = d->ui.actionPortTP2A; break;
		case IoManager::VIRTPORT_TP2B:		actionPort = d->ui.actionPortTP2B; break;
		case IoManager::VIRTPORT_TP2C:		actionPort = d->ui.actionPortTP2C; break;
		case IoManager::VIRTPORT_TP2D:		actionPort = d->ui.actionPortTP2D; break;

		// EA 4-Way Play.
		case IoManager::VIRTPORT_4WPA:		actionPort = d->ui.actionPort4WPA; break;
		case IoManager::VIRTPORT_4WPB:		actionPort = d->ui.actionPort4WPB; break;
		case IoManager::VIRTPORT_4WPC:		actionPort = d->ui.actionPort4WPC; break;
		case IoManager::VIRTPORT_4WPD:		actionPort = d->ui.actionPort4WPD; break;

		// Master Tap, Port 1.
		case IoManager::VIRTPORT_MTAP1A:	actionPort = d->ui.actionPortMTAP1A; break;
		case IoManager::VIRTPORT_MTAP1B:	actionPort = d->ui.actionPortMTAP1B; break;
		case IoManager::VIRTPORT_MTAP1C:	actionPort = d->ui.actionPortMTAP1C; break;
		case IoManager::VIRTPORT_MTAP1D:	actionPort = d->ui.actionPortMTAP1D; break;

		// Master Tap, Port 2.
		case IoManager::VIRTPORT_MTAP2A:	actionPort = d->ui.actionPortMTAP2A; break;
		case IoManager::VIRTPORT_MTAP2B:	actionPort = d->ui.actionPortMTAP2B; break;
		case IoManager::VIRTPORT_MTAP2C:	actionPort = d->ui.actionPortMTAP2C; break;
		case IoManager::VIRTPORT_MTAP2D:	actionPort = d->ui.actionPortMTAP2D; break;

		// TODO: J-Cart

		default:
			// Unknown port.
			return;
	}

	// Make sure the device type is in bounds.
	const IoManager::IoType_t ioType = d->keyManager->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);

	// Update the port icon and tooltip.
	actionPort->setIcon(d->ctrlIcons[ioType]);
	actionPort->setToolTip(d->getPortName(virtPort) +
				QLatin1String(": ") +
				d->getLongDeviceName(ioType));

	// Hide the EXT port for now.
	d->ui.actionPortEXT->setVisible(false);

	if (virtPort == IoManager::VIRTPORT_1) {
		// Update multitap button state for Port 1.
		const bool isTP = (ioType == IoManager::IOT_TEAMPLAYER);
		const bool isMTAP = (ioType == IoManager::IOT_MASTERTAP);
		d->vecTbSep[d->CTRL_CFG_TBSEP_TP1]->setVisible(isTP);
		d->vecTbSep[d->CTRL_CFG_TBSEP_MTAP1]->setVisible(isMTAP);
		d->ui.actionPortTP1A->setVisible(isTP);
		d->ui.actionPortTP1B->setVisible(isTP);
		d->ui.actionPortTP1C->setVisible(isTP);
		d->ui.actionPortTP1D->setVisible(isTP);
		d->ui.actionPortMTAP1A->setVisible(isMTAP);
		d->ui.actionPortMTAP1B->setVisible(isMTAP);
		d->ui.actionPortMTAP1C->setVisible(isMTAP);
		d->ui.actionPortMTAP1D->setVisible(isMTAP);
	} else if (virtPort == IoManager::VIRTPORT_2) {
		// Update multitap button state for Port 2.
		const bool isTP = (ioType == IoManager::IOT_TEAMPLAYER);
		const bool isMTAP = (ioType == IoManager::IOT_MASTERTAP);
		d->vecTbSep[d->CTRL_CFG_TBSEP_TP2]->setVisible(isTP);
		d->vecTbSep[d->CTRL_CFG_TBSEP_MTAP2]->setVisible(isMTAP);
		d->ui.actionPortTP2A->setVisible(isTP);
		d->ui.actionPortTP2B->setVisible(isTP);
		d->ui.actionPortTP2C->setVisible(isTP);
		d->ui.actionPortTP2D->setVisible(isTP);
		d->ui.actionPortMTAP2A->setVisible(isMTAP);
		d->ui.actionPortMTAP2B->setVisible(isMTAP);
		d->ui.actionPortMTAP2C->setVisible(isMTAP);
		d->ui.actionPortMTAP2D->setVisible(isMTAP);
	}

	if (virtPort == IoManager::VIRTPORT_1 ||
	    virtPort == IoManager::VIRTPORT_2) {
		// Port 1 or 2. Update EA 4-Way Play button state.
		const bool is4WP =
			(d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE &&
			 d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER);

		d->vecTbSep[d->CTRL_CFG_TBSEP_4WP]->setVisible(is4WP);
		d->ui.actionPort4WPA->setVisible(is4WP);
		d->ui.actionPort4WPB->setVisible(is4WP);
		d->ui.actionPort4WPC->setVisible(is4WP);
		d->ui.actionPort4WPD->setVisible(is4WP);
	}
}

/**
 * Update port settings.
 * @param virtPort Virtual port to display.
 */
void CtrlConfigWindow::updatePortSettings(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	Q_D(CtrlConfigWindow);
	IoManager::IoType_t ioType = d->keyManager->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);

	// Set the "Port Settings" text.
	// TODO: Port names for when e.g. EXT, J-Cart, etc. are added.
	d->ui.grpPortSettings->setTitle(d->getPortName(virtPort));

	// Set the device type in the dropdown.
	if (ioType == IoManager::IOT_4WP_SLAVE) {
		// 4WP slave isn't listed in the dropdown.
		// Use 4WP master instead.
		ioType = IoManager::IOT_4WP_MASTER;
	}
	d->setCboDeviceIoType(ioType);

	// Set the device information in the GensCtrlCfgWidget.
	d->ui.ctrlCfgWidget->setIoType(ioType);
	uint32_t keyMap[IoManager::BTNI_MAX];
	d->keyManager->keyMap(virtPort, keyMap, ARRAY_SIZE(keyMap));

	// TODO: Add ctrlCfgWidget overload that takes uint32_t* + size.
	QVector<uint32_t> qvKeyMap;
	qvKeyMap.reserve(ARRAY_SIZE(keyMap));
	for (int i = 0; i < ARRAY_SIZE(keyMap); i++) {
		qvKeyMap.append(keyMap[i]);
	}
	d->ui.ctrlCfgWidget->setKeyMap(qvKeyMap);
}


/**
 * Select a port to display.
 * @param virtPort Virtual port to display.
 */
void CtrlConfigWindow::selectPort(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	Q_D(CtrlConfigWindow);
	// Check if this is a multitap port.
	bool isTP = false;
	switch (virtPort) {
		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:
		case IoManager::VIRTPORT_TP1B:
		case IoManager::VIRTPORT_TP1C:
		case IoManager::VIRTPORT_TP1D:
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_TEAMPLAYER)
				isTP = true;
			break;

		// EA 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE &&
			    d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER)
				isTP = true;
			break;

		// Master Tap, Port 1.
		case IoManager::VIRTPORT_MTAP1A:
		case IoManager::VIRTPORT_MTAP1B:
		case IoManager::VIRTPORT_MTAP1C:
		case IoManager::VIRTPORT_MTAP1D:
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_MASTERTAP)
				isTP = true;
			break;

		// Master Tap, Port 2.
		case IoManager::VIRTPORT_MTAP2A:
		case IoManager::VIRTPORT_MTAP2B:
		case IoManager::VIRTPORT_MTAP2C:
		case IoManager::VIRTPORT_MTAP2D:
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_MASTERTAP)
				isTP = true;
			break;

		default:
			// Other port.
			break;
	}

	// Device setting is valid.

	// Make sure the dropdown index is set properly to reduce flicker.
	d->cboDevice_lock();
	const int ioType = (int)d->keyManager->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	if (ioType < d->ui.cboDevice->count())
		d->ui.cboDevice->setCurrentIndex(ioType);
	d->cboDevice_unlock();

	// Update the port settings.
	d->selPort = virtPort;
	d->initCboDevice(isTP);
	updatePortSettings(virtPort);	// Update the port settings.
}


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
	Q_D(CtrlConfigWindow);
	d->keyManager->copyFrom(*gqt4_cfg->m_keyManager);

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
			// Make sure Port 1 is still Team Player.
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_TEAMPLAYER) {
				// Port 1 is no longer Team Player.
				// Switch to Port 1 instead of the TP1 port.
				d->ui.actionPort1->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:
		case IoManager::VIRTPORT_TP2B:
		case IoManager::VIRTPORT_TP2C:
		case IoManager::VIRTPORT_TP2D:
			// Make sure Port 2 is still Team Player.
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_TEAMPLAYER) {
				// Port 2 is no longer Team Player.
				// Switch to Port 2 instead of the TP2 port.
				d->ui.actionPort2->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// EA 4-Way Play.
		case IoManager::VIRTPORT_4WPA:
		case IoManager::VIRTPORT_4WPB:
		case IoManager::VIRTPORT_4WPC:
		case IoManager::VIRTPORT_4WPD:
			// Make sure Port 1 is still 4WP slave,
			// and Port 2 is still 4WP master.
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_4WP_SLAVE ||
			    d->keyManager->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_4WP_MASTER) {
				// 4WP is no longer set. Switch to Port 1.
				d->ui.actionPort1->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// Master Tap, Port 1.
		case IoManager::VIRTPORT_MTAP1A:
		case IoManager::VIRTPORT_MTAP1B:
		case IoManager::VIRTPORT_MTAP1C:
		case IoManager::VIRTPORT_MTAP1D:
			// Make sure Port 1 is still Master Tap.
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_MASTERTAP) {
				// Port 1 is no longer Master Tap.
				// Switch to Port 1 instead of the MTAP1 port.
				d->ui.actionPort1->setChecked(true);
				hasUpdatedPortSettings = true;
			}
			break;

		// Master Tap, Port 2.
		case IoManager::VIRTPORT_MTAP2A:
		case IoManager::VIRTPORT_MTAP2B:
		case IoManager::VIRTPORT_MTAP2C:
		case IoManager::VIRTPORT_MTAP2D:
			// Make sure Port 2 is still Master Tap.
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_MASTERTAP) {
				// Port 2 is no longer Master Tap.
				// Switch to Port 2 instead of the MTAP2 port.
				d->ui.actionPort2->setChecked(true);
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
	Q_D(CtrlConfigWindow);
	gqt4_cfg->m_keyManager->copyFrom(*d->keyManager);

	// I/O Manager is updated once per frame.
	// TODO: Send a message to the emulation queue to force an update
	// if emulation is running?
}

/** Widget slots. **/

/**
 * The selected port in the toolbar has been changed.
 * @param virtPort Port number.
 */
void CtrlConfigWindow::toolbarPortSelected(int virtPort)
{
	Q_D(CtrlConfigWindow);
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
	// TODO: Make this function a wrapper for CtrlConfigWindowPrivate?
	Q_D(CtrlConfigWindow);
	if (d->isCboDeviceLocked())
		return;

	const IoManager::VirtPort_t virtPort = d->selPort;
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	// Check if the device type has been changed.
	const IoManager::IoType_t newIoType = d->cboDeviceIoType(index);
	assert(newIoType >= IoManager::IOT_NONE && newIoType < IoManager::IOT_MAX);
	const IoManager::IoType_t ioType = d->keyManager->ioType(virtPort);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);

	// Don't do anything if the device type hasn't actually changed.
	if (ioType == newIoType)
		return;

	// Check for 4WP.
	if (d->selPort == IoManager::VIRTPORT_1) {
		// Port 1.
		if (newIoType == IoManager::IOT_4WP_MASTER) {
			// 4WP SLAVE set for Port 1.
			// (4WP_MASTER index used for dropdown.)
			d->keyManager->setIoType(d->selPort, IoManager::IOT_4WP_SLAVE);

			// Make sure Port 2 is set to 4WP MASTER.
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) != IoManager::IOT_4WP_MASTER) {
				d->keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_4WP_MASTER);
				updatePortButton(IoManager::VIRTPORT_2);
			}
		} else {
			// 4WP SLAVE not set for Port 1.
			d->keyManager->setIoType(d->selPort, newIoType);
			
			// Check if Port 2 is set to 4WP MASTER. (or 4WP SLAVE for sanity check)
			if (d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_MASTER ||
			    d->keyManager->ioType(IoManager::VIRTPORT_2) == IoManager::IOT_4WP_SLAVE)
			{
				// Port 2 is set to 4WP MASTER. Unset it.
				d->keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
				updatePortButton(IoManager::VIRTPORT_2);
			}
		}
	} else if (d->selPort == IoManager::VIRTPORT_2) {
		// Port 2.
		if (newIoType == IoManager::IOT_4WP_MASTER) {
			// 4WP MASTER set for Port 2.
			d->keyManager->setIoType(d->selPort, IoManager::IOT_4WP_MASTER);
			
			// Make sure Port 1 is set to 4WP SLAVE.
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) != IoManager::IOT_4WP_SLAVE) {
				d->keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_4WP_SLAVE);
				updatePortButton(IoManager::VIRTPORT_1);
			}
		} else {
			// 4WP MASTER not set for Port 2.
			d->keyManager->setIoType(d->selPort, newIoType);
			
			// Check if Port 1 is set to 4WP SLAVE. (or 4WP MASTER for sanity check)
			if (d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_SLAVE ||
			    d->keyManager->ioType(IoManager::VIRTPORT_1) == IoManager::IOT_4WP_MASTER)
			{
				// Port 1 is set to 4WP SLAVE. Unset it.
				d->keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_NONE);
				updatePortButton(IoManager::VIRTPORT_1);
			}
		}
	} else {
		// Other port.
		d->keyManager->setIoType(d->selPort, newIoType);
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
	Q_D(CtrlConfigWindow);
	QVector<GensKey_t> keyMap = d->ui.ctrlCfgWidget->keyMap();
	d->keyManager->setKeyMap(d->selPort, keyMap.constData(), keyMap.size());
}

}
