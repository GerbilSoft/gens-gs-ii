/***************************************************************************
 * libgenskeys: Gens Key Handling Library.                                 *
 * KeyManager.hpp: Key manager.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#ifndef __LIBGENSKEYS_KEYMANAGER_HPP__
#define __LIBGENSKEYS_KEYMANAGER_HPP__

#include "GensKey_t.h"
#include "libgens/IO/IoManager.hpp"

namespace LibGensKeys
{

class KeyManagerPrivate;

class KeyManager
{
	public:
		KeyManager();
		~KeyManager();

	private:
		friend class KeyManagerPrivate;
		KeyManagerPrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensKeys-specific version of Q_DISABLE_COPY().
		KeyManager(const KeyManager &);
		KeyManager &operator=(const KeyManager &);

	public:
		LibGens::IoManager *ioManager(void) const;
		void setIoManager(LibGens::IoManager *ioManager);

		/**
		 * Update the I/O manager with the current key states.
		 */
		void updateIoManager(void);

		/**
		 * Get the keymap for the specified Virtual Port.
		 * @param virtPort I/O Manager Virtual Port.
		 * @param keymap Keymap.
		 * @param siz Size of keymap.
		 * @return 0 on success; non-zero on error.
		 */
		int keymap(int virtPort, GensKey_t *keymap, int siz) const;

		/**
		 * Set the keymap for the specified Virtual Port.
		 * @param virtPort I/O Manager Virtual Port.
		 * @param keymap Keymap.
		 * @param siz Number of keys in the keymap.
		 * @return 0 on success; non-zero on error.
		 */
		int setKeymap(int virtPort, const GensKey_t *keymap, int siz);

		/**
		 * Keyboard: "Key Down" event.
		 * @param keycode Key code.
		 */
		void keyDown(GensKey_t keycode);

		/**
		 * Keyboard: "Key Up" event.
		 * @param keycode Key code.
		 */
		void keyUp(GensKey_t keycode);

		/**
		 * Check if a key is pressed.
		 * @param keycode Key code.
		 * @return True if pressed; false if not.
		 */
		bool isKeyPressed(GensKey_t keycode) const;
};

}

#endif /* __LIBGENSKEYS_KEYMANAGER_HPP__ */
