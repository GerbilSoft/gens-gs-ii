/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfig.hpp: Controller configuration.                               *
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

#ifndef __GENS_QT4_CTRLCONFIG_HPP__
#define __GENS_QT4_CTRLCONFIG_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
class QSettings;

// LibGens includes.
#include "libgens/IO/IoBase.hpp"

namespace GensQt4
{

class CtrlConfigPrivate;

class CtrlConfig : public QObject
{
	Q_OBJECT
	
	public:
		CtrlConfig(QObject *parent = 0);
		~CtrlConfig();
		
		// Dirty flag.
		bool isDirty(void) const;
		void clearDirty(void);
		
		// Maximum number of buttons.
		static const int MAX_BTNS = 12;
		
		// Controller ports.
		enum CtrlPort_t
		{
			// System controller ports.
			PORT_1		= 0,
			PORT_2		= 1,
			
			// Team Player, Port 1.
			PORT_TP1A	= 2,
			PORT_TP1B	= 3,
			PORT_TP1C	= 4,
			PORT_TP1D	= 5,
			
			// Team Player, Port 2.
			PORT_TP2A	= 6,
			PORT_TP2B	= 7,
			PORT_TP2C	= 8,
			PORT_TP2D	= 9,
			
			// 4-Way Play.
			PORT_4WPA	= 10,
			PORT_4WPB	= 11,
			PORT_4WPC	= 12,
			PORT_4WPD	= 13,
			
			PORT_MAX
		};
		
		/**
		 * load(): Load controller configuration from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param settings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QSettings& settings);
		
		/**
		 * save(): Save controller configuration to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param settings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int save(QSettings& settings);
		
		LibGens::IoBase *updatePort1(LibGens::IoBase *oldPort) const;
	
	private:
		friend class CtrlConfigPrivate;
		CtrlConfigPrivate *d;
		Q_DISABLE_COPY(CtrlConfig)
};

}

#endif /* __GENS_QT4_CTRLCONFIG_HPP__ */
