/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * KeyNames.cpp: Qt key names.                                             *
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

#include "KeyNames.hpp"

// Needed for tr().
#include <QtCore/QCoreApplication>

namespace GensQt4
{

/**
 * OS-specific key names.
 */
#if defined(__APPLE__)
#define RETURN_KEYNAME "Return"
#define ENTER_KEYNAME "Enter"
#define SUPER_KEYNAME "Command"
#define ALT_KEYNAME "Option"
#define NO_MODIFIER_LR
#elif defined(_WIN32)
#define RETURN_KEYNAME "Enter"
#define ENTER_KEYNAME "Numpad Enter"
#define SUPER_KEYNAME "Win"
#define ALT_KEYNAME "Alt"
#else
#define RETURN_KEYNAME "Enter"
#define ENTER_KEYNAME "Numpad Enter"
#define SUPER_KEYNAME "Super"
#define ALT_KEYNAME "Alt"
#endif

// TODO: Qt refers to Windows keys as "Meta",
// but X11 refers to Windows keys as "Super".

static const char *const keyNames[KEYV_LAST] =
{
	// 0x00
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "None"), nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Backspace"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Tab"), nullptr, nullptr,
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Clear"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", RETURN_KEYNAME), nullptr, nullptr,
	nullptr, nullptr, nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Pause"),
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Escape"),
	nullptr, nullptr, nullptr, nullptr,

	// 0x20
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Space"), "!", "\"", "#",
	"$", "%", "&", "'",
	"(", ")", "*", "+", ",", "-", ".", "/",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", ":", ";", "<", "=", ">", "?",

	// 0x40
	"@", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "[", "\\", "]", "_", "^",

	// 0x60
	"`", "A", "B", "C", "D", "E", "F", "G",
	"H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W",
	"X", "Y", "Z", nullptr,
	nullptr, nullptr, nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Delete"),

	// 0x80
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 0"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 1"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 2"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 3"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 4"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 5"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 6"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 7"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 8"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad 9"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad ."),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad /"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad *"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad -"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad +"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", ENTER_KEYNAME),

	// 0x90
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Numpad ="),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Up"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Down"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Insert"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Home"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "End"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Page Up"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Page Down"), nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	/** @name 0xA0: Function keys */
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
	"F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
	"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",
	"F25", "F26", "F27", "F28", "F29", "F30", "F31", "F32",

#ifndef NO_MODIFIER_LR
	// System supports L/R modifiers.

	/** @name 0xC0: Key state modifier keys */
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Num Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Caps Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Scroll Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Shift"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Shift"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Ctrl"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Ctrl"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left " ALT_KEYNAME),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right " ALT_KEYNAME),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Meta"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Meta"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left " SUPER_KEYNAME),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right " SUPER_KEYNAME),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Alt-Gr"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Compose"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Hyper"),

	/** @name 0xD0: Key state modifier keys (continued) */
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Hyper"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Direction"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Direction"), nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
#else
	// System doesn't support L/R modifiers.
	// Map Left keys as general keys and don't map Right keys.

	/** @name 0xC0: Key state modifier keys */
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Num Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Caps Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Scroll Lock"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Shift"),
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Ctrl"),
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", ALT_KEYNAME),
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Meta"),
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", SUPER_KEYNAME),
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Alt-Gr"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Compose"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Hyper"),

	/** @name 0xD0: Key state modifier keys (continued) */
	nullptr, QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Direction"),
	nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
#endif /* NO_MODIFIER_LR */

	/** @name 0xE0: Miscellaneous function keys */
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Help"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Print Screen"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "SysRq"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Break"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Menu"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Power"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Euro"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Undo"),
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

	/** @name 0xF0: Mouse buttons */
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Unknown Mouse Button"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Left Mouse Button"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Middle Mouse Button"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Right Mouse Button"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Mouse Wheel Up"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Mouse Wheel Down"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Mouse Wheel Left"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Mouse Wheel Right"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Extra Mouse Button 1"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Extra Mouse Button 2"),
	nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	/** @name 0x100: Multimedia/Internet keys */
	// NOTE: Only back and forward are implemented,
	// since ThinkPads have them as real keys.
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Back"),
	QT_TRANSLATE_NOOP("GensQt4::KeyNames", "Forward")
};

/**
 * Get a key name.
 * @param keycode Gens keycode.
 * @return Key name, or empty string if invalid.
 */
QString KeyNames::keyName(GensKey_t keycode)
{
	GensKey_u gkey;
	gkey.keycode = keycode;
	if (gkey.type != GKT_KEYBOARD)
		return QString();
	if (gkey.key16 >= KEYV_LAST)
		return QString();

	return QCoreApplication::translate("GensQt4::KeyNames", keyNames[gkey.key16]);
}

}
