/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensConfig.hpp: Gens configuration.                                     *
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

#ifndef __GENS_QT4_GENSCONFIG_HPP__
#define __GENS_QT4_GENSCONFIG_HPP__

#warning Do not use GensConfig.hpp

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QColor>

// Key configuration.
#include "actions/GensKeyConfig.hpp"

// Controller configuration.
#include "CtrlConfig.hpp"

// Property function macro.
// NOTE: We can't include Q_PROPERTY() or signals here due to moc limitations.
#define GC_PROPERTY(propType, propName, setPropType, setPropName) \
		propType propName(void) const; \
		void set##setPropName(setPropType new##setPropName); \

namespace GensQt4
{

class GensConfigPrivate;

class GensConfig : public QObject
{
	Q_OBJECT
	
	public:
		GensConfig(QObject *parent = 0);
		~GensConfig();
		
		/**
		 * reload(): Load the user's configuration file.
		 * @param filename Filename. (If not specified, uses the default filename.)
		 * @return 0 on success; non-zero on error.
		 */
		int reload(void);
		int reload(const QString& filename);
		
		/**
		 * save(): Save the user's configuration file.
		 * @param filename Filename. (If not specified, uses the default filename.)
		 * @return 0 on success; non-zero on error.
		 */
		int save(void);
		int save(const QString& filename);
		
		/**
		 * emitAll(): Emit all configuration settings.
		 * Useful when starting the emulator.
		 */
		void emitAll(void);
		
	/** Properties. **/
		
		/** Configuration path. **/
		// TODO: Mark cfgPath as CONSTANT?
		// NOTE: This uses Qt directory separators.
		Q_PROPERTY(QString cfgPath READ cfgPath)
		QString cfgPath(void) const;
		
		/** User configuration paths. **/
		// TODO: Make these configurable?
		// NOTE: These use Qt directory separators.
		public:
			enum ConfigPath
			{
				GCPATH_CONFIG		= 0,	// Same as cfgPath.
				GCPATH_SAVESTATES	= 1,	// Savestates.
				GCPATH_SRAM		= 2,	// Cartridge SRAM.
				GCPATH_BRAM		= 3,	// Sega CD BRAM.
				GCPATH_WAV		= 4,	// WAV dumping.
				GCPATH_VGM		= 5,	// VGM dumping.
				GCPATH_SCREENSHOTS	= 6,	// Screemshots.
				
				GCPATH_MAX
			};
			QString userPath(ConfigPath pathID);
		
		/** Key configuration. **/
		int keyToAction(GensKey_t key);
		GensKey_t actionToKey(int action);
	
	signals:
		/** General settings. **/
		void autoFixChecksum_changed(bool newAutoFixChecksum);
		void autoPause_changed(bool newAutoPause);
	
	private:
		friend class GensConfigPrivate;
		GensConfigPrivate *d;
		Q_DISABLE_COPY(GensConfig)
	
	public:
		// Controller configuration class.
		// TODO: Make it private?
		CtrlConfig *m_ctrlConfig;
};

}

#endif /* __GENS_QT4_GENSCONFIG_HPP__ */
