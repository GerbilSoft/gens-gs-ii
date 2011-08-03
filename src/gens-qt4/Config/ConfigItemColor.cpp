/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigItemColor.cpp: Gens configuration item. (QColor)                  *
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

#include "ConfigItemColor.hpp"

namespace GensQt4
{

/**
 * ConfigItemColor(): Create a new configuration item. (QColor)
 * @param section Section name.
 * @param key Key name.
 * @param def Default value. (QColor)
 * @param parent Parent for this QObject.
 */
ConfigItemColor::ConfigItemColor(const QString& section, const QString& key, const QColor& def, QObject *parent)
	: ConfigItem(section, key, QVariant(), parent)
	, m_def(def)
{
	// TODO: m_def is initialized after ConfigItem() loads the item.
	// Figure out a way to prevent that from happening.
	
	// Load the item again.
	load_item();
}


/**
 * ConfigItemColor(): Create a new configuration item. (QColor)
 * @param section_key Section+Key name. (format: "section/key")
 * @param def Default value. (QColor)
 * @param parent Parent for this QObject.
 */
ConfigItemColor::ConfigItemColor(const QString& section_key, const QColor& def, QObject *parent)
	: ConfigItem(section_key, QVariant(), parent)
	, m_def(def)
{
	// TODO: m_def is initialized after ConfigItem() loads the item.
	// Figure out a way to prevent that from happening.
	
	// Load the item again.
	load_item();
}


/**
 * load_item(): Load the configuration item from the settings file.
 */
void ConfigItemColor::load_item(void)
{
	m_value = ms_Settings->value(m_section_key, QColor()).value<QColor>();
	if (!m_value.isValid())
		m_value = m_def;
	this->ConfigItem::m_value = m_value.name();
	m_dirty = false;		// No longer dirty.
	
	// NOTE: emit_item() is NOT called here.
	// The calling function must call EmitAll() after loading settings.
}


/**
 * save_item(): Save the configuration item to the settings file.
 */
void ConfigItemColor::save_item(void)
{
	// TODO: Convert QColor::name() to uppercase?
	ms_Settings->setValue(m_section_key, m_value.name());
	m_dirty = false;	// No longer dirty.
}


/**
 * setValue(): Set the configuration item's value.
 * @param newValue New value for the configuration item.
 */
void ConfigItemColor::setValue(const QVariant& newValue)
{
	// Parse the value as a QColor.
	setValue(newValue.value<QColor>());
}


/**
 * setValue(): Set the configuration item's value.
 * @param newValue New value for the configuration item.
 */
void ConfigItemColor::setValue(const QColor& newValue)
{
	if (m_value == newValue)
		return;
	m_value = newValue;
	this->ConfigItem::m_value = m_value.name();
	
	// Value has changed.
	emit valueChanged(m_value);
	m_dirty = true;
}

}
