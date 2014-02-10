/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfig.hpp: Controller configuration.                               *
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

#ifndef __GENS_QT4_CTRLCONFIG_HPP__
#define __GENS_QT4_CTRLCONFIG_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
#include <QtCore/QVector>
class QSettings;

// LibGens includes.
#include "libgens/IO/IoManager.hpp"
#include "libgens/GensInput/GensKey_t.h"

namespace GensQt4
{

class CtrlConfigPrivate;

class CtrlConfig : public QObject
{
	Q_OBJECT

	public:
		CtrlConfig(QObject *parent = 0);
		CtrlConfig(const CtrlConfig *src, QObject *parent = 0);
		virtual ~CtrlConfig();

	private:
		CtrlConfigPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(CtrlConfig)
	private:
		Q_DISABLE_COPY(CtrlConfig)

	public:
		/**
		 * Copy settings from another CtrlConfig.
		 * TODO: Make this a copy constructor.
		 * @param src Other CtrlConfig to copy from.
		 */
		void copyFrom(const CtrlConfig *src);

		// Dirty flag.
		bool isDirty(void) const;
		void clearDirty(void);

		/**
		 * Load controller configuration from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QSettings *qSettings);

		/**
		 * Save controller configuration to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int save(QSettings *qSettings) const;

		/**
		 * Update the controller I/O manager.
		 * @param ioManager I/O manager class.
		 */
		void updateIoManager(LibGens::IoManager *ioManager) const;

		/** CtrlConfigWindow interface. **/

		/**
		 * Get a controller's I/O device type.
		 * @param virtPort Virtual controller port.
		 * @return Device type.
		 */
		LibGens::IoManager::IoType_t ioType(LibGens::IoManager::VirtPort_t virtPort);

		/**
		 * Set a controller's I/O device type.
		 * NOTE: IoManager should be updated after calling this function.
		 * @param virtPort Virtual controller port.
		 * @return Device type.
		 */
		void setIoType(LibGens::IoManager::VirtPort_t virtPort, LibGens::IoManager::IoType_t ioType);


		/**
		 * Get a controller's keymap.
		 * @param virtPort Virtual controller port.
		 * @return Keymap.
		 */
		QVector<GensKey_t> keyMap(LibGens::IoManager::VirtPort_t virtPort);

		/**
		 * Set a controller's keymap.
		 * NOTE: IoManager should be updated after calling this function.
		 * @param virtPort Virtual controller port.
		 * @param keyMap New keymap.
		 */
		void setKeyMap(LibGens::IoManager::VirtPort_t virtPort, QVector<GensKey_t> keyMap);
};

}

#endif /* __GENS_QT4_CTRLCONFIG_HPP__ */
