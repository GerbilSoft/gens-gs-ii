/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigItem.hpp: Gens configuration item.                                *
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

#ifndef __GENS_QT4_CONFIG_CONFIGITEM_HPP__
#define __GENS_QT4_CONFIG_CONFIGITEM_HPP__

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include <QtCore/QList>

namespace GensQt4
{

class ConfigItem : public QObject
{
	Q_OBJECT
	
	public:
		ConfigItem(const QString& section, const QString& key, const QVariant& def, QObject *parent = 0);
		ConfigItem(const QString& section_key, const QVariant& def, QObject *parent = 0);
		~ConfigItem();
		
		
		/** Static functions. **/
		
		/**
		 * GetConfigPath(): Determine the configuration path.
		 * @return Configuration filename.
		 */
		static QString GetConfigPath(void);
		
		/**
		 * Load(): Load (or reload) all configuration items.
		 */
		static void Load(void);
		
		/**
		 * Save(): Save all configuration items.
		 */
		static void Save(void);
		
		/**
		 * EmitAll(): Emit signals for all configuration items.
		 */
		static void EmitAll(void);
		
		
		/** Instance functions. **/
		
		/**
		 * value(): Get the configuration item's value.
		 * @return Configuration item's value.
		 */
		QVariant value(void);
		
		/**
		 * def(): Get the configuration item's default value.
		 * @return Configuration item's default value.
		 */
		QVariant def(void);
		
		/**
		 * setValue(): Set the configuration item's value.
		 * @param newValue New value for the configuration item.
		 */
		void setValue(const QVariant& newValue);
	
	signals:
		void valueChanged(const QVariant& value);
	
	protected:
		/** Static variables. **/
		
		/**
		 * ms_Settings: Settings file.
		 * Opened on first ConfigItem instantiation; closed on last ConfigItem destruction.
		 */
		static QSettings *ms_Settings;
		
		/** Instance variables. **/
		const QString m_section_key;	// Section+Key. (format: "section/key")
		const QVariant m_def;		// Default value.
		QVariant m_value;		// Configuration entry.
		bool m_dirty;			// Is this item dirty?
		
		/**
		 * init(): Initialize this ConfigItem.
		 * Called from ConfigItem::ConfigItem().
		 */
		void init(void);
		
		/**
		 * load_item(): Load the configuration item from the settings file.
		 */
		virtual void load_item(void);
		
		/**
		 * save_item(): Save the configuration item to the settings file.
		 */
		virtual void save_item(void);
		
		/**
		 * emit_item(): Emit valueChanged() signal for this item.
		 */
		void emit_item(void);
	
	private:
		Q_DISABLE_COPY(ConfigItem)
		
		// Organization and program names.
		static const char *ms_OrgName;
		static const char *ms_PrgName;
		static const char *ms_CfgFilename;
		
		// Reference count.
		static int ms_RefCnt;
		
		// List of all ConfigItems.
		static QList<ConfigItem*>* ms_pLstItems;
};

/**
 * value(): Get the configuration item's value.
 * @return Configuration item's value.
 */
inline QVariant ConfigItem::value(void)
	{ return m_value; }

/**
 * def(): Get the configuration item's default value.
 * @return Configuration item's default value.
 */
inline QVariant ConfigItem::def(void)
	{ return m_def; }

/**
 * emit_item(): Emit valueChanged() signal for this item.
 */
inline void ConfigItem::emit_item(void)
	{ emit valueChanged(m_value); }

}

#endif /* __GENS_QT4_CONFIG_CONFIGITEM_HPP__ */
