/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigItem.cpp: Gens configuration item.                                *
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

#include "ConfigItem.hpp"

// Qt includes.
#include <QtCore/QDir>
#include <QtCore/QTextCodec>

namespace GensQt4
{

/** Static member initialiation. **/

/**
 * ms_Settings: Settings file.
 * Opened on first ConfigItem instantiation; closed on last ConfigItem destruction.
 */
QSettings *ConfigItem::ms_Settings = NULL;

// Organization and program names.
const char *ConfigItem::ms_OrgName = "gens-gs-ii";
const char *ConfigItem::ms_PrgName = "gens-gs-ii";
const char *ConfigItem::ms_CfgFilename = "gens-gs-ii.conf";

// Reference count.
int ConfigItem::ms_RefCnt = 0;

// List of all ConfigItems.
// NOTE: This cannot be statically-allocated, because it might
// not get initialized before statically-allocated ConfigItem objects.
QList<ConfigItem*>* ConfigItem::ms_pLstItems = NULL;


/** Static functions. **/


/**
 * GetConfigPath(): Determine the configuration path.
 * @return Configuration path.
 */
QString ConfigItem::GetConfigPath(void)
{
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String(ms_OrgName),
				QLatin1String(ms_PrgName));
	
	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	QString cfgPath = settings.fileName();
	int sepChr = cfgPath.lastIndexOf(QChar(L'/'));
	if (sepChr >= 0)
		cfgPath.remove(sepChr + 1, cfgPath.size());
	
	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir dir(cfgPath);
	if (!dir.exists())
		dir.mkpath(cfgPath);
	
	// Return the configuration path.
	return cfgPath;
}


/**
 * Load(): Load (or reload) all configuration items.
 */
void ConfigItem::Load(void)
{
	for (int i = 0; i < ms_pLstItems->count(); i++)
		ms_pLstItems->at(i)->load_item();
	
	// NOTE: EmitAll() is NOT called here.
	// The calling function must call EmitAll() after loading settings.
}


/**
 * Save(): Save all configuration items.
 */
void ConfigItem::Save(void)
{
	for (int i = 0; i < ms_pLstItems->count(); i++)
		ms_pLstItems->at(i)->save_item();
}


/**
 * EmitAll(): Emit signals for all configuration items.
 */
void ConfigItem::EmitAll(void)
{
	for (int i = 0; i < ms_pLstItems->count(); i++)
		ms_pLstItems->at(i)->emit_item();
}


/** Instance functions. **/


/**
 * init(): Initialize this ConfigItem.
 * Called from ConfigItem::ConfigItem().
 */
void ConfigItem::init(void)
{
	// Check the reference count.
	if (ms_RefCnt == 0)
	{
		// Reference count is 0.
		const QString filename = GetConfigPath() + QChar(L'/') + QLatin1String(ms_CfgFilename);
		
		// Initialize the configuration file.
		ms_pLstItems = new QList<ConfigItem*>();
		ms_Settings = new QSettings(filename, QSettings::IniFormat);
		ms_Settings->setIniCodec(QTextCodec::codecForName("UTF-8"));
	}
	
	// Increment the reference count.
	ms_RefCnt++;
	
	// Load the configuration item from the QSettings object.
	load_item();
	
	// Register this ConfigItem in ms_LstItems.
	ms_pLstItems->append(this);
}


/**
 * ConfigItem(): Create a new configuration item.
 * @param section Section name.
 * @param key Key name.
 * @param def Default value.
 * @param parent Parent for this QObject.
 */
ConfigItem::ConfigItem(const QString& section, const QString& key, const QVariant& def, QObject *parent)
	: QObject(parent)
	, m_section_key(section + QChar(L'/') + key)
	, m_def(def)
	, m_dirty(false)
{
	// Initialize this ConfigItem.
	init();
}


/**
 * ConfigItem(): Create a new configuration item.
 * @param section_key Section+Key name. (format: "section/key")
 * @param def Default value.
 * @param parent Parent for this QObject.
 */
ConfigItem::ConfigItem(const QString& section_key, const QVariant& def, QObject *parent)
	: QObject(parent)
	, m_section_key(section_key)
	, m_def(def)
	, m_dirty(false)
{
	// Initialize this ConfigItem.
	init();
}


/**
 * ~ConfigItem(): Delete this configuration item.
 */
ConfigItem::~ConfigItem()
{
	// Save this configuration item.
	if (m_dirty)
		save_item();
	
	// Remove this ConfigItem from ms_pLstItems.
	ms_pLstItems->removeOne(this);
	
	// Decrement the reference count.
	ms_RefCnt--;
	if (ms_RefCnt <= 0)
	{
		// All configuration items have been deleted.
		ms_RefCnt = 0;
		delete ms_pLstItems;
		ms_pLstItems = NULL;
		
		// Close the settings file.
		delete ms_Settings;
		ms_Settings = NULL;
	}
}


/**
 * load_item(): Load the configuration item from the settings file.
 */
void ConfigItem::load_item(void)
{
	m_value = ms_Settings->value(m_section_key, m_def);
	m_dirty = false;		// No longer dirty.
	
	// NOTE: emit_item() is NOT called here.
	// The calling function must call EmitAll() after loading settings.
}


/**
 * save_item(): Save the configuration item to the settings file.
 */
void ConfigItem::save_item(void)
{
	ms_Settings->setValue(m_section_key, m_value);
	m_dirty = false;	// No longer dirty.
}


/**
 * setValue(): Set the configuration item's value.
 * @param newValue New value for the configuration item.
 */
void ConfigItem::setValue(const QVariant& newValue)
{
	if (m_value == newValue)
		return;
	m_value = newValue;
	
	// Value has changed.
	emit valueChanged(m_value);
	m_dirty = true;
}

}
