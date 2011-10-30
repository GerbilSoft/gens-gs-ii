/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigStore.cpp: Configuration store.                                   *
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

#include "ConfigStore.hpp"

// C includes.
#include <stdint.h>

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/log_msg.h"

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QHash>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaMethod>

// QtGui includes.
#include <QtGui/QColor>

// Key configuration.
#include "actions/GensKeyConfig.hpp"

namespace GensQt4
{

class ConfigStorePrivate
{
	public:
		ConfigStorePrivate(ConfigStore *q);
		~ConfigStorePrivate();
	
	private:
		ConfigStore *const q;
		Q_DISABLE_COPY(ConfigStorePrivate)
	
	public:
		/**
		 * Initialize the Default Settings QHash.
		 */
		static void InitDefaultSettingsHash(void);
		
		/**
		 * Reset all settings to defaults.
		 */
		void reset(void);
		
		/**
		 * Check if a region code order is valid.
		 * @param regionCodeOrder Region code order to check.
		 * @return True if valid; false if invalid.
		 */
		static bool IsRegionCodeOrderValid(uint16_t regionCodeOrder);
		
		/**
		 * Validate a property.
		 * @param key Property name.
		 * @param value Property value. (May be edited for validation.)
		 * @return Property value (possibly adjusted) if validated; invalid QVariant on failure.
		 */
		static QVariant Validate(const QString& name, const QVariant& value);
		
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
		 * Get a property.
		 * Converts hexadecimal string values to signed int.
		 * @param key Property name.
		 * @return Property value.
		 */
		int getInt(const QString& key);
		
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
		 * @param filename Configuration filename.
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
		 * @param slot Slot name.
		 */
		void registerChangeNotification(const QString& property, QObject *object, const char *slot);
		
		/**
		 * Unregister an object for property change notification.
		 * @param property Property to watch.
		 * @param object QObject to register.
		 * @param slot Slot name.
		 */
		void unregisterChangeNotification(const QString& property, QObject *object, const char *slot);
		
		/**
		 * Notify all registered objects that configuration settings have changed.
		 * Useful when starting the emulator.
		 */
		void notifyAll(void);
		
		/**
		 * Look up the method index of a SIGNAL() or SLOT() in a QObject.
		 * @param object Qt object.
		 * @param method Method name.
		 * @return Method index, or negative on error.
		 */
		static int LookupQtMethod(const QObject *object, const char *method);
		
		/**
		 * Invoke a Qt method by method index, with one QVariant parameter.
		 * @param object Qt object.
		 * @param method_idx Method index.
		 * @param param QVariant parameter.
		 */
		static void InvokeQtMethod(QObject *object, int method_idx, QVariant param);
	
		/** Internal variables. **/
		
		// Current settings.
		// TODO: Use const char* for the key instead of QString?
		QHash<QString, QVariant> settings;
		
		// Default configuration filename.
		static const char DefaultConfigFilename[];
		
		// Default settings.
		struct DefaultSetting
		{
			const char *key;
			const char *value;
			int hex_digits;		// If non-zero, saves as hexadecimal with this many digits.
			
			/**
			 * If true, allow a setting to be reset to the current value,
			 * which will result in property change signals being emitted
			 * regardless of whether or not the setting has actually changed.
			 *
			 * This is useful for e.g. "Savestate/saveSlot", since the user
			 * should be able to press the key corresponding to the current
			 * save slot in order to see the preview image for that savestate.
			 */
			bool allow_same_value;
			
			// Parameter validation.
			enum ValidationType
			{
				VT_NONE,		// No validation.
				VT_BOOL,		// Boolean; normalize to true/false.
				VT_COLOR,		// QColor.
				VT_RANGE,		// Integer range.
				VT_REGIONCODEORDER,	// RegionCodeOrder.
				
				VT_MAX
			};
			ValidationType validation;
			int range_min;
			int range_max;
		};
		static const DefaultSetting DefaultSettings[];
		static QHash<QString, const DefaultSetting*> DefaultSettingsHash;
		static QMutex MtxDefaultSettingsHash;
		
		// Configuration path.
		QString configPath;
		
		/**
		 * Signal mappings.
		 * Format:
		 * - Key: Property to watch.
		 * - Value: List of SignalMaps.
		 *   - SignalMap.object: Object to send signal to.
		 *   - SignalMap.method: Method name.
		 */
		struct SignalMap
		{
			QPointer<QObject> object;
			int method_idx;
		};
		QHash<QString, QVector<SignalMap>* > signalMaps;
		QMutex mtxSignalMaps;
		
		/** Recent ROMs. **/
		RecentRoms *const recentRoms;
		
		/** Key configuration. **/
		GensKeyConfig keyConfig;
};


/**
 * Default configuration filename.
 */
const char ConfigStorePrivate::DefaultConfigFilename[] = "gens-gs-ii.NEWCONF.conf";

/**
 * Default settings.
 */
const ConfigStorePrivate::DefaultSetting ConfigStorePrivate::DefaultSettings[] =
{
	/** General settings. **/
	{"autoFixChecksum",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"autoPause",			"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"pauseTint",			"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	
	/** Onscreen display. **/
	{"OSD/fpsEnabled",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"OSD/fpsColor",		"#ffffff", 0, false,	DefaultSetting::VT_COLOR, 0, 0},
	{"OSD/msgEnabled",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"OSD/msgColor",		"#ffffff", 0, false,	DefaultSetting::VT_COLOR, 0, 0},
	
	/** Intro effect. **/
	// TODO: Use enum constants for range.
	{"Intro_Effect/introStyle",	"0", 0, false,	DefaultSetting::VT_RANGE, 0, 2},	// none
	{"Intro_Effect/introColor",	"7", 0, false,	DefaultSetting::VT_RANGE, 0, 7},	// white
	
	/** System. **/
	// TODO: Use enum constants for range.
	{"System/regionCode",		"-1", 0, false,		DefaultSetting::VT_RANGE, -1, 4},	// LibGens::SysVersion::REGION_AUTO
	{"System/regionCodeOrder",	"0x4812", 4, false,	DefaultSetting::VT_REGIONCODEORDER, 0, 0},	// US, Europe, Japan, Asia
	
	/** Sega CD Boot ROMs. **/
	{"Sega_CD/bootRomUSA", 		"", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomEUR",		"", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomJPN",		"", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomAsia",		"", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	
	/** External programs. **/
#ifdef Q_OS_WIN32
#ifdef __amd64__
	{"External_Programs/UnRAR", "UnRAR64.dll", 0, false,	DefaultSetting::VT_NONE, 0, 0},
#else
	{"External_Programs/UnRAR", "UnRAR.dll", 0, false,	DefaultSetting::VT_NONE, 0, 0},
#endif
#else /* !Q_OS_WIN32 */
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	{"External_Programs/UnRAR", "/usr/bin/unrar", 0, false, DefaultSetting::VT_NONE, 0, 0},
#endif /* Q_OS_WIN32 */
	
	/** Graphics settings. **/
	// TODO: Use enum constants for range.
	{"Graphics/aspectRatioConstraint",	"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/fastBlur",			"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/bilinearFilter",		"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/interlacedMode",		"2", 0, false,		DefaultSetting::VT_RANGE, 0, 2},	// GensConfig::INTERLACED_FLICKER
	{"Graphics/contrast",			"0", 0, false,		DefaultSetting::VT_RANGE, -100, 100},
	{"Graphics/brightness",			"0", 0, false,		DefaultSetting::VT_RANGE, -100, 100},
	{"Graphics/grayscale",			"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/inverted",			"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/colorScaleMethod",		"1", 0, false,		DefaultSetting::VT_RANGE, 0, 2},	// LibGens::VdpPalette::COLSCALE_FULL
	{"Graphics/stretchMode",		"1", 0, false,		DefaultSetting::VT_RANGE, 0, 3},	// GensConfig::STRETCH_H
	
	/** VDP settings. **/
	{"VDP/borderColorEmulation",	"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/ntscV30Rolling",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/zeroLengthDMA",		"false", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/spriteLimits",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/vscrollBug",		"true", 0, false,	DefaultSetting::VT_BOOL, 0, 0},
	
	/** Savestates. **/
	{"Savestates/saveSlot", "0", 0, true, DefaultSetting::VT_RANGE, 0, 9},
	
	/** GensWindow configuration. **/
	{"GensWindow/showMenuBar", "true", 0, false, DefaultSetting::VT_BOOL, 0, 0},
	
	/** Emulation options. (Options menu) **/
	{"Options/enableSRam", "true", 0, false, DefaultSetting::VT_BOOL, 0, 0},
	
	/** Directories. **/
	// TODO: Add a class to handle path resolution.
	// TODO: Validation type? (Or will that be handled using the class...)
	{"Directories/Savestates",	"./Savestates/", 0, false,	DefaultSetting::VT_NONE, 0, 0},
	{"Directories/SRAM",		"./SRAM/", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Directories/BRAM",		"./BRAM/", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Directories/WAV",		"./WAV/", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Directories/VGM",		"./VGM/", 0, false,		DefaultSetting::VT_NONE, 0, 0},
	{"Directories/Screenshots",	"./Screenshots/", 0, false,	DefaultSetting::VT_NONE, 0, 0},
	
	// TODO: Shortcut keys, controllers, recent ROMs.
	
	/** End of array. **/
	{NULL, NULL, 0, false, DefaultSetting::VT_NONE, 0, 0}
};

QHash<QString, const ConfigStorePrivate::DefaultSetting*> ConfigStorePrivate::DefaultSettingsHash;
QMutex ConfigStorePrivate::MtxDefaultSettingsHash;


/** ConfigStorePrivate **/


ConfigStorePrivate::ConfigStorePrivate(ConfigStore* q)
	: q(q)
	, recentRoms(new RecentRoms())
{
	// Initialize the Default Settings QHash.
	InitDefaultSettingsHash();
	
	// Initialize settings.
	reset();
	
	// Determine the configuration path.
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String("gens-gs-ii"),
				QLatin1String("gens-gs-ii"));
	
	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	configPath = settings.fileName();
	int sepChr = configPath.lastIndexOf(QChar(L'/'));
	if (sepChr >= 0)
		configPath.remove(sepChr + 1, configPath.size());
	
	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir dir(configPath);
	if (!dir.exists())
		dir.mkpath(configPath);
	
	// Load the user's settings.
	load();
}


/**
 * Initialize the Default Settings QHash.
 */
void ConfigStorePrivate::InitDefaultSettingsHash(void)
{
	QMutexLocker mtxLocker(&MtxDefaultSettingsHash);
	
	if (!DefaultSettingsHash.isEmpty())
		return;
	
	// Populate the default settings hash.
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != NULL; def++)
	{
		const QString key = QLatin1String(def->key);
		DefaultSettingsHash.insert(key, def);
	}
}


/**
 * Register an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStorePrivate::registerChangeNotification(const QString& property, QObject *object, const char *method)
{
	if (!object)
		return;
	
	// Get the vector of signal maps for this property.
	QMutexLocker mtxLocker(&mtxSignalMaps);
	QVector<SignalMap>* signalMapVector = signalMaps.value(property, NULL);
	if (!signalMapVector)
	{
		// No vector found. Create one.
		signalMapVector = new QVector<SignalMap>();
		signalMaps.insert(property, signalMapVector);
	}
	
	// Look up the method index.
	int method_idx = LookupQtMethod(object, method);
	if (method_idx < 0)
		return;
	
	// Add this object and slot to the signal maps vector.
	SignalMap smap;
	smap.object = object;
	smap.method_idx = method_idx;
	signalMapVector->append(smap);
}


/**
 * Unregister an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name. (If NULL, unregisters all slots for this object.)
 */
void ConfigStorePrivate::unregisterChangeNotification(const QString& property, QObject *object, const char *method)
{
	if (!object)
		return;
	
	// Get the vector of signal maps for this property.
	QMutexLocker mtxLocker(&mtxSignalMaps);
	QVector<SignalMap>* signalMapVector = signalMaps.value(property, NULL);
	if (!signalMapVector)
		return;
	
	// Get the method index.
	int method_idx = -1;
	if (method != NULL)
	{
		method_idx = LookupQtMethod(object, method);
		if (method_idx < 0)
			return;
	}
	
	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	// TODO: Use QLinkedList instead?
	for (int i = (signalMapVector->size() - 1); i >= 0; i--)
	{
		const SignalMap *smap = &signalMapVector->at(i);
		if (smap->object.isNull())
			signalMapVector->remove(i);
		else if (smap->object == object)
		{
			// Found the object.
			if (method == NULL || method_idx == smap->method_idx)
			{
				// Found a matching signal map.
				signalMapVector->remove(i);
			}
		}
	}
}


ConfigStorePrivate::~ConfigStorePrivate()
{
	// Save the configuration.
	// TODO: Handle non-default filenames.
	save();
	
	// Delete all the signal map vectors.
	qDeleteAll(signalMaps);
	signalMaps.clear();
}


/**
 * Reset all settings to defaults.
 */
void ConfigStorePrivate::reset(void)
{
	// Initialize settings with DefaultSettings.
	settings.clear();
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != NULL; def++)
	{
		settings.insert(QLatin1String(def->key),
				(def->value ? QLatin1String(def->value) : QString()));
	}
}


/**
 * Check if a region code order is valid.
 * @param regionCodeOrder Region code order to check.
 * @return True if valid; false if invalid.
 */
bool ConfigStorePrivate::IsRegionCodeOrderValid(uint16_t regionCodeOrder)
{
	static const uint16_t RegionCodeOrder_tbl[24] =
	{
		0x4812, 0x4821, 0x4182, 0x4128, 0x4281, 0x4218, 
		0x8412, 0x8421, 0x8124, 0x8142, 0x8241, 0x8214,
		0x1482, 0x1428, 0x1824, 0x1842, 0x1248, 0x1284,
		0x2481, 0x2418,	0x2814, 0x2841, 0x2148, 0x2184
	};
	
	for (size_t i = 0; i < (sizeof(RegionCodeOrder_tbl)/sizeof(RegionCodeOrder_tbl[0])); i++)
	{
		if (regionCodeOrder == RegionCodeOrder_tbl[i])
			return true;
	}
	
	// Region code order is not valid.
	return false;
}


/**
 * Validate a property.
 * @param key Property name.
 * @param value Property value. (May be edited for validation.)
 * @return Property value (possibly adjusted) if validated; invalid QVariant on failure.
 */
QVariant ConfigStorePrivate::Validate(const QString& name, const QVariant& value)
{
	// Get the DefaultSetting entry for this property.
	// TODO: Lock the hash?
	const DefaultSetting *def = DefaultSettingsHash.value(name, NULL);
	if (!def)
		return -1;
	
	switch (def->validation)
	{
		case DefaultSetting::VT_NONE:
		default:
			// No validation required.
			return value;
		
		case DefaultSetting::VT_BOOL:
			if (!value.canConvert(QVariant::Bool))
				return QVariant();
			return QVariant(value.toBool());
		
		case DefaultSetting::VT_COLOR:
		{
			QColor color = value.value<QColor>();
			if (!color.isValid())
				return QVariant();
			return QVariant(color.name());
		}
		
		case DefaultSetting::VT_RANGE:
		{
			if (!value.canConvert(QVariant::Int))
				return QVariant();
			int val = value.toString().toInt(NULL, 0);
			if (val < def->range_min || val > def->range_max)
				return QVariant();
			return QVariant(val);
		}
		
		case DefaultSetting::VT_REGIONCODEORDER:
		{
			if (!value.canConvert(QVariant::UInt))
				return QVariant();
			uint16_t rc_order = (uint16_t)value.toString().toUInt(NULL, 0);
			if (!IsRegionCodeOrderValid(rc_order))
				return QVariant();
			return QVariant(rc_order);
		}
	}
	
	// Should not get here...
	return QVariant();
}


/**
 * Set a property.
 * @param key Property name.
 * @param value Property value.
 */
void ConfigStorePrivate::set(const QString& key, const QVariant& value)
{
#ifndef NDEBUG
	// Make sure this property exists.
	if (!settings.contains(key))
	{
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"ConfigStorePrivate: Property '%s' has no default value. FIX THIS!",
			key.toUtf8().constData());
	}
#endif
	
	// Get the default value.
	const DefaultSetting *def = DefaultSettingsHash.value(key, NULL);
	if (!def)
		return;
	
	if (!def->allow_same_value)
	{
		// Check if the new value is the same as the old value.
		QVariant oldValue = settings.value(key);
		if (value == oldValue)
			return;
	}
	
	// Verify that this value passes validation.
	QVariant newValue = Validate(key, value);
	if (!newValue.isValid())
		return;
	
	// Set the new value.
	settings.insert(key, newValue);
	
	// Invoke methods for registered objects.
	QMutexLocker mtxLocker(&mtxSignalMaps);
	QVector<SignalMap> *signalMapVector = signalMaps.value(key, NULL);
	if (!signalMapVector)
		return;
	
	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	// TODO: Use QLinkedList instead?
	for (int i = (signalMapVector->size() - 1); i >= 0; i--)
	{
		const SignalMap *smap = &signalMapVector->at(i);
		if (smap->object.isNull())
			signalMapVector->remove(i);
		else
		{
			// Invoke this method.
			InvokeQtMethod(smap->object, smap->method_idx, newValue);
		}
	}
}


/**
 * Get a property.
 * @param key Property name.
 * @return Property value.
 */
QVariant ConfigStorePrivate::get(const QString& key)
{
#ifndef NDEBUG
	// Make sure this property exists.
	if (!settings.contains(key))
	{
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"ConfigStorePrivate: Property '%s' has no default value. FIX THIS!",
			key.toUtf8().constData());
	}
#endif

	return settings.value(key);
}

/**
 * Get a property.
 * Converts hexadecimal string values to unsigned int.
 * @param key Property name.
 * @return Property value.
 */
unsigned int ConfigStorePrivate::getUInt(const QString& key)
	{ return get(key).toString().toUInt(NULL, 0); }

/**
 * Get a property.
 * Converts hexadecimal string values to signed int.
 * @param key Property name.
 * @return Property value.
 */
int ConfigStorePrivate::getInt(const QString& key)
	{ return get(key).toString().toInt(NULL, 0); }


/**
 * Load the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStorePrivate::load(const QString& filename)
{
	QSettings qSettings(filename, QSettings::IniFormat);
	
	// NOTE: Only known settings will be loaded.
	settings.clear();
	settings.reserve(sizeof(DefaultSettings)/sizeof(DefaultSettings[0]));
	
	// Load known settings from the configuration file.
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != NULL; def++)
	{
		const QString key = QLatin1String(def->key);
		QVariant value = qSettings.value(key, QLatin1String(def->value));
		
		// Validate this value.
		value = Validate(key, value);
		if (!value.isValid())
		{
			// Validation failed. Use the default value.
			value = QVariant(QLatin1String(def->value));
		}
		
		settings.insert(key, value);
	}
	
	// Load the Recent ROMs settings.
	// TODO: Remove Recent ROMs entries from qSettings?
	qSettings.beginGroup(QLatin1String("Recent_ROMs"));
	recentRoms->load(&qSettings);
	qSettings.endGroup();
	
	// Load the key configuration.
	// TODO: Remove key configuration entries from qSettings?
	qSettings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.load(&qSettings);
	qSettings.endGroup();
	
	// Finished loading settings.
	// NOTE: Caller must call emitAll() for settings to take effect.
	return 0;
}

/**
 * Load the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStorePrivate::load(void)
{
	const QString cfgFilename = configPath + QLatin1String(DefaultConfigFilename);
	return load(cfgFilename);
}


/**
 * Save the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStorePrivate::save(const QString& filename)
{
	QSettings qSettings(filename, QSettings::IniFormat);
	
	/** Application information. **/
	
	// Stored in the "General" section.
	// TODO: Move "General" settings to another section?
	// ("General" is always moved to the top of the file.)
	// TODO: Get the application information from somewhere else.
	// TODO: Use MDP version macros.
	const QString sVersion = QString::fromLatin1("%1.%2.%3")
					.arg((LibGens::version >> 24) & 0xFF)
					.arg((LibGens::version >> 16) & 0xFF)
					.arg(LibGens::version & 0xFFFF);
	
	qSettings.setValue(QLatin1String("_Application"), QLatin1String("Gens/GS II"));
	qSettings.setValue(QLatin1String("_Version"), sVersion);
	
	if (LibGens::version_desc)
	{
		qSettings.setValue(QLatin1String("_VersionExt"),
					QString::fromUtf8(LibGens::version_desc));
	}
	else
	{
		qSettings.remove(QLatin1String("_VersionExt"));
	}
	
	if (LibGens::version_vcs)
	{
		qSettings.setValue(QLatin1String("_VersionVcs"),
					QString::fromUtf8(LibGens::version_vcs));
	}
	else
	{
		qSettings.remove(QLatin1String("_VersionVcs"));
	}
	
	// NOTE: Only known settings will be saved.
	
	// Save known settings to the configuration file.
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != NULL; def++)
	{
		const QString key = QLatin1String(def->key);
		QVariant value = settings.value(key, QLatin1String(def->value));
		if (def->hex_digits > 0)
		{
			// Convert to hexadecimal.
			unsigned int uint_val = value.toString().toUInt(NULL, 0);
			value = QLatin1String("0x") + 
					QString::number(uint_val, 16).toUpper().rightJustified(4, QChar(L'0'));
		}
		
		qSettings.setValue(key, value);
	}
	
	// Save the Recent ROMs settings.
	// TODO: Remove Recent ROMs entries from qSettings?
	qSettings.beginGroup(QLatin1String("Recent_ROMs"));
	recentRoms->save(&qSettings);
	qSettings.endGroup();
	
	// Save the key configuration.
	// TODO: Remove key configuration entries from qSettings?
	qSettings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.save(&qSettings);
	qSettings.endGroup();
	
	return 0;
}

/**
 * Save the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStorePrivate::save(void)
{
	const QString cfgFilename = configPath + QLatin1String(DefaultConfigFilename);
	return save(cfgFilename);
}


/**
 * Notify all registered objects that configuration settings have changed.
 * Useful when starting the emulator.
 */
void ConfigStorePrivate::notifyAll(void)
{
	// Invoke methods for registered objects.
	QMutexLocker mtxLocker(&mtxSignalMaps);
	
	foreach (const QString& property, signalMaps.keys())
	{
		QVector<SignalMap> *signalMapVector = signalMaps.value(property);
		if (signalMapVector->isEmpty())
			continue;
		
		// Get the property value.
		const QVariant value = settings.value(property);
		
		// Process the signal map vector in reverse-order.
		// Reverse order makes it easier to remove deleted objects.
		// TODO: Use QLinkedList instead?
		for (int i = (signalMapVector->size() - 1); i >= 0; i--)
		{
			const SignalMap *smap = &signalMapVector->at(i);
			if (smap->object.isNull())
				signalMapVector->remove(i);
			else
			{
				// Invoke this method.
				InvokeQtMethod(smap->object, smap->method_idx, value);
			}
		}
	}
}


/**
 * Look up the method index of a SIGNAL() or SLOT() in a QObject.
 * @param object Qt object.
 * @param method Method name.
 * @return Method index, or negative on error.
 */
int ConfigStorePrivate::LookupQtMethod(const QObject *object, const char *method)
{
	// Based on QMetaObject::invokeMethod().
	
	// NOTE: The first character of method indicates whether it's a signal or slot.
	// We don't need this information, so we use method+1.
	method++;
	
	int idx = object->metaObject()->indexOfMethod(method);
	if (idx < 0)
	{
		QByteArray norm = QMetaObject::normalizedSignature(method);
		idx = object->metaObject()->indexOfMethod(norm.constData());
	}
	
	if (idx < 0 || idx >= object->metaObject()->methodCount())
	{
		// TODO: Do verification in registerChangeNotification()?
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"No such method %s::%s",
			object->metaObject()->className(), method);
		return -1;
	}
	
	// Method index found.
	return idx;
}

/**
 * Invoke a Qt method by method index, with one QVariant parameter.
 * @param object Qt object.
 * @param method_idx Method index.
 * @param param QVariant parameter.
 */
void ConfigStorePrivate::InvokeQtMethod(QObject *object, int method_idx, QVariant param)
{
	// Based on QMetaObject::invokeMethod().
	QMetaMethod metaMethod = object->metaObject()->method(method_idx);
	metaMethod.invoke(object, Qt::AutoConnection,
		      QGenericReturnArgument(), Q_ARG(QVariant, param));
}


/** ConfigStore **/


ConfigStore::ConfigStore(QObject *parent)
	: QObject(parent)
	, d(new ConfigStorePrivate(this))
{ }

ConfigStore::~ConfigStore()
	{ delete d; }


/**
 * Reset all settings to defaults.
 */
void ConfigStore::reset(void)
	{ d->reset(); }


/**
 * Set a property.
 * @param key Property name.
 * @param value Property value.
 */
void ConfigStore::set(const QString& key, const QVariant& value)
	{ d->set(key, value); }


/**
 * Get a property.
 * @param key Property name.
 * @return Property value.
 */
QVariant ConfigStore::get(const QString& key)
	{ return d->get(key); }

/**
 * Get a property.
 * Converts hexadecimal string values to unsigned int.
 * @param key Property name.
 * @return Property value.
 */
unsigned int ConfigStore::getUInt(const QString& key)
	{ return d->getUInt(key); }

/**
 * Get a property.
 * Converts hexadecimal string values to signed int.
 * @param key Property name.
 * @return Property value.
 */
int ConfigStore::getInt(const QString& key)
	{ return d->getInt(key); }


/**
 * Load the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::load(const QString& filename)
	{ return d->load(filename); }

/**
 * Load the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::load(void)
	{ return d->load(); }


/**
 * Save the configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(const QString& filename)
	{ return d->save(filename); }

/**
 * Save the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(void)
	{ return d->save(); }


/**
 * Register an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::registerChangeNotification(const QString& property, QObject *object, const char *method)
	{ d->registerChangeNotification(property, object, method); }

/**
 * Unregister an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::unregisterChangeNotification(const QString& property, QObject *object, const char *method)
	{ d->unregisterChangeNotification(property, object, method); }

/**
 * Notify all registered objects that configuration settings have changed.
 * Useful when starting the emulator.
 */
void ConfigStore::notifyAll(void)
	{ d->notifyAll(); }

/**
 * Get the configuration path.
 * @return Configuration path.
 */
QString ConfigStore::configPath(void)
	{ return d->configPath; }


/** Recent ROMs. **/

/**
 * Update the Recent ROMs list.
 * @param filename ROM filename.
 * @param z_filename Filename of ROM within archive.
 * @param sysId System ID.
 */
void ConfigStore::recentRomsUpdate(QString filename, QString z_filename,
					LibGens::Rom::MDP_SYSTEM_ID sysId)
{
	d->recentRoms->update(filename, z_filename, sysId);
}

/**
 * Get a const pointer to the Recent ROMs object.
 * @return Const pointer to the Recent ROMs object.
 */
const RecentRoms *ConfigStore::recentRomsObject(void)
	{ return d->recentRoms; }

/**
 * Get a Recent ROMs entry.
 * @param id Recent ROM ID.
 */
RecentRom_t ConfigStore::recentRomsEntry(int id)
	{ return d->recentRoms->getRom(id); }


/** Key configuration. **/

/**
 * Get the action associated with a GensKey_t.
 * @param key GensKey_t.
 * @return Action ID.
 */
int ConfigStore::keyToAction(GensKey_t key)
	{ return d->keyConfig.keyToAction(key); }

/**
 * Get the GensKey_t associated with an action.
 * @param actoin Action ID.
 * @return GensKey_t.
 */
GensKey_t ConfigStore::actionToKey(int action)
	{ return d->keyConfig.actionToKey(action); }

}
