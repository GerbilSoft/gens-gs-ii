/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigStore.hpp: Configuration store.                                   *
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

#ifndef __GENS_QT4_CONFIG_CONFIGSTORE_HPP__
#define __GENS_QT4_CONFIG_CONFIGSTORE_HPP__

// Qt includes.
#include <QtCore/QObject>

namespace GensQt4
{

class ConfigStorePrivate;

class ConfigStore : public QObject
{
	Q_OBJECT
	
	public:
		ConfigStore(QObject *parent = 0);
	
	private:
		friend class ConfigStorePrivate;
		ConfigStorePrivate *d;
		Q_DISABLE_COPY(ConfigStore)
};

}

#endif /* __GENS_QT4_CONFIG_CONFIGSTORE_HPP__ */
