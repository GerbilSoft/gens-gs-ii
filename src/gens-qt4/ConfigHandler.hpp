/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigHandler.hpp: General configuration signal handler.                *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_CONFIGHANDLER_HPP__
#define __GENS_QT4_CONFIGHANDLER_HPP__

// Qt includes.
#include <QtCore/QObject>

// PathConfig.
#include "Config/PathConfig.hpp"

namespace GensQt4 {

// General configuration signal handler.
class ConfigHandler : public QObject
{
	Q_OBJECT
	
	public:
		ConfigHandler(QObject *parent = 0);

	private:
		typedef QObject super;
	private:
		Q_DISABLE_COPY(ConfigHandler)

	public slots:
		void extprgUnRAR_changed_slot(const QVariant &extprgUnRAR);

		void tmssRomFilename_changed_slot(const QVariant &tmssRomFilename);
		void tmssEnabled_changed_slot(const QVariant &tmssEnabled);
		
		/**
		 * A configuration path has been changed.
		 * @param path Configuration path.
		 * @param dir New directory.
		 */
		void pathChanged(GensQt4::PathConfig::ConfigPath path, const QString &dir);
};

}

#endif /* __GENS_QT4_CONFIGHANDLER_HPP__ */
