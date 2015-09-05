/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * KeyHandlerQt.hpp: Qt key remapping handler.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
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

#ifndef __GENS_QT4_INPUT_KEYHANDLERQT_HPP__
#define __GENS_QT4_INPUT_KEYHANDLERQT_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
class QKeyEvent;
class QMouseEvent;

// LibGensKeys
#include "libgenskeys/GensKey_t.h"
namespace LibGensKeys {
	class KeyManager;
}

namespace GensQt4 {

class KeyHandlerQt : public QObject
{
	Q_OBJECT

	public:
		KeyHandlerQt(QObject *parent = 0, LibGensKeys::KeyManager *keyManager = 0);
		~KeyHandlerQt();

	private:
		typedef QObject super;
	private:
		Q_DISABLE_COPY(KeyHandlerQt)

	public:
		LibGensKeys::KeyManager *keyManager(void) const;
		void setKeyManager(LibGensKeys::KeyManager *keyManager);

		// QKeyEvent to LibGens Key Value.
		static GensKey_t QKeyEventToKeyVal(QKeyEvent *event);
		static GensKey_t NativeModifierToKeyVal(QKeyEvent *event);

		/**
		 * Convert a GensKey_t to a Qt key value, with GensKey modifiers.
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
		// TODO: Move to a private class?

		// Key Manager.
		LibGensKeys::KeyManager *m_keyManager;

#if 0
		// TODO
		// Last mouse position.
		static bool m_lastMousePosValid;
		static QPoint m_lastMousePos;
#endif
};

}

#endif /* __GENS_QT4_INPUT_KEYHANDLERQT_HPP__ */
