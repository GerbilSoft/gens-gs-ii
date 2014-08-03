/***************************************************************************
 * libgenskeys: Gens Key Handling Library.                                 *
 * KeyManager.cpp: Key manager.                                            *
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

#include "KeyManager.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace LibGensKeys
{

class KeyManagerPrivate
{
	public:
		KeyManagerPrivate(KeyManager *q);

	private:
		KeyManager *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensKeys-specific version of Q_DISABLE_COPY().
		KeyManagerPrivate(const KeyManagerPrivate &);
		KeyManagerPrivate &operator=(const KeyManagerPrivate &);

	public:
		/**
		 * Keymaps for all devices.
		 * - Index 0: Virtual Port.
		 * - Index 1: Key. (TODO: Use the next-highest power of two?)
		 */
		GensKey_t keymap[LibGens::IoManager::VIRTPORT_MAX][LibGens::IoManager::BTNI_MAX];

		/**
		 * Keyboard state.
		 * - Index: GensKeyVal_t
		 * - Value: True if pressed; false if not.
		 */
		bool kbdState[KEYV_MAX];
		

		// TODO: Improve performance by adding a GensKey_t to Device lookup?
		// (may be needed when joysticks are added)
};

/** KeyManagerPrivate **/

KeyManagerPrivate::KeyManagerPrivate(KeyManager *q)
	: q(q)
{
	// Initialize keymaps.
	// TODO: Load keymap from configuration?
	memset(keymap, 0, sizeof(keymap));

	// Initialize keyboard state.
	memset(kbdState, 0, sizeof(kbdState));

	// Set a basic keymap for port 1 for now.
	keymap[0][0] = KEYV_UP;
	keymap[0][1] = KEYV_DOWN;
	keymap[0][2] = KEYV_LEFT;
	keymap[0][3] = KEYV_RIGHT;
	keymap[0][4] = KEYV_d;
	keymap[0][5] = KEYV_s;
	keymap[0][6] = KEYV_a;
	keymap[0][7] = KEYV_RETURN;
}

/** KeyManager **/

KeyManager::KeyManager()
	: d(new KeyManagerPrivate(this))
{ }

KeyManager::~KeyManager()
{
	delete d;
}

/**
 * Update the I/O Manager with the current key states.
 * @param ioManager I/O Manager to update.
 */
void KeyManager::updateIoManager(LibGens::IoManager *ioManager)
{
#ifdef _WIN32
	// Windows: Update Shift/Control/Alt states.
	// TODO: Only do this if the input backend doesn't support L/R modifiers natively.
	// QWidget doesn't; GLFW does.
	// TODO: When should these key states be updated?
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	d->kbdState[KEYV_LSHIFT] = (!!(GetAsyncKeyState(VK_LSHIFT) & 0x8000));
	d->kbdState[KEYV_RSHIFT] = (!!(GetAsyncKeyState(VK_RSHIFT) & 0x8000));
	d->kbdState[KEYV_LCTRL]	= (!!(GetAsyncKeyState(VK_LCONTROL) & 0x8000));
	d->kbdState[KEYV_RCTRL]	= (!!(GetAsyncKeyState(VK_RCONTROL) & 0x8000));
	d->kbdState[KEYV_LALT]	= (!!(GetAsyncKeyState(VK_LMENU) & 0x8000));
	d->kbdState[KEYV_RALT]	= (!!(GetAsyncKeyState(VK_RMENU) & 0x8000));
#endif

	// Scan the keymap and determine buttons.
	for (int virtPort = 0; virtPort < LibGens::IoManager::VIRTPORT_MAX; virtPort++) {
		LibGens::IoManager::IoType_t ioType = ioManager->devType((LibGens::IoManager::VirtPort_t)virtPort);
		int numButtons = ioManager->NumDevButtons(ioType);

		uint32_t buttons = 0;
		const GensKey_t *port_keymap = &d->keymap[virtPort][numButtons - 1];
		for (int btn = numButtons - 1; btn >= 0; btn--) {
			buttons <<= 1;
			buttons |= isKeyPressed(*port_keymap--);
		}

		// Buttons are typically active-low. (except Mega Mouse)
		if (ioType != LibGens::IoManager::IOT_MEGA_MOUSE)
			buttons = ~buttons;

		ioManager->update(virtPort, buttons);
	}
}

/**
 * Get the keymap for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param keymap Keymap.
 * @param siz Size of keymap.
 * @return 0 on success; non-zero on error.
 */
int KeyManager::keymap(int virtPort, GensKey_t *keymap, int siz) const
{
	// TODO
}

/**
 * Set the keymap for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param keymap Keymap.
 * @param siz Number of keys in the keymap.
 * @return 0 on success; non-zero on error.
 */
int KeyManager::setKeymap(int virtPort, const GensKey_t *keymap, int siz)
{
	// TODO
}

/**
 * Check if a key is pressed.
 * @param keycode Key code.
 * @return True if pressed; false if not.
 */
bool KeyManager::isKeyPressed(GensKey_t keycode) const
{
	GensKey_u keyu;
	keyu.keycode = keycode;
	if (keyu.type != GKT_KEYBOARD)
		return false;
	if (keyu.dev_id != 0)
		return false;

	return d->kbdState[keyu.key16 & 0x1FF];
}

/** Keyboard **/

/**
 * Keyboard: "Key Down" event.
 * @param keycode Key code.
 */
void KeyManager::keyDown(GensKey_t keycode)
{
	GensKey_u keyu;
	keyu.keycode = keycode;
	if (keyu.type != GKT_KEYBOARD)
		return;
	if (keyu.dev_id != 0)
		return;

	d->kbdState[keyu.key16 & 0x1FF] = true;
}

/**
 * Keyboard: "Key Up" event.
 * @param keycode Key code.
 */
void KeyManager::keyUp(GensKey_t keycode)
{
	GensKey_u keyu;
	keyu.keycode = keycode;
	if (keyu.type != GKT_KEYBOARD)
		return;
	if (keyu.dev_id != 0)
		return;

	d->kbdState[keyu.key16 & 0x1FF] = false;
}

}
