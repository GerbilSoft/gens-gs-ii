/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FpsManager.hpp: FPS Manager class.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2012 by David Korth.                                 *
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

#ifndef __GENS_QT4_VBACKEND_FPSMANAGER_HPP__
#define __GENS_QT4_VBACKEND_FPSMANAGER_HPP__

#include <QtCore/QObject>

namespace GensQt4
{

class FpsManager : public QObject
{
	Q_OBJECT
	
	public:
		FpsManager(QObject *parent = 0);
	
	private:
		Q_DISABLE_COPY(FpsManager);
	
	public:
		/**
		 * Reset the FPS manager.
		 */
		void reset(void);
		
		/**
		 * Push an FPS value.
		 * @param fps FPS value.
		 */
		void push(double fps);
		
		/**
		 * Get the current average FPS.
		 * @return Current average FPS.
		 */
		double get(void);
	
	signals:
		/**
		 * The FPS manager has been updated.
		 * @param fps New average FPS.
		 */
		void updated(double fps);
	
	private:
		double m_fps[8];
		double m_fpsAvg;	// Average fps.
		int m_fpsPtr;		// Pointer to next fps slot to use.
};

/**
 * Get the current average FPS.
 * @return Current average FPS.
 */
inline double FpsManager::get(void)
	{ return m_fpsAvg; }

}

#endif /* __GENS_QT4_VBACKEND_FPSMANAGER_HPP__ */
