/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * KeyHandlerQt.hpp: Qt key remapping handler.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __GENS_QT4_INPUT_KEYHANDLERQT_HPP__
#define __GENS_QT4_INPUT_KEYHANDLERQT_HPP__

#include "libgens/GensInput/DevManager.hpp"

// Gens Action Manager.
#include "../actions/GensActions.hpp"

// Qt includes and classes.
#include <QtCore/QObject>
class QKeyEvent;
class QMouseEvent;

namespace GensQt4
{

class KeyHandlerQt : public QObject
{
	Q_OBJECT
	
	public:
		KeyHandlerQt(QObject *parent = 0, GensActions *gensActions = 0);
		~KeyHandlerQt();
		
		void setGensActions(GensActions *newGensActions);
		
		// QKeyEvent to LibGens Key Value.
		static GensKey_t QKeyEventToKeyVal(QKeyEvent *event);
		static GensKey_t NativeModifierToKeyVal(QKeyEvent *event);
		
		/**
		 * KeyValMToQtKey(): Convert a GensKey_t to a Qt key value, with GensKey modifiers.
		 * @param keyM Gens keycode, with modifiers.
		 * @return Qt key value, or 0 on error.
		 */
		static int KeyValMToQtKey(GensKey_t keyM);
		
		// QKeyEvent wrappers.
		void keyPressEvent(QKeyEvent *event);
		void keyReleaseEvent(QKeyEvent *event);
		
		// QMouseEvent wrappers.
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
	
	private:
		/**
		 * DevHandler(): LibGens Device Handler function. (STATIC function)
		 * @param param Parameter specified when registering the device handler function.
		 * @param key Gens keycode. (~0 for Update; return value is true on success.)
		 * @return True if the key is pressed; false if it isn't.
		 */
		static bool DevHandler(void *param, GensKey_t key);
		
		/**
		 * DevHandler(): LibGens Device Handler function. (member function)
		 * @param key Gens keycode. (~0 for Update; return value is true on success.)
		 * @return True if the key is pressed; false if it isn't.
		 */
		bool devHandler(GensKey_t key);
		
		// Gens Actions Manager.
		GensActions *m_gensActions;
		
		// Keypress array.
		bool m_keyPress[KEYV_LAST];
		
#if 0
		// TODO
		// Last mouse position.
		static bool m_lastMousePosValid;
		static QPoint m_lastMousePos;
#endif
	
	private slots:
		// GensActions destroyed slot.
		void gensActionsDestroyed(void);
};

}

#endif /* __GENS_QT4_INPUT_KEYHANDLERQT_HPP__ */
