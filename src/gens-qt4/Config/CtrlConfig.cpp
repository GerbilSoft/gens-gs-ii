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

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QLatin1String>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

// ARRAY_SIZE()
#include "libgens/macros/common.h"

namespace GensQt4
{

class CtrlConfigPrivate
{
	public:
		CtrlConfigPrivate(CtrlConfig *q);
		CtrlConfigPrivate(CtrlConfig *q, const CtrlConfigPrivate *src);
		void copyFrom(const CtrlConfigPrivate *src);

	private:
		CtrlConfig *const q_ptr;
		Q_DECLARE_PUBLIC(CtrlConfig)
	private:
		Q_DISABLE_COPY(CtrlConfigPrivate)

	public:
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
		int save(QSettings *qSettings) const;

	private:
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
	KEYV_e, KEYV_w, KEYV_q,
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

CtrlConfigPrivate::CtrlConfigPrivate(CtrlConfig *q)
	: q_ptr(q)
	, m_dirty(true)
{
	// Clear the controller types and key configuration arrays.
	// TODO: Load defaults.
	memset(ctrlTypes, 0x00, sizeof(ctrlTypes));
	memset(ctrlKeys, 0x00, sizeof(ctrlKeys));
}

CtrlConfigPrivate::CtrlConfigPrivate(CtrlConfig *q, const CtrlConfigPrivate *src)
	: q_ptr(q)
{
	// Copy from another CtrlConfigPrivate.
	copyFrom(src);
}

/**
 * Copy from another CtrlConfigPrivate to this CtrlConfigPrivate.
 * NOTE: Does not copy 'q'.
 * @param src Other CtrlConfigPrivate to copy from.
 */
void CtrlConfigPrivate::copyFrom(const CtrlConfigPrivate *src)
{
	m_dirty = src->m_dirty;
	memcpy(ctrlTypes, src->ctrlTypes, sizeof(ctrlTypes));
	memcpy(ctrlKeys, src->ctrlKeys, sizeof(ctrlKeys));
}

/**
 * Check if the controller configuration is dirty.
 * @return True if the controller configuration is dirty; false otherwise.
 */
inline bool CtrlConfigPrivate::isDirty(void) const
	{ return m_dirty; }

/**
 * Clear the dirty flag.
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
		// Get the controller type. (Stored as FourCC.)
		const QString portName = PortName((IoManager::VirtPort_t)virtPort);
		const QString qsFourCC = qSettings->value(portName + QLatin1String("/type")).toString();
		IoManager::IoType_t ioType_tmp = IoManager::StringToIoType(qsFourCC.toStdString());

		// Check if the controller type is valid.
		bool isValidCtrl = true;
		if (ioType_tmp < IoManager::IOT_NONE ||
		    ioType_tmp >= IoManager::IOT_MAX) {
			// No controller information.
			isValidCtrl = false;
		} else if (virtPort > IoManager::VIRTPORT_EXT &&
			   ioType_tmp > IoManager::IOT_6BTN) {
			// Team Player / 4WP doesn't support this controller.
			isValidCtrl = false;
		} else if (!IoManager::IsDevTypeUsable(ioType_tmp)) {
			// Device type isn't usable in this build.
			isValidCtrl = false;
		}

		if (!isValidCtrl) {
			// Controller information is invalid.
			// Use the default settings.
			ctrlTypes[virtPort] = Def_CtrlTypes[virtPort];
			memcpy(ctrlKeys[virtPort], Def_CtrlKeys[virtPort], sizeof(ctrlKeys[virtPort]));
		} else {
			// Controller information specified.
			ctrlTypes[virtPort] = ioType_tmp;

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
				ctrlKeys[virtPort][j] = keyData.at(j).toUInt(nullptr, 0);
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
int CtrlConfigPrivate::save(QSettings *qSettings) const
{
	for (int virtPort = IoManager::VIRTPORT_1;
	     virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		// Save the controller type as a FourCC.
		const QString portName = PortName((IoManager::VirtPort_t)virtPort);
		const QString qsFourCC = QString::fromStdString(IoManager::IoTypeToString(ctrlTypes[virtPort]));
		qSettings->setValue(portName + QLatin1String("/type"), qsFourCC);

		// Save the controller keys.
		// TODO: Save all keys, even those not being used by the current type.
		// Trim all 0 buttons from the keys afterwards.
		int numButtons = IoManager::NumDevButtons(ctrlTypes[virtPort]);
		if (numButtons > ARRAY_SIZE(ctrlKeys[virtPort]))
			numButtons = ARRAY_SIZE(ctrlKeys[virtPort]);

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
	, d_ptr(new CtrlConfigPrivate(this))
{ }

CtrlConfig::CtrlConfig(const CtrlConfig *src, QObject *parent)
	: QObject(parent)
	, d_ptr(new CtrlConfigPrivate(this))
{
	// Copy the CtrlConfigPrivate from the specified source.
	// (Copy constructors don't work too well with Qt.)
	d_ptr->copyFrom(src->d_ptr);
}

CtrlConfig::~CtrlConfig()
{
	delete d_ptr;
}

/**
 * Copy settings from another CtrlConfig.
 * @param src Other CtrlConfig to copy from.
 */
void CtrlConfig::copyFrom(const CtrlConfig *src)
{
	Q_D(CtrlConfig);
	d->copyFrom(src->d_ptr);
}

/**
 * Check if the controller configuration is dirty.
 * WRAPPER FUNCTION for CtrlConfigPrivate::isDirty().
 * @return True if the controller configuration is dirty; false otherwise.
 */
bool CtrlConfig::isDirty(void) const
{
	Q_D(const CtrlConfig);
	return d->isDirty();
}

/**
 * Clear the dirty flag.
 * WRAPPER FUNCTION for CtrlConfigPrivate::clearDirty().
 */
void CtrlConfig::clearDirty(void)
{
	Q_D(CtrlConfig);
	d->clearDirty();
}

/**
 * Load controller configuration from a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::load(const QSettings *qSettings)
{
	Q_D(CtrlConfig);
	return d->load(qSettings);
}

/**
 * Save controller configuration to a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::save(QSettings *qSettings) const
{
	Q_D(const CtrlConfig);
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
	Q_D(const CtrlConfig);
	for (int virtPort = 0; virtPort < ARRAY_SIZE(d->ctrlKeys); virtPort++) {
		// Set the device type.
		ioManager->setDevType(
				(IoManager::VirtPort_t)virtPort, 
				d->ctrlTypes[virtPort]);

		// Set the new keymaps.
		ioManager->setKeymap(virtPort,
				&d->ctrlKeys[virtPort][0],
				ARRAY_SIZE(d->ctrlKeys[virtPort]));
	}
}

/** CtrlConfigWindow interface. **/

/**
 * Get a controller's I/O device type.
 * @param virtPort Virtual controller port.
 * @return Device type.
 */
IoManager::IoType_t CtrlConfig::ioType(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	Q_D(CtrlConfig);
	return d->ctrlTypes[virtPort];
}

/**
 * Set a controller's I/O device type.
 * NOTE: IoManager should be updated after calling this function.
 * @param virtPort Virtual controller port.
 * @return Device type.
 */
void CtrlConfig::setIoType(IoManager::VirtPort_t virtPort, IoManager::IoType_t ioType)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	Q_D(CtrlConfig);
	d->ctrlTypes[virtPort] = ioType;
}

/**
 * Get a controller's keymap.
 * @param virtPort Virtual controller port.
 * @return Keymap.
 */
QVector<GensKey_t> CtrlConfig::keyMap(IoManager::VirtPort_t virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	Q_D(CtrlConfig);
	const int numButtons = IoManager::NumDevButtons(d->ctrlTypes[virtPort]);

	QVector<GensKey_t> keyMap(numButtons);
	for (int i = 0; i < numButtons; i++) {
		keyMap[i] = d->ctrlKeys[virtPort][i];
	}

	return keyMap;
}

/**
 * Set a controller's keymap.
 * NOTE: IoManager should be updated after calling this function.
 * @param virtPort Virtual controller port.
 * @param keyMap New keymap.
 */
void CtrlConfig::setKeyMap(LibGens::IoManager::VirtPort_t virtPort, QVector<GensKey_t> keyMap)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	Q_D(CtrlConfig);
	const int numButtons = IoManager::NumDevButtons(d->ctrlTypes[virtPort]);
	const int maxButtons = std::min(keyMap.size(), numButtons);

	for (int i = 0; i < maxButtons; i++) {
		d->ctrlKeys[virtPort][i] = keyMap[i];
	}

	if (maxButtons < numButtons) {
		// Clear the rest of the keys.
		for (int i = maxButtons; i < numButtons; i++) {
			d->ctrlKeys[virtPort][i] = 0;
		}
	}
}

}
