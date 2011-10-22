/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigStore.hpp: Configuration store.                                   *
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

#ifndef __GENS_QT4_CONFIG_CONFIGSTORE_HPP__
#define __GENS_QT4_CONFIG_CONFIGSTORE_HPP__

// Qt includes.
#include <QtCore/QObject>

namespace GensQt4
{

class ConfigStorePrivate;

class ConfigStore : public QObject
{
	Q_OBJECT
	
	public:
		ConfigStore(QObject *parent = 0);
		
		/**
		 * Reset all settings to defaults.
		 */
		void reset(void);
		
		/**
		 * Set a property.
		 * @param key Property name.
		 * @param value Property value.
		 */
		void set(const QString& key, const QVariant& value);
		
		/**
		 * Get a property.
		 * @param key Property name.
		 * @return Property value.
		 */
		QVariant get(const QString& key);
		
		/**
		 * Get a property.
		 * Converts hexadecimal string values to unsigned int.
		 * @param key Property name.
		 * @return Property value.
		 */
		unsigned int getUInt(const QString& key);
		
		/**
		 * Load the configuration file.
		 * @param filename Configuration filename.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QString& filename);
		
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
		int save(const QString& filename);
		
		/**
		 * Save the configuration file.
		 * No filename specified; use the default filename.
		 * @return 0 on success; non-zero on error.
		 */
		int save(void);
		
		/**
		 * Register an object for property change notification.
		 * @param property Property to watch.
		 * @param object QObject to register.
		 * @param method Method name.
		 */
		void registerChangeNotification(const QString& property, QObject *object, const char *method);
		
		/**
		 * Unregister an object for property change notification.
		 * @param property Property to watch.
		 * @param object QObject to register.
		 * @param method Method name.
		 */
		void unregisterChangeNotification(const QString& property, QObject *object, const char *method);
		
		/**
		 * Notify all registered objects that configuration settings have changed.
		 * Useful when starting the emulator.
		 */
		void notifyAll(void);
		
		/**
		 * Get the configuration path.
		 * @return Configuration path.
		 */
		QString configPath(void);
	
	private:
		friend class ConfigStorePrivate;
		ConfigStorePrivate *d;
		Q_DISABLE_COPY(ConfigStore)
};

}

#endif /* __GENS_QT4_CONFIG_CONFIGSTORE_HPP__ */
