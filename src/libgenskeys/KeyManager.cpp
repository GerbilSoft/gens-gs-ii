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
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>

// C++ includes.
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "libgens/macros/common.h"
#include "libgens/IO/IoManager.hpp"
using LibGens::IoManager;

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
		GensKey_t keyMap[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX];

		/**
		 * Device types.
		 * - Index: Virtual Port.
		 * - Value: Device type.
		 */
		IoManager::IoType_t ioTypes[IoManager::VIRTPORT_MAX];

		/**
		 * Keyboard state.
		 * - Index: GensKeyVal_t
		 * - Value: True if pressed; false if not.
		 */
		bool kbdState[KEYV_MAX];

		// TODO: Improve performance by adding a GensKey_t to Device lookup?
		// (may be needed when joysticks are added)

		// Default keymap.
		static const IoManager::IoType_t def_ioTypes[IoManager::VIRTPORT_MAX];
		static const GensKey_t def_keyMap[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX];
};

/** KeyManagerPrivate **/

// Default controller configuration.
const IoManager::IoType_t KeyManagerPrivate::def_ioTypes[IoManager::VIRTPORT_MAX] =
{
	// System controller ports.
	IoManager::IOT_6BTN,	// Port 1
	IoManager::IOT_3BTN,	// Port 2

	// Team Player, Port 1.
	IoManager::IOT_NONE,	// Port TP1A
	IoManager::IOT_NONE,	// Port TP1B
	IoManager::IOT_NONE,	// Port TP1C
	IoManager::IOT_NONE,	// Port TP1D

	// Team Player, Port 2.
	IoManager::IOT_NONE,	// Port TP2A
	IoManager::IOT_NONE,	// Port TP2B
	IoManager::IOT_NONE,	// Port TP2C
	IoManager::IOT_NONE,	// Port TP2D

	// 4-Way Play.
	IoManager::IOT_NONE,	// Port 4WPA
	IoManager::IOT_NONE,	// Port 4WPB
	IoManager::IOT_NONE,	// Port 4WPC
	IoManager::IOT_NONE,	// Port 4WPD
};

const GensKey_t KeyManagerPrivate::def_keyMap[IoManager::VIRTPORT_MAX][IoManager::BTNI_MAX] =
{
	// Port 1
	// NOTE: Both shift keys are mapped to LSHIFT on Mac OS X.
	{KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,
	KEYV_s, KEYV_d, KEYV_a, KEYV_RETURN,
	KEYV_e, KEYV_w, KEYV_q,
#ifdef __APPLE__
	KEYV_LSHIFT
#else
	KEYV_RSHIFT
#endif
	},

	// Port 2 (TODO: This needs to be improved!)
	{KEYV_i, KEYV_k, KEYV_j, KEYV_l,
	KEYV_t, KEYV_y, KEYV_r, KEYV_u,
	0, 0, 0, 0},

	// Other ports are left undefined.

	// Team Player, Port 1.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1A
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1B
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1C
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP1D

	// Team Player, Port 2.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2A
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2B
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2C
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port TP2D

	// 4-Way Play.
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPA
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPC
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// Port 4WPD
};

KeyManagerPrivate::KeyManagerPrivate(KeyManager *q)
	: q(q)
{
	// Initialize keymaps to the defaults.
	memcpy(keyMap, def_keyMap, sizeof(keyMap));
	memcpy(ioTypes, def_ioTypes, sizeof(ioTypes));

	// Initialize keyboard state.
	memset(kbdState, 0, sizeof(kbdState));
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
 * Copy keymaps and device types, but not key states.
 * @param keyManager Source keyManager.
 */
void KeyManager::copyFrom(const KeyManager &keyManager)
{
	memcpy(d->keyMap, keyManager.d->keyMap, sizeof(d->keyMap));
	memcpy(d->ioTypes, keyManager.d->ioTypes, sizeof(d->ioTypes));
}

/**
 * Update the I/O Manager with the current key states.
 * @param ioManager I/O Manager to update.
 */
void KeyManager::updateIoManager(IoManager *ioManager)
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
	for (int virtPort = 0; virtPort < IoManager::VIRTPORT_MAX; virtPort++) {
		IoManager::IoType_t ioType = ioManager->devType((IoManager::VirtPort_t)virtPort);
		if (d->ioTypes[virtPort] != ioType) {
			// Update the device type.
			ioManager->setDevType(
				(IoManager::VirtPort_t)virtPort, 
				d->ioTypes[virtPort]);
			ioType = d->ioTypes[virtPort];
		}

		const int numButtons = ioManager->NumDevButtons(ioType);
		uint32_t buttons = 0;
		const GensKey_t *port_keyMap = &d->keyMap[virtPort][numButtons - 1];
		for (int btn = numButtons - 1; btn >= 0; btn--) {
			buttons <<= 1;
			buttons |= isKeyPressed(*port_keyMap--);
		}

		// Buttons are typically active-low.
		buttons = ~buttons;

		ioManager->update(virtPort, buttons);
	}
}

/**
 * Get the keymap for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param keyMap Keymap.
 * @param siz Size of keymap.
 * @return Number of keys copied on success; non-zero on error.
 */
int KeyManager::keyMap(IoManager::VirtPort_t virtPort, GensKey_t *keyMap, int siz) const
{
	if (virtPort < IoManager::VIRTPORT_1 || virtPort >= IoManager::VIRTPORT_MAX)
		return -EINVAL;

	const uint32_t *src_keyMap = d->keyMap[virtPort];
	int btns = std::min(siz, ARRAY_SIZE(d->keyMap[virtPort]));
	for (int i = 0; i < btns; i++) {
		keyMap[i] = src_keyMap[i];
	}

	return btns;
}

/**
 * Set the keymap for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param keyMap Keymap.
 * @param siz Number of keys in the keymap.
 * @return Number of keys copied on success; non-zero on error.
 */
int KeyManager::setKeyMap(IoManager::VirtPort_t virtPort, const GensKey_t *keyMap, int siz)
{
	if (virtPort < IoManager::VIRTPORT_1 || virtPort >= IoManager::VIRTPORT_MAX)
		return -EINVAL;

	uint32_t *dest_keyMap = d->keyMap[virtPort];
	int btns = std::min(siz, ARRAY_SIZE(d->keyMap[virtPort]));
	for (int i = 0; i < btns; i++) {
		dest_keyMap[i] = keyMap[i];
	}

	return btns;
}

/**
 * Reset virtual port to the default keymap.
 * @param virtPort I/O Manager Virtual port.
 * @return 0 on success; non-zero on error.
 */
int KeyManager::resetKeyMap(IoManager::VirtPort_t virtPort)
{
	if (virtPort < IoManager::VIRTPORT_1 || virtPort >= IoManager::VIRTPORT_MAX)
		return -EINVAL;

	d->ioTypes[virtPort] = d->def_ioTypes[virtPort];
	memcpy(d->keyMap[virtPort], d->def_keyMap[virtPort], sizeof(d->keyMap[virtPort]));
	return 0;
}

/**
 * Get the device type for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param ioType New device type.
 */
IoManager::IoType_t KeyManager::ioType(IoManager::VirtPort_t virtPort) const
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	return d->ioTypes[virtPort];
}

/**
 * Set the device type for the specified Virtual Port.
 * @param virtPort I/O Manager Virtual Port.
 * @param ioType New device type.
 */
void KeyManager::setIoType(IoManager::VirtPort_t virtPort, IoManager::IoType_t ioType)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	d->ioTypes[virtPort] = ioType;
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
