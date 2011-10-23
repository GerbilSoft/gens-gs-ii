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

// SysVersion.hpp (contains RegionCode_t)
#include "libgens/MD/SysVersion.hpp"

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
		
		/** System. **/
		// NOTE: Uses LibGens::SysVersion::RegionCode_t, but Q_ENUMS requires a QObject for storage.
		Q_PROPERTY(int regionCode READ regionCode WRITE setRegionCode NOTIFY regionCode_changed);
		GC_PROPERTY(int, regionCode, int, RegionCode);
		
		// Region code auto-detection order.
		Q_PROPERTY(uint16_t regionCodeOrder READ regionCodeOrder WRITE setRegionCodeOrder NOTIFY regionCodeOrder_changed);
		GC_PROPERTY(uint16_t, regionCodeOrder, uint16_t, RegionCodeOrder);
		
		/** General settings. **/
		Q_PROPERTY(bool autoFixChecksum READ autoFixChecksum WRITE setAutoFixChecksum NOTIFY autoFixChecksum_changed)
		GC_PROPERTY(bool, autoFixChecksum, bool, AutoFixChecksum);
		Q_PROPERTY(bool autoPause READ autoPause WRITE setAutoPause NOTIFY autoPause_changed)
		GC_PROPERTY(bool, autoPause, bool, AutoPause);
		Q_PROPERTY(bool borderColor READ borderColor WRITE setBorderColor NOTIFY borderColor_changed)
		GC_PROPERTY(bool, borderColor, bool, BorderColor);
		Q_PROPERTY(bool ntscV30Rolling READ ntscV30Rolling WRITE setNtscV30Rolling NOTIFY ntscV30Rolling_changed)
		GC_PROPERTY(bool, ntscV30Rolling, bool, NtscV30Rolling);
		
		/** Savestates. **/
		Q_PROPERTY(int saveSlot READ saveSlot WRITE setSaveSlot NOTIFY saveSlot_changed)
		GC_PROPERTY(int, saveSlot, int, SaveSlot);
		void setSaveSlot_Prev(void);
		void setSaveSlot_Next(void);
		
		/** Key configuration. **/
		int keyToAction(GensKey_t key);
		GensKey_t actionToKey(int action);
		
		/** Emulation options. (Options menu) **/
		Q_PROPERTY(bool enableSRam READ enableSRam WRITE setEnableSRam NOTIFY enableSRam_changed)
		GC_PROPERTY(bool, enableSRam, bool, EnableSRam)
	
	signals:
		/** System. **/
		void regionCode_changed(int newRegionCode); // LibGens::SysVersion::RegionCode_t
		void regionCodeOrder_changed(uint16_t newRegionCodeOrder);
		
		/** General settings. **/
		void autoFixChecksum_changed(bool newAutoFixChecksum);
		void autoPause_changed(bool newAutoPause);
		void borderColor_changed(bool newBorderColor);
		void ntscV30Rolling_changed(bool newNtscV30Rolling);
		
		/** Savestates. **/
		void saveSlot_changed(int newSaveSlot);
		
		/** Emulation options. (Options menu) **/
		void enableSRam_changed(bool newEnableSRam);
	
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
