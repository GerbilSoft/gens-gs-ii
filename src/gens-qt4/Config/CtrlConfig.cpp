/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfig.cpp: Controller configuration.                               *
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

#include "CtrlConfig.hpp"

// Controller I/O manager.
#include "libgens/IoManager.hpp"
using LibGens::IoManager;

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QLatin1String>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace GensQt4
{

class CtrlConfigPrivate
{
	public:
		CtrlConfigPrivate(CtrlConfig *q);

		// Dirty flag.
		bool isDirty(void) const;
		void clearDirty(void);

		// Controller types.
		IoManager::IoType_t ctrlTypes[IoManager::VIRTPORT_MAX];

		// Key configuration.
		// TODO: Use next-highest power-of-two?
		GensKey_t ctrlKeys[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX];

		// Get an internal port name. (non-localized)
		static QString PortName(IoManager::VirtPort_t virtPort);

		// Load/Save functions.
		int load(const QSettings *qSettings);
		int save(QSettings *qSettings);

	private:
		CtrlConfig *const q;
		Q_DISABLE_COPY(CtrlConfigPrivate)

		// Dirty flag.
		bool m_dirty;

		// Key value separator in the config file.
		static const char chrKeyValSep = ':';

		// Default controller configuration.
		static const IoManager::IoType_t Def_CtrlTypes[IoManager::VIRTPORT_MAX];
		static const GensKey_t Def_CtrlKeys[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX];
};

/********************************
 * CtrlConfigPrivate functions. *
 ********************************/

// Default controller configuration.
const IoManager::IoType_t CtrlConfigPrivate::Def_CtrlTypes[IoManager::VIRTPORT_MAX] =
{
	// System controller ports.
	IoManager::IOT_6BTN,	// Port 1
	IoManager::IOT_3BTN,	// Port 2
	
	// Team Player, Port 1.
	IoManager::IOT_NONE,	// Port TP1A
	IoManager::IOT_NONE,	// Port TP1B
	IoManager::IOT_NONE,	// Port TP1C
	IoManager::IOT_NONE,	// Port TP1D
	
	// Team Player, Port 2.
	IoManager::IOT_NONE,	// Port TP2A
	IoManager::IOT_NONE,	// Port TP2B
	IoManager::IOT_NONE,	// Port TP2C
	IoManager::IOT_NONE,	// Port TP2D
	
	// 4-Way Play.
	IoManager::IOT_NONE,	// Port 4WPA
	IoManager::IOT_NONE,	// Port 4WPB
	IoManager::IOT_NONE,	// Port 4WPC
	IoManager::IOT_NONE,	// Port 4WPD
};

const GensKey_t CtrlConfigPrivate::Def_CtrlKeys[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX] =
{
	// Port 1
	// NOTE: Both shift keys are mapped to LSHIFT on Mac OS X.
	{KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,
	KEYV_s, KEYV_d, KEYV_a, KEYV_RETURN,
	KEYV_q, KEYV_w, KEYV_e,
#ifdef Q_WS_MAC
	KEYV_LSHIFT
#else
	KEYV_RSHIFT
#endif
	},
	
	// Port 2 (TODO: This needs to be improved!)
	{KEYV_i, KEYV_k, KEYV_j, KEYV_l,
	KEYV_t, KEYV_y, KEYV_r, KEYV_u,
	0, 0, 0, 0},
	
	// Other ports are left undefined.
	
	// Team Player, Port 1.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1A
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1B
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1C
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1D
	
	// Team Player, Port 2.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2A
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2B
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2C
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2D
	
	// 4-Way Play.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPA
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPC
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPD
};


CtrlConfigPrivate::CtrlConfigPrivate(CtrlConfig* q)
	: q(q)
	, m_dirty(true)
{
	// Clear the controller types and key configuration arrays.
	// TODO: Load defaults.
	memset(ctrlTypes, 0x00, sizeof(ctrlTypes));
	memset(ctrlKeys, 0x00, sizeof(ctrlKeys));
}


/**
 * CtrlConfigPrivate::isDirty(): Check if the controller configuration is dirty.
 * @return True if the controller configuration is dirty; false otherwise.
 */
inline bool CtrlConfigPrivate::isDirty(void) const
	{ return m_dirty; }

/**
 * CtrlConfigPrivate::clearDirty(): Clear the dirty flag.
 */
inline void CtrlConfigPrivate::clearDirty(void)
	{ m_dirty = false; }


/**
 * Get an internal port name. (non-localized)
 * @param port Port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigPrivate::PortName(IoManager::VirtPort_t virtPort)
{
	switch (virtPort) {
		// System controller ports.
		case IoManager::VIRTPORT_1:		return QLatin1String("port1");
		case IoManager::VIRTPORT_2:		return QLatin1String("port2");
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


/**
 * Load controller configuration from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfigPrivate::load(const QSettings *qSettings)
{
	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		// Get the controller type.
		// TODO: Allow ASCII controller types?
		const QString portName = PortName((IoManager::VirtPort_t)virtPort);
		IoManager::IoType_t ioType_tmp =
				(IoManager::IoType_t)
				(qSettings->value(portName + QLatin1String("/type"), -1).toInt());
		if (ioType_tmp < IoManager::IOT_NONE ||
		    ioType_tmp >= IoManager::IOT_MAX) {
			// No controller information.
			// Use the default.
			ctrlTypes[virtPort] = Def_CtrlTypes[virtPort];
			memcpy(ctrlKeys[virtPort], Def_CtrlKeys[virtPort], sizeof(ctrlKeys[virtPort]));
		} else {
			// Controller information specified.
			ctrlTypes[virtPort] = (IoManager::IoType_t)ioType_tmp;

			// Clear the controller keys.
			memset(ctrlKeys[virtPort], 0x00, sizeof(ctrlKeys[virtPort]));

			// Read the controller keys from the configuration file.
			const QStringList keyData =
				qSettings->value(portName + QLatin1String("/keys"),
				QString()).toString().split(QChar((uint16_t)chrKeyValSep));

			int numButtons = IoManager::NumDevButtons(ctrlTypes[virtPort]);
			if (numButtons > keyData.size())
				numButtons = keyData.size();

			// Copy the controller keys into ctrlKeys[].
			for (int j = numButtons - 1; j >= 0; j--)
				ctrlKeys[virtPort][j] = keyData.at(j).toUInt(NULL, 0);
		}
	}

	// Controller configuration loaded.
	m_dirty = true;
	return 0;
}


/**
 * Save controller configuration to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfigPrivate::save(QSettings *qSettings)
{
	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		// Save the controller type.
		// TODO: Allow ASCII controller types?
		const QString portName = PortName((IoManager::VirtPort_t)virtPort);
		qSettings->setValue(portName + QLatin1String("/type"), (int)ctrlTypes[virtPort]);

		// Save the controller keys.
		// TODO: Save all keys, even those not being used by the current type.
		// Trim all 0 buttons from the keys afterwards.
		int numButtons = IoManager::NumDevButtons(ctrlTypes[virtPort]);
		if (numButtons > NUM_ELEMENTS(ctrlKeys[virtPort]))
			numButtons = NUM_ELEMENTS(ctrlKeys[virtPort]);

		// Write the buttons to the configuration file.
		QString keyData;
		QString keyHex;
		for (int j = 0; j < numButtons; j++) {
			if (j > 0)
				keyData += QChar((uint16_t)chrKeyValSep);
			keyHex = QString::number(ctrlKeys[virtPort][j], 16).toUpper();
			keyData += QLatin1String("0x");
			if (ctrlKeys[virtPort][j] <= 0xFFFF)
				keyData += keyHex.rightJustified(4, QChar(L'0'));
			else
				keyData += keyHex.rightJustified(8, QChar(L'0'));
		}

		qSettings->setValue(portName + QLatin1String("/keys"), keyData);
	}

	// Controller configuration saved.
	return 0;
}


/*************************
 * CtrlConfig functions. *
 *************************/

CtrlConfig::CtrlConfig(QObject *parent)
	: QObject(parent)
	, d(new CtrlConfigPrivate(this))
{ }

CtrlConfig::~CtrlConfig()
{
	delete d;
}


/**
 * CtrlConfig::isDirty(): Check if the controller configuration is dirty.
 * WRAPPER FUNCTION for CtrlConfigPrivate::isDirty().
 * @return True if the controller configuration is dirty; false otherwise.
 */
bool CtrlConfig::isDirty(void) const
	{ return d->isDirty(); }

/**
 * CtrlConfig::clearDirty(): Clear the dirty flag.
 * WRAPPER FUNCTION for CtrlConfigPrivate::clearDirty().
 */
void CtrlConfig::clearDirty(void)
	{ d->clearDirty(); }


/**
 * Load controller configuration from a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::load(const QSettings *qSettings)
{
	return d->load(qSettings);
}


/**
 * Save controller configuration to a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::save(QSettings *qSettings)
{
	return d->save(qSettings);
}


/** Controller update functions. **/


/**
 * Update the controller I/O manager.
 * @param ioManager I/O manager class.
 * @param virtPort Virtual port number.
 */
void CtrlConfig::updateIoManager(IoManager *ioManager) const
{
	for (int virtPort = 0; virtPort < NUM_ELEMENTS(d->ctrlKeys); virtPort++) {
		// Set the device type.
		ioManager->setDevType(
				(IoManager::VirtPort_t)virtPort, 
				d->ctrlTypes[virtPort]);

		// Set the new keymaps.
		ioManager->setKeymap(virtPort,
				&d->ctrlKeys[virtPort][0],
				NUM_ELEMENTS(d->ctrlKeys[virtPort]));
	}
}

}

