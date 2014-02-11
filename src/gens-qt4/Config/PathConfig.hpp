/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * PathConfig.hpp: Path configuration.                                     *
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

#ifndef __GENS_QT4_CONFIG_PATHCONFIG_HPP__
#define __GENS_QT4_CONFIG_PATHCONFIG_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSettings>

namespace GensQt4
{

class PathConfigPrivate;

class PathConfig : public QObject
{
	Q_OBJECT
	
	public:
		PathConfig(QObject *parent = 0);
		virtual ~PathConfig();
	
	private:
		PathConfigPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(PathConfig)
	private:
		Q_DISABLE_COPY(PathConfig)
	
	public:
		/** QSettings functions. **/
		
		/**
		 * Load the configuration paths from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return Number of paths loaded on success; negative on error.
		 */
		int load(const QSettings *qSettings);
		
		/**
		 * Save the configuration paths to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return Number of paths saved on success; negative on error.
		 */
		int save(QSettings *qSettings) const;

	public:
		/**
		 * Configuration path.
		 * TODO: Add support for plugin configuration paths.
		 */
		enum ConfigPath
		{
			GCPATH_CONFIG		= 0,	// Same as cfgPath.
			GCPATH_SAVESTATES	= 1,	// Savestates.
			GCPATH_SRAM		= 2,	// Cartridge SRAM.
			GCPATH_BRAM		= 3,	// Sega CD BRAM.
			GCPATH_WAV		= 4,	// WAV dumping.
			GCPATH_VGM		= 5,	// VGM dumping.
			GCPATH_SCREENSHOTS	= 6,	// Screenshots.
			
			GCPATH_MAX
		};
		Q_ENUMS(GensQt4::PathConfig::ConfigPath)
		
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
		QString configPath(ConfigPath path) const;
	
	signals:
		// NOTE: Signals must use the full namespace specification
		// of enums in order to get moc to handle them properly.
		/**
		 * A configuration path has been changed.
		 * This signal *is* emitted on load().
		 * @param path Configuration path.
		 * @param dir New directory.
		 */
		void pathChanged(GensQt4::PathConfig::ConfigPath path, const QString &dir);
};

}

#endif /* __GENS_QT4_CONFIG_PATHCONFIG_HPP__ */
