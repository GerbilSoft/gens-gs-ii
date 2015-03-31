/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfig.cpp: Controller configuration.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#include "CtrlConfig.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Controller I/O manager.
#include "libgens/IO/IoManager.hpp"
using LibGens::IoManager;
#include "libgenskeys/KeyManager.hpp"
using LibGensKeys::KeyManager;

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QStringList>

// ARRAY_SIZE()
#include "libgens/macros/common.h"

namespace GensQt4
{

class CtrlConfigPrivate
{
	// Static class.

	private:
		CtrlConfigPrivate() { }
		~CtrlConfigPrivate() { }
		Q_DISABLE_COPY(CtrlConfigPrivate)

	public:
		// Controller types.
		// Get an internal port name. (non-localized)
		static QString PortName(IoManager::VirtPort_t virtPort);

		// Key value separator in the config file.
		static const char chrKeyValSep = ':';
};

/********************************
 * CtrlConfigPrivate functions. *
 ********************************/

/**
 * Get an internal port name. (non-localized)
 * @param port Port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigPrivate::PortName(IoManager::VirtPort_t virtPort)
{
	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:	return QLatin1String("port1");
		case IoManager::VIRTPORT_2:	return QLatin1String("port2");
		case IoManager::VIRTPORT_EXT:	return QLatin1String("portEXT");

		// Team Player, Port 1.
		case IoManager::VIRTPORT_TP1A:	return QLatin1String("portTP1A");
		case IoManager::VIRTPORT_TP1B:	return QLatin1String("portTP1B");
		case IoManager::VIRTPORT_TP1C:	return QLatin1String("portTP1C");
		case IoManager::VIRTPORT_TP1D:	return QLatin1String("portTP1D");

		// Team Player, Port 2.
		case IoManager::VIRTPORT_TP2A:	return QLatin1String("portTP2A");
		case IoManager::VIRTPORT_TP2B:	return QLatin1String("portTP2B");
		case IoManager::VIRTPORT_TP2C:	return QLatin1String("portTP2C");
		case IoManager::VIRTPORT_TP2D:	return QLatin1String("portTP2D");

		// 4-Way Play.
		case IoManager::VIRTPORT_4WPA:	return QLatin1String("port4WPA");
		case IoManager::VIRTPORT_4WPB:	return QLatin1String("port4WPB");
		case IoManager::VIRTPORT_4WPC:	return QLatin1String("port4WPC");
		case IoManager::VIRTPORT_4WPD:	return QLatin1String("port4WPD");

		// J_Cart
		case IoManager::VIRTPORT_JCART1:	return QLatin1String("portJCart1");
		case IoManager::VIRTPORT_JCART2:	return QLatin1String("portJCart2");

		default:
			// Unknown port.
			return QString();
	}

	// Should not get here...
	return QString();
}

/*************************
 * CtrlConfig functions. *
 *************************/

/**
 * Load controller configuration from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @param keyManager Key Manager to load settings into.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::load(const QSettings &qSettings, KeyManager *keyManager)
{
	IoManager::IoType_t ioType;
	GensKey_t keyMap[IoManager::BTNI_MAX];

	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		// Get the controller type. (Stored as FourCC.)
		const QString portName = CtrlConfigPrivate::PortName((IoManager::VirtPort_t)virtPort);
		const QString qsFourCC = qSettings.value(portName + QLatin1String("/type")).toString();
		ioType = IoManager::StringToIoType(qsFourCC.toStdString());

		// Check if the controller type is valid.
		bool isValidCtrl = true;
		if (ioType < IoManager::IOT_NONE ||
		    ioType >= IoManager::IOT_MAX) {
			// No controller information.
			isValidCtrl = false;
		} else if (virtPort >= IoManager::VIRTPORT_TP1A &&
			   virtPort <= IoManager::VIRTPORT_TP2A)
		{
			// Team Player supports NONE, 3BTN, 6BTN, and MOUS.
			if (ioType > IoManager::IOT_6BTN && ioType != IoManager::IOT_MEGA_MOUSE) {
				// Not supported.
				isValidCtrl = false;
			}
		} else if (virtPort > IoManager::VIRTPORT_EXT &&
			   ioType > IoManager::IOT_6BTN) {
			// Team Player / 4WP doesn't support this controller.
			isValidCtrl = false;
		} else if (!IoManager::IsDevTypeUsable(ioType)) {
			// Device type isn't usable in this build.
			isValidCtrl = false;
		}

		if (!isValidCtrl) {
			// Controller information is invalid.
			// Use the default settings.
			keyManager->resetKeyMap((IoManager::VirtPort_t)virtPort);
		} else {
			// Controller information specified.
			keyManager->setIoType((IoManager::VirtPort_t)virtPort, ioType);

			// Clear the controller keys.
			memset(keyMap, 0, sizeof(keyMap));

			// Read the controller keys from the configuration file.
			const QStringList keyData =
				qSettings.value(portName + QLatin1String("/keys"),
				QString()).toString().split(QChar((uint16_t)CtrlConfigPrivate::chrKeyValSep));

			int numButtons = IoManager::NumDevButtons(ioType);
			if (numButtons > keyData.size())
				numButtons = keyData.size();

			// Copy the controller keys into ctrlKeys[].
			for (int j = numButtons - 1; j >= 0; j--)
				keyMap[j] = keyData.at(j).toUInt(nullptr, 0);

			// Set the controller keys.
			keyManager->setKeyMap((IoManager::VirtPort_t)virtPort, keyMap, ARRAY_SIZE(keyMap));
		}
	}

	// Controller configuration loaded.
	return 0;
}

/**
 * Save controller configuration to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @param keyManager Key Manager to save settings from.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::save(QSettings &qSettings, const KeyManager *keyManager)
{
	IoManager::IoType_t ioType;
	GensKey_t keyMap[IoManager::BTNI_MAX];

	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		// Save the controller type as a FourCC.
		const QString portName = CtrlConfigPrivate::PortName((IoManager::VirtPort_t)virtPort);
		ioType = keyManager->ioType((IoManager::VirtPort_t)virtPort);
		const QString qsFourCC = QString::fromStdString(IoManager::IoTypeToString(ioType));
		qSettings.setValue(portName + QLatin1String("/type"), qsFourCC);

		// Save the controller keys.
		// TODO: Save all keys, even those not being used by the current type.
		// Trim all 0 buttons from the keys afterwards.
		keyManager->keyMap((IoManager::VirtPort_t)virtPort, keyMap, ARRAY_SIZE(keyMap));
		int numButtons = IoManager::NumDevButtons(ioType);
		if (numButtons > ARRAY_SIZE(keyMap))
			numButtons = ARRAY_SIZE(keyMap);

		// Write the buttons to the configuration file.
		QString keyData;
		QString keyHex;
		for (int j = 0; j < numButtons; j++) {
			if (j > 0)
				keyData += QChar((uint16_t)CtrlConfigPrivate::chrKeyValSep);
			keyHex = QString::number(keyMap[j], 16).toUpper();
			keyData += QLatin1String("0x");
			if (keyMap[j] <= 0xFFFF)
				keyData += keyHex.rightJustified(4, QChar(L'0'));
			else
				keyData += keyHex.rightJustified(8, QChar(L'0'));
		}

		qSettings.setValue(portName + QLatin1String("/keys"), keyData);
	}

	// Controller configuration saved.
	return 0;
}

}
