/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensKeyConfig.hpp: Gens key configuration.                              *
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

#ifndef __GENS_QT4_ACTIONS_GENSKEYCONFIG_HPP__
#define __GENS_QT4_ACTIONS_GENSKEYCONFIG_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QHash>

// Qt classes.
class QSettings;

// Gens keys.
#include "libgens/GensInput/GensKey_t.h"

namespace GensQt4
{

class GensKeyConfigPrivate;

class GensKeyConfig : public QObject
{
	Q_OBJECT
	
	public:
		GensKeyConfig(QObject *parent = 0);
		~GensKeyConfig();
		
		/**
		 * keyToAction(): Look up an action based on a GensKey_t value.
		 * @param key GensKey_t value. (WITH MODIFIERS)
		 * @return Action, or 0 if no action was found.
		 */
		int keyToAction(GensKey_t key);
		
		/**
		 * actionToKey(): Look up a GensKey_t based on an action value.
		 * @param action Action value.
		 * @return GensKey_t (WITH MODIFIERS), or 0 if no key was found.
		 */
		int actionToKey(int action);
		
		/**
		 * load(): Load key configuration from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QSettings *qSettings);
		
		/**
		 * save(): Save key configuration to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int save(QSettings *qSettings);
	
	private:
		GensKeyConfigPrivate *d;
		Q_DISABLE_COPY(GensKeyConfig);
};

}

#endif /* __GENS_QT4_ACTIONS_GENSKEYCONFIG_HPP__ */
