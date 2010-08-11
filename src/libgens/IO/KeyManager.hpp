/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * KeyManager.hpp: Keyboard manager.                                       *
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

#ifndef __LIBGENS_IO_KEYMANAGER_HPP__
#define __LIBGENS_IO_KEYMANAGER_HPP__

namespace LibGens
{

/**
 * @name Key values
 * Based on SDL_keysym.h.
 */
enum KeyVal
{
	/**
	 * @name ASCII-mapped keysyms
	 * The keyboard syms have been cleverly chosen to map to ASCII.
	 */
	/*@{*/
	KEYV_UNKNOWN		= 0,
	KEYV_FIRST		= 0,
	
	KEYV_BACKSPACE		= 8,
	KEYV_TAB		= 9,
	KEYV_CLEAR		= 12,
	KEYV_RETURN		= 13,
	KEYV_PAUSE		= 19,
	KEYV_ESCAPE		= 27,
	KEYV_SPACE		= 32,
	KEYV_EXCLAIM		= 33,
	KEYV_QUOTEDBL		= 34,
	KEYV_HASH		= 35,
	KEYV_DOLLAR		= 36,
	KEYV_PERCENT		= 37,
	KEYV_AMPERSAND		= 38,
	KEYV_QUOTE		= 39,
	KEYV_LEFTPAREN		= 40,
	KEYV_RIGHTPAREN		= 41,
	KEYV_ASTERISK		= 42,
	KEYV_PLUS		= 43,
	KEYV_COMMA		= 44,
	KEYV_MINUS		= 45,
	KEYV_PERIOD		= 46,
	KEYV_SLASH		= 47,
	KEYV_0			= 48,
	KEYV_1			= 49,
	KEYV_2			= 50,
	KEYV_3			= 51,
	KEYV_4			= 52,
	KEYV_5			= 53,
	KEYV_6			= 54,
	KEYV_7			= 55,
	KEYV_8			= 56,
	KEYV_9			= 57,
	KEYV_COLON		= 58,
	KEYV_SEMICOLON		= 59,
	KEYV_LESS		= 60,
	KEYV_EQUALS		= 61,
	KEYV_GREATER		= 62,
	KEYV_QUESTION		= 63,
	KEYV_AT			= 64,
	/** Uppercase letters are skipped. */
	KEYV_LEFTBRACKET	= 91,
	KEYV_BACKSLASH		= 92,
	KEYV_RIGHTBRACKET	= 93,
	KEYV_CARET		= 94,
	KEYV_UNDERSCORE		= 95,
	KEYV_BACKQUOTE		= 96,
	KEYV_a			= 97,
	KEYV_b			= 98,
	KEYV_c			= 99,
	KEYV_d			= 100,
	KEYV_e			= 101,
	KEYV_f			= 102,
	KEYV_g			= 103,
	KEYV_h			= 104,
	KEYV_i			= 105,
	KEYV_j			= 106,
	KEYV_k			= 107,
	KEYV_l			= 108,
	KEYV_m			= 109,
	KEYV_n			= 110,
	KEYV_o			= 111,
	KEYV_p			= 112,
	KEYV_q			= 113,
	KEYV_r			= 114,
	KEYV_s			= 115,
	KEYV_t			= 116,
	KEYV_u			= 117,
	KEYV_v			= 118,
	KEYV_w			= 119,
	KEYV_x			= 120,
	KEYV_y			= 121,
	KEYV_z			= 122,
	KEYV_BRACELEFT		= 123,
	KEYV_BAR		= 124,
	KEYV_BRACERIGHT		= 125,
	KEYV_TILDE		= 126,
	KEYV_DELETE		= 127,
	/* End of ASCII mapped keysyms */
	/*@}*/
	
	/** @name Numeric keypad */
	/*@{*/
	KEYV_KP0		= 0x80,
	KEYV_KP1		= 0x81,
	KEYV_KP2		= 0x82,
	KEYV_KP3		= 0x83,
	KEYV_KP4		= 0x84,
	KEYV_KP5		= 0x85,
	KEYV_KP6		= 0x86,
	KEYV_KP7		= 0x87,
	KEYV_KP8		= 0x88,
	KEYV_KP9		= 0x89,
	KEYV_KP_PERIOD		= 0x8A,
	KEYV_KP_DIVIDE		= 0x8B,
	KEYV_KP_MULTIPLY	= 0x8C,
	KEYV_KP_MINUS		= 0x8D,
	KEYV_KP_PLUS		= 0x8E,
	KEYV_KP_ENTER		= 0x8F,
	KEYV_KP_EQUALS		= 0x90,
	/*@}*/
	
	/** @name Arrows + Home/End pad */
	/*@{*/
	KEYV_UP			= 0x91,
	KEYV_DOWN		= 0x92,
	KEYV_RIGHT		= 0x93,
	KEYV_LEFT		= 0x94,
	KEYV_INSERT		= 0x95,
	KEYV_HOME		= 0x96,
	KEYV_END		= 0x97,
	KEYV_PAGEUP		= 0x98,
	KEYV_PAGEDOWN		= 0x99,
	/*@}*/
	
	/** @name Function keys */
	/*@{*/
	KEYV_F1			= 0xA0,
	KEYV_F2			= 0xA1,
	KEYV_F3			= 0xA2,
	KEYV_F4			= 0xA3,
	KEYV_F5			= 0xA4,
	KEYV_F6			= 0xA5,
	KEYV_F7			= 0xA6,
	KEYV_F8			= 0xA7,
	KEYV_F9			= 0xA8,
	KEYV_F10		= 0xA9,
	KEYV_F11		= 0xAA,
	KEYV_F12		= 0xAB,
	KEYV_F13		= 0xAC,
	KEYV_F14		= 0xAD,
	KEYV_F15		= 0xAE,
	KEYV_F16		= 0xAF,
	KEYV_F17		= 0xB0,
	KEYV_F18		= 0xB1,
	KEYV_F19		= 0xB2,
	KEYV_F20		= 0xB3,
	KEYV_F21		= 0xB4,
	KEYV_F22		= 0xB5,
	KEYV_F23		= 0xB6,
	KEYV_F24		= 0xB7,
	KEYV_F25		= 0xB8,
	KEYV_F26		= 0xB9,
	KEYV_F27		= 0xBA,
	KEYV_F28		= 0xBB,
	KEYV_F29		= 0xBC,
	KEYV_F30		= 0xBD,
	KEYV_F31		= 0xBE,
	KEYV_F32		= 0xBF,
	/*@}*/
	
	/** @name Key state modifier keys */
	KEYV_NUMLOCK		= 0xC0,
	KEYV_CAPSLOCK		= 0xC1,
	KEYV_SCROLLLOCK		= 0xC2,
	KEYV_LSHIFT		= 0xC3,
	KEYV_RSHIFT		= 0xC4,
	KEYV_LCTRL		= 0xC5,
	KEYV_RCTRL		= 0xC6,
	KEYV_LALT		= 0xC7,
	KEYV_RALT		= 0xC8,
	KEYV_LMETA		= 0xC9,
	KEYV_RMETA		= 0xCA,
	KEYV_LSUPER		= 0xCB,	/** Left "Windows" key */
	KEYV_RSUPER		= 0xCC,	/** Right "Windows" key */
	KEYV_MODE		= 0xCD,	/**< "Alt Gr" key */
	KEYV_COMPOSE		= 0xCE,
	KEYV_LHYPER		= 0xCF,
	KEYV_RHYPER		= 0xD0,
	KEYV_LDIRECTION		= 0xD1,
	KEYV_RDIRECTION		= 0xD2,
	/*@}*/
	
	/** @name Miscellaneous function keys */
	/*@{*/
	KEYV_HELP		= 0xE0,
	KEYV_PRINT		= 0xE1,
	KEYV_SYSREQ		= 0xE2,
	KEYV_BREAK		= 0xE3,
	KEYV_MENU		= 0xE4,
	KEYV_POWER		= 0xE5,
	KEYV_EURO		= 0xE6,
	KEYV_UNDO		= 0xE7,
	/*@}*/
	
	/** @name Multimedia/Internet keys */
	// NOTE: Only back and forward are implemented,
	// since ThinkPads have them as real keys.
	KEYV_BACK		= 0x100,
	KEYV_FORWARD		= 0x101,
	
	/* End of key listing. */
	KEYV_LAST
};

/**
 * @name Mouse buttons
 */
enum MouseButton
{
	MBTN_UNKNOWN	= 0,
	MBTN_LEFT	= 1,
	MBTN_MIDDLE	= 2,
	MBTN_RIGHT	= 3,
	MBTN_WHEELUP	= 4,
	MBTN_WHEELDOWN	= 5,
	MBTN_X1		= 6,
	MBTN_X2		= 7,
	
	MBTN_LAST
};

class KeyManager
{
	public:
		static void Init(void);
		static void End(void);
		
		static const char *GetKeyName(KeyVal key);
		
		static void KeyPressEvent(int key);
		static void KeyReleaseEvent(int key);
		
		static void MousePressEvent(int button);
		static void MouseReleaseEvent(int button);
		
		static inline bool IsKeyPressed(int key)
		{
			return ((key >= KEYV_UNKNOWN && key <= KEYV_LAST)
				? ms_KeyPress[key]
				: false);
		}
		
		static inline bool IsMouseButtonPressed(int button)
		{
			return ((button >= MBTN_UNKNOWN && button <= MBTN_LAST)
				? ms_MouseButton[button]
				: false);
		}
	
	protected:
		// Keypress array.
		static bool ms_KeyPress[KEYV_LAST];
		
		// Mouse button array.
		static bool ms_MouseButton[MBTN_LAST];
	
	private:
		KeyManager() { }
		~KeyManager() { }
};

}

#endif /* __LIBGENS_IO_KEYMANAGER_HPP__ */
