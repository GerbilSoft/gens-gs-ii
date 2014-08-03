/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigStore.cpp: Configuration store.                                   *
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

#include "ConfigStore.hpp"

// C includes.
#include <stdint.h>

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/common.h"
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

// Default settings.
#include "ConfigDefaults.hpp"

// Key configuration.
#include "actions/GensKeyConfig.hpp"

// Controller Configuration.
// (now only has static load/save functions)
#include "CtrlConfig.hpp"

// gqt4_app
#include "gqt4_main.hpp"
#include "GensQApplication.hpp"

namespace GensQt4
{

class ConfigStorePrivate
{
	public:
		ConfigStorePrivate(ConfigStore *q);
		~ConfigStorePrivate();

	private:
		ConfigStore *const q_ptr;
		Q_DECLARE_PUBLIC(ConfigStore);
	private:
		Q_DISABLE_COPY(ConfigStorePrivate)

	public:
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
		static QVariant Validate(const QString &name, const QVariant &value);

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
		static void InvokeQtMethod(QObject *object, int method_idx, const QVariant &param);

		/** Internal variables. **/

		// Current settings.
		// TODO: Use const char* for the key instead of QString?
		QHash<QString, QVariant> settings;

		/**
		 * Signal mappings.
		 * Format:
		 * - Key: Property to watch.
		 * - Value: List of SignalMaps.
		 *   - SignalMap.object: Object to send signal to.
		 *   - SignalMap.method: Method name.
		 */
		struct SignalMap {
			QPointer<QObject> object;
			int method_idx;
		};
		QHash<QString, QVector<SignalMap>* > signalMaps;
		QMutex mtxSignalMaps;

		/** PathConfig **/
		PathConfig *const pathConfig;

		/** Recent ROMs. **/
		RecentRoms *const recentRoms;

		/** Key configuration. **/
		GensKeyConfig keyConfig;
};

/** ConfigStorePrivate **/

ConfigStorePrivate::ConfigStorePrivate(ConfigStore* q)
	: q_ptr(q)
	, pathConfig(new PathConfig(q))
	, recentRoms(new RecentRoms(q))
{
	// TODO: This shouldn't be publicly accessible...
	q->m_keyManager = new LibGensKeys::KeyManager();
}

ConfigStorePrivate::~ConfigStorePrivate()
{
	// Delete all the signal map vectors.
	qDeleteAll(signalMaps);
	signalMaps.clear();

	Q_Q(ConfigStore);
	delete q->m_keyManager;
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

	for (size_t i = 0; i < ARRAY_SIZE(RegionCodeOrder_tbl); i++) {
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
QVariant ConfigStorePrivate::Validate(const QString &name, const QVariant &value)
{
	// Get the DefaultSetting entry for this property.
	// TODO: Lock the hash?
	const ConfigDefaults::DefaultSetting *def = ConfigDefaults::Instance()->get(name);
	if (!def)
		return -1;

	switch (def->validation) {
		case ConfigDefaults::DefaultSetting::VT_NONE:
		default:
			// No validation required.
			return value;

		case ConfigDefaults::DefaultSetting::VT_BOOL:
			if (!value.canConvert(QVariant::Bool))
				return QVariant();
			return QVariant(value.toBool());

		case ConfigDefaults::DefaultSetting::VT_COLOR: {
			QColor color = value.value<QColor>();
			if (!color.isValid())
				return QVariant();
			return QVariant(color.name());
		}

		case ConfigDefaults::DefaultSetting::VT_RANGE: {
			if (!value.canConvert(QVariant::Int))
				return QVariant();
			int val = value.toString().toInt(nullptr, 0);
			if (val < def->range_min || val > def->range_max)
				return QVariant();
			return QVariant(val);
		}

		case ConfigDefaults::DefaultSetting::VT_REGIONCODEORDER: {
			if (!value.canConvert(QVariant::UInt))
				return QVariant();
			uint16_t rc_order = (uint16_t)value.toString().toUInt(nullptr, 0);
			if (!IsRegionCodeOrderValid(rc_order))
				return QVariant();
			return QVariant(rc_order);
		}
	}

	// Should not get here...
	return QVariant();
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
	// We don't need this information, so we'll use method+1.
	method++;

	int idx = object->metaObject()->indexOfMethod(method);
	if (idx < 0) {
		// Normalize the signature and try again.
		QByteArray norm = QMetaObject::normalizedSignature(method);
		idx = object->metaObject()->indexOfMethod(norm.constData());
	}

	if (idx < 0 || idx >= object->metaObject()->methodCount()) {
		// Method index not found.
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
void ConfigStorePrivate::InvokeQtMethod(QObject *object, int method_idx, const QVariant &param)
{
	// Based on QMetaObject::invokeMethod().
	QMetaMethod metaMethod = object->metaObject()->method(method_idx);
	metaMethod.invoke(object, Qt::AutoConnection,
		      QGenericReturnArgument(), Q_ARG(QVariant, param));
}

/** ConfigStore **/

ConfigStore::ConfigStore(QObject *parent)
	: QObject(parent)
	, d_ptr(new ConfigStorePrivate(this))
{
	// Initialize defaults and load user settings.
	reset();
	load();
}

ConfigStore::~ConfigStore()
{
	// Save the configuration.
	// TODO: Handle non-default filenames.
	save();

	delete d_ptr;
}

/**
 * Reset all settings to defaults.
 */
void ConfigStore::reset(void)
{
	// Initialize settings with DefaultSettings.
	Q_D(ConfigStore);
	d->settings.clear();
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		d->settings.insert(QLatin1String(def->key),
			(def->value ? QLatin1String(def->value) : QString()));
	}
}

/**
 * Set a property.
 * @param key Property name.
 * @param value Property value.
 */
void ConfigStore::set(const QString &key, const QVariant &value)
{
	Q_D(ConfigStore);

#ifndef NDEBUG
	// Make sure this property exists.
	if (!d->settings.contains(key)) {
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		fprintf(stderr, "ConfigStore: Property '%s' has no default value. FIX THIS!\n",
			key.toUtf8().constData());
	}
#endif

	// Get the default value.
	const ConfigDefaults::DefaultSetting *def = ConfigDefaults::Instance()->get(key);
	if (!def)
		return;

	if (!(def->flags & ConfigDefaults::DefaultSetting::DEF_ALLOW_SAME_VALUE)) {
		// Check if the new value is the same as the old value.
		QVariant oldValue = d->settings.value(key);
		if (value == oldValue)
			return;
	}

	// Verify that this value passes validation.
	QVariant newValue = ConfigStorePrivate::Validate(key, value);
	if (!newValue.isValid())
		return;

	// Set the new value.
	d->settings.insert(key, newValue);

	// Invoke methods for registered objects.
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap> *signalMapVector = d->signalMaps.value(key, nullptr);
	if (!signalMapVector)
		return;

	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	for (int i = (signalMapVector->size() - 1); i >= 0; i--) {
		const ConfigStorePrivate::SignalMap &smap = signalMapVector->at(i);
		if (smap.object.isNull()) {
			signalMapVector->remove(i);
		} else {
			// Invoke this method.
			ConfigStorePrivate::InvokeQtMethod(smap.object, smap.method_idx, newValue);
		}
	}
}

/**
 * Get a property.
 * @param key Property name.
 * @return Property value.
 */
QVariant ConfigStore::get(const QString &key) const
{
	Q_D(const ConfigStore);

#ifndef NDEBUG
	// Make sure this property exists.
	if (!d->settings.contains(key)) {
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		fprintf(stderr, "ConfigStore: Property '%s' has no default value. FIX THIS!\n",
			key.toUtf8().constData());
	}
#endif

	return d->settings.value(key);
}

/**
 * Get a property.
 * Converts hexadecimal string values to unsigned int.
 * @param key Property name.
 * @return Property value.
 */
unsigned int ConfigStore::getUInt(const QString &key) const
{
	return get(key).toString().toUInt(nullptr, 0);
}

/**
 * Get a property.
 * Converts hexadecimal string values to signed int.
 * @param key Property name.
 * @return Property value.
 */
int ConfigStore::getInt(const QString &key) const
{
	return get(key).toString().toInt(nullptr, 0);
}

/**
 * Load the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::load(const QString &filename)
{
	Q_D(ConfigStore);
	QSettings qSettings(filename, QSettings::IniFormat);

	// NOTE: Only known settings will be loaded.
	d->settings.clear();
	// TODO: Add function to get sizeof(DefaultSettings) from ConfigDefaults.
	d->settings.reserve(32);

	// Load known settings from the configuration file.
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		const QString key = QLatin1String(def->key);
		QVariant value = qSettings.value(key, QLatin1String(def->value));

		// Validate this value.
		value = ConfigStorePrivate::Validate(key, value);
		if (!value.isValid()) {
			// Validation failed. Use the default value.
			value = QVariant(QLatin1String(def->value));
		}

		d->settings.insert(key, value);
	}

	// Load the PathConfig settings.
	qSettings.beginGroup(QLatin1String("Directories"));
	d->pathConfig->load(&qSettings);
	qSettings.endGroup();

	// Load the Recent ROMs settings.
	qSettings.beginGroup(QLatin1String("Recent_ROMs"));
	d->recentRoms->load(&qSettings);
	qSettings.endGroup();

	// Load the key configuration.
	qSettings.beginGroup(QLatin1String("Shortcut_Keys"));
	d->keyConfig.load(&qSettings);
	qSettings.endGroup();

	// Load the controller configuration.
	// TODO: Rework this with the upcoming all-in-one IoManager.
	qSettings.beginGroup(QLatin1String("Controllers"));
	CtrlConfig::load(qSettings, m_keyManager);
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
int ConfigStore::load(void)
{
	Q_D(const ConfigStore);
	const QString cfgFilename = (d->pathConfig->configPath() +
		QLatin1String(ConfigDefaults::DefaultConfigFilename));
	return load(cfgFilename);
}

/**
 * Save the configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(const QString &filename) const
{
	QSettings qSettings(filename, QSettings::IniFormat);

	/** Application information. **/

	// Stored in the "General" section.
	// TODO: Move "General" settings to another section?
	// ("General" is always moved to the top of the file.)
	qSettings.setValue(QLatin1String("_Application"), gqt4_app->applicationName());
	qSettings.setValue(QLatin1String("_Version"), gqt4_app->applicationVersion());

	if (LibGens::version_desc) {
		qSettings.setValue(QLatin1String("_VersionExt"),
					QString::fromUtf8(LibGens::version_desc));
	} else {
		qSettings.remove(QLatin1String("_VersionExt"));
	}

	if (LibGens::version_vcs) {
		qSettings.setValue(QLatin1String("_VersionVcs"),
					QString::fromUtf8(LibGens::version_vcs));
	} else {
		qSettings.remove(QLatin1String("_VersionVcs"));
	}

	// NOTE: Only known settings will be saved.
	
	// Save known settings to the configuration file.
	Q_D(const ConfigStore);
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		if (def->flags & ConfigDefaults::DefaultSetting::DEF_NO_SAVE)
			continue;

		const QString key = QLatin1String(def->key);
		QVariant value = d->settings.value(key, QLatin1String(def->value));
		if (def->hex_digits > 0) {
			// Convert to hexadecimal.
			unsigned int uint_val = value.toString().toUInt(nullptr, 0);
			QString str = QLatin1String("0x") +
					QString::number(uint_val, 16).toUpper().rightJustified(4, QChar(L'0'));
			value = str;
		}

		qSettings.setValue(key, value);
	}

	// Save the PathConfig settings.
	qSettings.beginGroup(QLatin1String("Directories"));
	d->pathConfig->save(&qSettings);
	qSettings.endGroup();

	// Save the Recent ROMs settings.
	qSettings.beginGroup(QLatin1String("Recent_ROMs"));
	d->recentRoms->save(&qSettings);
	qSettings.endGroup();

	// Save the key configuration.
	qSettings.beginGroup(QLatin1String("Shortcut_Keys"));
	d->keyConfig.save(&qSettings);
	qSettings.endGroup();

	// Save the controller configuration.
	// TODO: Rework this with the upcoming all-in-one IoManager.
	qSettings.beginGroup(QLatin1String("Controllers"));
	CtrlConfig::save(qSettings, m_keyManager);
	qSettings.endGroup();

	return 0;
}

/**
 * Save the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(void) const
{
	Q_D(const ConfigStore);
	const QString cfgFilename = (d->pathConfig->configPath() +
		QLatin1String(ConfigDefaults::DefaultConfigFilename));
	return save(cfgFilename);
}

/**
 * Register an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::registerChangeNotification(const QString &property, QObject *object, const char *method)
{
	if (!object || !method || property.isEmpty())
		return;

	// Get the vector of signal maps for this property.
	Q_D(ConfigStore);
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap>* signalMapVector =
		d->signalMaps.value(property, nullptr);
	if (!signalMapVector) {
		// No vector found. Create one.
		signalMapVector = new QVector<ConfigStorePrivate::SignalMap>();
		d->signalMaps.insert(property, signalMapVector);
	}

	// Look up the method index.
	int method_idx = ConfigStorePrivate::LookupQtMethod(object, method);
	if (method_idx < 0) {
		// NOTE: The first character of method indicates whether it's a signal or slot.
		// This is useless in error messages, so we'll use method+1.
		if (*method != 0)
			method++;
		fprintf(stderr, "ConfigStore::registerChangeNotification(): "
			"No such method %s::%s\n",
			object->metaObject()->className(), method);
		return;
	}

	// Add this object and slot to the signal maps vector.
	ConfigStorePrivate::SignalMap smap;
	smap.object = object;
	smap.method_idx = method_idx;
	signalMapVector->append(smap);
}

/**
 * Unregister an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::unregisterChangeNotification(const QString &property, QObject *object, const char *method)
{
	if (!object || !method || property.isEmpty())
		return;

	// Get the vector of signal maps for this property.
	Q_D(ConfigStore);
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap>* signalMapVector =
		d->signalMaps.value(property, nullptr);
	if (!signalMapVector)
		return;

	// Get the method index.
	int method_idx = -1;
	if (method != nullptr) {
		method_idx = ConfigStorePrivate::LookupQtMethod(object, method);
		if (method_idx < 0)
			return;
	}

	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	for (int i = (signalMapVector->size() - 1); i >= 0; i--) {
		const ConfigStorePrivate::SignalMap &smap = signalMapVector->at(i);
		if (smap.object.isNull()) {
			signalMapVector->remove(i);
		} else if (smap.object == object) {
			// Found the object.
			if (method == nullptr || method_idx == smap.method_idx) {
				// Found a matching signal map.
				signalMapVector->remove(i);
			}
		}
	}
}

/**
 * Notify all registered objects that configuration settings have changed.
 * Useful when starting the emulator.
 */
void ConfigStore::notifyAll(void)
{
	// Invoke methods for registered objects.
	Q_D(ConfigStore);
	QMutexLocker mtxLocker(&d->mtxSignalMaps);

	foreach (QString property, d->signalMaps.keys()) {
		QVector<ConfigStorePrivate::SignalMap> *signalMapVector =
			d->signalMaps.value(property);
		if (signalMapVector->isEmpty())
			continue;

		// Get the property value.
		const QVariant value = d->settings.value(property);

		// Process the signal map vector in reverse-order.
		// Reverse order makes it easier to remove deleted objects.
		for (int i = (signalMapVector->size() - 1); i >= 0; i--) {
			const ConfigStorePrivate::SignalMap &smap = signalMapVector->at(i);
			if (smap.object.isNull()) {
				signalMapVector->remove(i);
			} else {
				// Invoke this method.
				ConfigStorePrivate::InvokeQtMethod(smap.object, smap.method_idx, value);
			}
		}
	}
}

/**
 * Get the main configuration path. (GCPATH_CONFIG)
 * @return Main configuration path.
 */
QString ConfigStore::configPath(void) const
{
	Q_D(const ConfigStore);
	return d->pathConfig->configPath();
}

/**
 * Get the specified configuration path.
 * @param path Configuration path to get. (Invalid paths act like GCPATH_CONFIG.)
 * @return Configuration path.
 */
QString ConfigStore::configPath(PathConfig::ConfigPath path) const
{
	Q_D(const ConfigStore);
	return d->pathConfig->configPath(path);
}

/**
 * Get a const pointer to the PathConfig object.
 * @return Const pointer to the PathConfig object.
 */
const PathConfig *ConfigStore::pathConfigObject(void) const
{
	Q_D(const ConfigStore);
	return d->pathConfig;
}

/** Recent ROMs. **/

/**
 * Update the Recent ROMs list.
 * @param filename ROM filename.
 * @param z_filename Filename of ROM within archive.
 * @param sysId System ID.
 */
void ConfigStore::recentRomsUpdate(const QString &filename,
				   const QString &z_filename,
				   LibGens::Rom::MDP_SYSTEM_ID sysId)
{
	Q_D(ConfigStore);
	d->recentRoms->update(filename, z_filename, sysId);
}

/**
 * Get a const pointer to the Recent ROMs object.
 * @return Const pointer to the Recent ROMs object.
 */
const RecentRoms *ConfigStore::recentRomsObject(void) const
{
	Q_D(const ConfigStore);
	return d->recentRoms;
}

/**
 * Get a Recent ROMs entry.
 * @param id Recent ROM ID.
 */
RecentRom_t ConfigStore::recentRomsEntry(int id) const
{
	Q_D(const ConfigStore);
	return d->recentRoms->getRom(id);
}

/** Key configuration. **/

/**
 * Get the action associated with a GensKey_t.
 * @param key GensKey_t.
 * @return Action ID.
 */
int ConfigStore::keyToAction(GensKey_t key) const
{
	Q_D(const ConfigStore);
	return d->keyConfig.keyToAction(key);
}

/**
 * Get the GensKey_t associated with an action.
 * @param actoin Action ID.
 * @return GensKey_t.
 */
GensKey_t ConfigStore::actionToKey(int action) const
{
	Q_D(const ConfigStore);
	return d->keyConfig.actionToKey(action);
}

}
