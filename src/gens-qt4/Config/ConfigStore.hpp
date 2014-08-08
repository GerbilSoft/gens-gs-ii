/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigStore.hpp: Configuration store.                                   *
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

#ifndef __GENS_QT4_CONFIG_CONFIGSTORE_HPP__
#define __GENS_QT4_CONFIG_CONFIGSTORE_HPP__

// LibGens includes.
#include "libgens/Rom.hpp"

// LibGensKeys. (TODO: Save from KeyManager)
#include "libgenskeys/GensKey_t.h"

// Qt includes.
#include <QtCore/QObject>

// Configuration classes.
#include "RecentRoms.hpp"
#include "PathConfig.hpp"
#include "libgenskeys/KeyManager.hpp"

namespace GensQt4
{

class ConfigStorePrivate;

class ConfigStore : public QObject
{
	Q_OBJECT

	public:
		ConfigStore(QObject *parent = 0);
		~ConfigStore();

	private:
		ConfigStorePrivate *const d_ptr;
		Q_DECLARE_PRIVATE(ConfigStore)
	private:
		Q_DISABLE_COPY(ConfigStore)

	public:
		/**
		 * Reset all settings to defaults.
		 */
		void reset(void);

		/**
		 * Set a property.
		 * @param key Property name.
		 * @param value Property value.
		 */
		void set(const QString &key, const QVariant &value);

		/**
		 * Get a property.
		 * @param key Property name.
		 * @return Property value.
		 */
		QVariant get(const QString &key) const;

		/**
		 * Get a property.
		 * Converts hexadecimal string values to unsigned int.
		 * @param key Property name.
		 * @return Property value.
		 */
		unsigned int getUInt(const QString &key) const;

		/**
		 * Get a property.
		 * Converts hexadecimal string values to signed int.
		 * @param key Property name.
		 * @return Property value.
		 */
		int getInt(const QString &key) const;

		/**
		 * Load the configuration file.
		 * @param filename Configuration filename.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QString &filename);

		/**
		 * Load the configuration file.
		 * No filename specified; use the default filename.
		 * @return 0 on success; non-zero on error.
		 */
		int load(void);

		/**
		 * Save the configuration file.
		 * @param filename Filename.
		 * @return 0 on success; non-zero on error.
		 */
		int save(const QString &filename) const;

		/**
		 * Save the configuration file.
		 * No filename specified; use the default filename.
		 * @return 0 on success; non-zero on error.
		 */
		int save(void) const;

		/**
		 * Register an object for property change notification.
		 * @param property Property to watch.
		 * @param object QObject to register.
		 * @param method Method name.
		 */
		void registerChangeNotification(const QString &property, QObject *object, const char *method);

		/**
		 * Unregister an object for property change notification.
		 * @param property Property to watch.
		 * @param object QObject to register.
		 * @param method Method name.
		 */
		void unregisterChangeNotification(const QString &property, QObject *object, const char *method);

		/**
		 * Notify all registered objects that configuration settings have changed.
		 * Useful when starting the emulator.
		 */
		void notifyAll(void);

		/** Configuration Paths. **/

		/**
		 * Get the main configuration path. (GCPATH_CONFIG)
		 * @return Main configuration path.
		 */
		QString configPath(void) const;

		/**
		 * Get the specified configuration path.
		 * @param path Configuration path to get. (Invalid paths act like GCPATH_CONFIG.)
		 * @return Configuration path.
		 */
		QString configPath(PathConfig::ConfigPath path) const;

		/**
		 * Get a const pointer to the PathConfig object.
		 * @return Const pointer to the PathConfig object.
		 */
		const PathConfig *pathConfigObject(void) const;

		/** Recent ROMs. **/

		/**
		 * Update the Recent ROMs list.
		 * @param filename ROM filename.
		 * @param z_filename Filename of ROM within archive.
		 * @param sysId System ID.
		 */
		void recentRomsUpdate(const QString &filename,
				      const QString &z_filename,
				      LibGens::Rom::MDP_SYSTEM_ID sysId);

		/**
		 * Get a const pointer to the Recent ROMs object.
		 * @return Const pointer to the Recent ROMs object.
		 */
		const RecentRoms *recentRomsObject(void) const;

		/**
		 * Get a Recent ROMs entry.
		 * @param id Recent ROM ID.
		 */
		RecentRom_t recentRomsEntry(int id) const;

		/** Key configuration. **/

		/**
		 * Get the action associated with a GensKey_t.
		 * @param key GensKey_t.
		 * @return Action ID.
		 */
		int keyToAction(GensKey_t key) const;

		/**
		 * Get the GensKey_t associated with an action.
		 * @param actoin Action ID.
		 * @return GensKey_t.
		 */
		GensKey_t actionToKey(int action) const;

	public:
		// Key Manager.
		// TODO: Rework this so it isn't public.
		LibGensKeys::KeyManager *m_keyManager;
};

}

#endif /* __GENS_QT4_CONFIG_CONFIGSTORE_HPP__ */
