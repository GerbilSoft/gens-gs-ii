/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigItemColor.hpp: Gens configuration item. (QColor)                  *
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

#ifndef __GENS_QT4_CONFIG_CONFIGITEMCOLOR_HPP__
#define __GENS_QT4_CONFIG_CONFIGITEMCOLOR_HPP__

#include "ConfigItem.hpp"

// Qt includes.
#include <QtGui/QColor>

namespace GensQt4
{

class ConfigItemColor : public ConfigItem
{
	Q_OBJECT
	
	public:
		ConfigItemColor(const QString& section, const QString& key, const QColor& def, QObject *parent = 0);
		ConfigItemColor(const QString& section_key, const QColor& def, QObject *parent = 0);
		
		/**
		 * value(): Get the configuration item's value.
		 * @return Configuration item's value.
		 */
		QColor value(void);
		
		/**
		 * def(): Get the configuration item's default value.
		 * @return Configuration item's default value.
		 */
		QColor def(void);
		
		/**
		 * setValue(): Set the configuration item's value.
		 * @param newValue New value for the configuration item.
		 */
		void setValue(const QColor& newValue);
	
	signals:
		void valueChanged(const QColor& value);
	
	protected:
		const QColor m_def;		// Default value.
		QColor m_value;		// Configuration entry.
		bool m_dirty;			// Is this item dirty?
		
		/**
		 * load_item(): Load the configuration item from the settings file.
		 */
		void load_item(void);
		
		/**
		 * save_item(): Save the configuration item to the settings file.
		 */
		void save_item(void);
		
		/**
		 * emit_item(): Emit valueChanged() signal for this item.
		 */
		void emit_item(void);
	
	private:
		Q_DISABLE_COPY(ConfigItemColor)
};

/**
 * value(): Get the configuration item's value.
 * @return Configuration item's value.
 */
inline QColor ConfigItemColor::value(void)
	{ return m_value; }

/**
 * def(): Get the configuration item's default value.
 * @return Configuration item's default value.
 */
inline QColor ConfigItemColor::def(void)
	{ return m_def; }

/**
 * emit_item(): Emit valueChanged() signal for this item.
 */
inline void ConfigItemColor::emit_item(void)
	{ emit valueChanged(m_value); }

}

#endif /* __GENS_QT4_CONFIG_CONFIGITEMCOLOR_HPP__ */
