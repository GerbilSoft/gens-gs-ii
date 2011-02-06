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

// Qt forward declarations.
class QKeyEvent;
class QMouseEvent;

namespace GensQt4
{

class KeyHandlerQt
{
	public:
		static void Init(GensActions *gensActions);
		static void End(void);
		
		static void KeyPressEvent(QKeyEvent *event);
		static void KeyReleaseEvent(QKeyEvent *event);
		
		static void MouseMoveEvent(QMouseEvent *event);
		static void MousePressEvent(QMouseEvent *event);
		static void MouseReleaseEvent(QMouseEvent *event);
		
		/**
		 * DevHandler(): LibGens Device Handler function.
		 * @param key Gens keycode. (~0 for Update; return value is true on success.)
		 * @return True if the key is pressed; false if it isn't.
		 */
		static bool DevHandler(GensKey_t key);
		
	protected:
		// QKeyEvent to LibGens Key Value.
		static int QKeyEventToKeyVal(QKeyEvent *event);
		static int NativeModifierToKeyVal(QKeyEvent *event);
		
		// Gens Actions Manager.
		static GensActions *ms_GensActions;
		
		// Keypress array.
		static bool ms_KeyPress[KEYV_LAST];
		
#if 0
		// TODO
		// Last mouse position.
		static bool m_lastMousePosValid;
		static QPoint m_lastMousePos;
#endif
	
	private:
		KeyHandlerQt() { }
		~KeyHandlerQt() { }
};

}

#endif /* __GENS_QT4_INPUT_KEYHANDLERQT_HPP__ */
