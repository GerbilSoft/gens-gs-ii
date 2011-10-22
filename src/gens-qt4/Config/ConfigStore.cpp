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

// Message logging subsystem.
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

namespace GensQt4
{

class ConfigStorePrivate
{
	public:
		ConfigStorePrivate(ConfigStore *q);
		~ConfigStorePrivate();
		
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
	
	private:
		ConfigStore *const q;
		Q_DISABLE_COPY(ConfigStorePrivate)
	
	public:
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
		};
		static const DefaultSetting DefaultSettings[];
		
		// Configuration path.
		QString configPath;
		
		/**
		 * Signal mappings.
		 * Format:
		 * - Key: Property to watch.
		 * - Value: List of SignalMaps.
		 *   - SignalMap.obj: Object to send signal to.
		 *   - SignalMap.slot: Slot name.
		 */
		struct SignalMap
		{
			QPointer<QObject> obj;
			const char *slot;
		};
		QHash<QString, QVector<SignalMap>* > signalMaps;
		QMutex mtxSignalMaps;
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
	{"autoFixChecksum",		"true"},
	{"autoPause",			"false"},
	{"borderColorEmulation",	"true"},
	{"pauseTint",			"true"},
	{"ntscV30Rolling",		"true"},
	
	/** Onscreen display. **/
	{"OSD/fpsEnabled",		"true"},
	{"OSD/fpsColor",		"#ffffff"},
	{"OSD/msgEnabled",		"true"},
	{"OSD/msgColor",		"#ffffff"},
	
	/** Intro effect. **/
	{"Intro_Effect/introStyle",	"0"},	// none
	{"Intro_Effect/introColor",	"7"},	// white
	
	/** System. **/
	{"System/regionCode",		"-1"},		// LibGens::SysVersion::REGION_AUTO
	{"System/regionCodeOrder",	"0x4812"},	// US, Europe, Japan, Asia
	
	/** Sega CD Boot ROMs. **/
	{"Sega_CD/bootRomUSA", NULL},
	{"Sega_CD/bootRomEUR", NULL},
	{"Sega_CD/bootRomJPN", NULL},
	{"Sega_CD/bootRomAsia", NULL},
	
	/** External programs. **/
#ifdef Q_OS_WIN32
#ifdef __amd64__
	{"External_Programs/UnRAR", "UnRAR64.dll"},
#else
	{"External_Programs/UnRAR", "UnRAR.dll"},
#endif
#else /* !Q_OS_WIN32 */
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	{"External_Programs/UnRAR", "/usr/bin/unrar"},
#endif /* Q_OS_WIN32 */
	
	/** Graphics settings. **/
	{"Graphics/aspectRatioConstraint",	"true"},
	{"Graphics/fastBlur",			"false"},
	{"Graphics/bilinearFilter",		"false"},
	{"Graphics/interlacedMode",		"2"},	// GensConfig::INTERLACED_FLICKER
	{"Graphics/contrast",			"0"},
	{"Graphics/brightness",			"0"},
	{"Graphics/grayscale",			"false"},
	{"Graphics/inverted",			"false"},
	{"Graphics/colorScaleMethod",		"1"},	// LibGens::VdpPalette::COLSCALE_FULL
	{"Graphics/stretchMode",		"1"},	// GensConfig::STRETCH_H
	
	/** Savestates. **/
	{"Savestates/saveSlot", "0"},
	
	/** GensWindow configuration. **/
	{"GensWindow/showMenuBar", "true"},
	
	/** Emulation options. (Options menu) **/
	{"Options/enableSRam", "true"},
	
	// TODO: Shortcut keys, controllers, recent ROMs.
	
	/** End of array. **/
	{NULL, NULL}
};

/** ConfigStorePrivate **/


ConfigStorePrivate::ConfigStorePrivate(ConfigStore* q)
	: q(q)
{
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
}


/**
 * Register an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param slot Slot name.
 */
void ConfigStorePrivate::registerChangeNotification(const QString& property, QObject *object, const char *slot)
{
	QMutexLocker mtxLocker(&mtxSignalMaps);
	
	// Get the vector of signal maps for this property.
	QVector<SignalMap>* signalMapVector = signalMaps.value(property, NULL);
	if (!signalMapVector)
	{
		// No vector found. Create one.
		signalMapVector = new QVector<SignalMap>();
		signalMaps.insert(property, signalMapVector);
	}
	
	// Add this object and slot to the signal maps vector.
	SignalMap smap;
	smap.obj = object;
	smap.slot = slot;
	signalMapVector->append(smap);
}


ConfigStorePrivate::~ConfigStorePrivate()
{
	// Save the configuration.
	// TODO: Handle non-default filenames.
	save();
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
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"ConfigStorePrivate: Property '%s' has no default value. FIX THIS!",
			key.toUtf8().constData());
	}
#endif
	
	settings.insert(key, value);
	
	// Invoke slots for registered objects.
	QMutexLocker mtxLocker(&mtxSignalMaps);
	QVector<SignalMap> *signalMapVector = signalMaps.value(key, NULL);
	if (!signalMapVector)
		return;
	
	// Process the signal map list in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	// TODO: Use QLinkedList instead?
	for (int i = (signalMapVector->size() - 1); i >= 0; i--)
	{
		const SignalMap *smap = &signalMapVector->at(i);
		if (smap->obj.isNull())
		{
			// NULL pointer. Remove this Signalmap.
			signalMapVector->remove(i);
		}
		else
		{
			// Invoke this slot.
			QMetaObject::invokeMethod(smap->obj, smap->slot, Q_ARG(QVariant, value));
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
	return settings.value(key);
}


/**
 * Load the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStorePrivate::load(const QString& filename)
{
	QSettings qSettings(filename, QSettings::IniFormat);
	
	// Reset the internal settings map to defaults.
	reset();
	
	// Load the settings from the file.
	foreach (const QString& key, qSettings.allKeys())
	{
		settings.insert(key, qSettings.value(key));
	}
	
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
	
	// Make a copy of the settings map.
	// Entries will be removed as DefaultSettings[] is traversed.
	// This is done to maintain ordering for known settings.
	QHash<QString, QVariant> settingsTmp(settings);
	
	// Save the default settings.
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != NULL; def++)
	{
		const QString key = QLatin1String(def->key);
		qSettings.setValue(key, settingsTmp.value(key,
					(def->value ? QLatin1String(def->value) : QString())));
		settingsTmp.remove(key);
	}
	
	// Save the remaining settings.
	foreach (const QString& key, settingsTmp.keys())
	{
		qSettings.setValue(key, settingsTmp.value(key));
	}
	
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


/** ConfigStore **/


ConfigStore::ConfigStore(QObject *parent)
	: QObject(parent)
	, d(new ConfigStorePrivate(this))
{ }


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
 * @param slot Slot name.
 */
void ConfigStore::registerChangeNotification(const QString& property, QObject *object, const char *slot)
	{ d->registerChangeNotification(property, object, slot); }

}
