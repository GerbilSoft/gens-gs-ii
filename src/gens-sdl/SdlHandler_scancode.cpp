/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlHandler_scancode.cpp: SDL library handler: Scancode conversion.      *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "SdlHandler.hpp"

namespace GensSdl {

/**
 * Convert an SDL2 scancode to a Gens keycode.
 * @param scancode SDL2 scancode.
 * @return Gens keycode, or 0 if unsupported.
 */
GensKey_t SdlHandler::scancodeToGensKey(SDL_Scancode scancode)
{
	static const GensKey_t sdlToGens[SDL_NUM_SCANCODES] = {
		// SDL2 scancodes are based on HID usage page 0x07 (USB keyboard).
		// TODO: Add Gens keycodes for new keys supported in SDL2.

		// 0x00
		0, 0, 0, 0, KEYV_a, KEYV_b, KEYV_c, KEYV_d,
		KEYV_e, KEYV_f, KEYV_g, KEYV_h,
		KEYV_i, KEYV_j, KEYV_k, KEYV_l,

		// 0x10
		KEYV_m, KEYV_n, KEYV_o, KEYV_p,
		KEYV_q, KEYV_r, KEYV_s, KEYV_t,
		KEYV_u, KEYV_v, KEYV_w, KEYV_x,
		KEYV_y, KEYV_z, KEYV_1, KEYV_2,

		// 0x20
		KEYV_3, KEYV_4, KEYV_5, KEYV_6,
		KEYV_7, KEYV_8, KEYV_9, KEYV_0,
		KEYV_RETURN, KEYV_ESCAPE, KEYV_BACKSPACE, KEYV_TAB,
		KEYV_SPACE, KEYV_MINUS, KEYV_EQUALS, KEYV_LEFTBRACKET,

		// 0x30
		KEYV_RIGHTBRACKET, KEYV_BACKSLASH, 0, KEYV_SEMICOLON,
		// TODO: Add APOSTROPHE and GRAVE aliases.
		KEYV_QUOTE, KEYV_BACKQUOTE, KEYV_COMMA, KEYV_PERIOD,
		KEYV_SLASH, KEYV_CAPSLOCK, KEYV_F1, KEYV_F2,
		KEYV_F3, KEYV_F4, KEYV_F5, KEYV_F6,

		// 0x40
		KEYV_F7, KEYV_F8, KEYV_F9, KEYV_F10,
		// TODO: KEYV_PRINT == printscreen?
		KEYV_F11, KEYV_F12, KEYV_PRINT, KEYV_SCROLLLOCK,
		KEYV_PAUSE, KEYV_INSERT, KEYV_HOME, KEYV_PAGEUP,
		KEYV_DELETE, KEYV_END, KEYV_PAGEDOWN, KEYV_RIGHT,

		// 0x50
		// TODO: KEYV_KP_? aliases for keypad keys.
		KEYV_LEFT, KEYV_DOWN, KEYV_UP, KEYV_NUMLOCK,
		KEYV_KP_DIVIDE, KEYV_KP_MULTIPLY, KEYV_KP_MINUS, KEYV_KP_PLUS,
		KEYV_KP_ENTER, KEYV_KP1, KEYV_KP2, KEYV_KP3,
		KEYV_KP4, KEYV_KP5, KEYV_KP6, KEYV_KP7,

		// 0x60
		KEYV_KP8, KEYV_KP9, KEYV_KP0, KEYV_KP_PERIOD,
		0, KEYV_MENU /* Windows menu */, KEYV_POWER, KEYV_KP_EQUALS,
		KEYV_F13, KEYV_F14, KEYV_F15, KEYV_F16,
		KEYV_F17, KEYV_F18, KEYV_F19, KEYV_F20,

		// 0x70
		KEYV_F21, KEYV_F22, KEYV_F23, KEYV_F24,
		// Sun keys
		0 /* EXECUTE */, KEYV_HELP, 0 /* MENU */, 0 /* SELECT */,
		0 /* STOP */, 0 /* AGAIN */, KEYV_UNDO, 0 /* CUT */,
		0 /* COPY */, 0 /* PASTE */, 0 /* FIND */, 0 /* MUTE */,

		// 0x80
		0 /* VOLUMEUP */, 0 /* VOLUMEDOWN */, 0 /* LOCKINGCAPSLOCK */, 0 /* LOCKINGNUMLOCK */,
		0 /* LOCKINGSCROLLLOCK */, 0 /* KP_COMMA */, 0 /* KP_EQUALSAS400 */, 0 /* INTERNATIONAL1 */,
		0 /* INTERNATIONAL2 */, 0 /* INTERNATIONAL3 */, 0 /* INTERNATIONAL4 */, 0 /* INTERNATIONAL5 */,
		0 /* INTERNATIONAL6 */, 0 /* INTERNATIONAL7 */, 0 /* INTERNATIONAL8 */, 0 /* INTERNATIONAL9 */,

		// 0x90
		0 /* LANG1 */, 0 /* LANG2 */, 0 /* LANG3 */, 0 /* LANG4 */, 
		0 /* LANG4 */, 0 /* LANG5 */, 0 /* LANG6 */, 0 /* LANG7 */, 
		0 /* LANG9 */, 0 /* ALTERASE */, KEYV_SYSREQ, 0 /* CANCEL */,
		KEYV_CLEAR, 0 /* PRIOR */, 0 /* RETURN2 */, 0 /* SEPARATOR */,

		// 0xA0
		0 /* OUT */, 0 /* OPER */, 0 /* CLEARAGAIN */, 0 /* CRSEL */,
		0 /* EXSEL */, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,

		// 0xB0
		0 /* KP_00 */, 0 /* KP_000 */, 0 /* THOUSANDSSEPARATOR */, 0 /* DECIMALSEPARATOR */,
		0 /* CURRENCYUNIT */, 0 /* CURRENCYSUBUNIT */, KEYV_LEFTPAREN, KEYV_RIGHTPAREN,
		0 /* KP_LEFTBRACE */, 0 /* KP_RIGHTBRACE */, 0 /* KP_TAB */, 0 /* KP_BACKSPACE */,
		0 /* KP_A */, 0 /* KP_B */, 0 /* KP_C */, 0 /* KP_D */,

		// 0xC0
		// TODO: Rework Gens keys to more closely match SDL2?
		// This will require updating gens-qt4 and will break existing configurations.
		0 /* KP_E */, 0 /* KP_F */, 0 /* KP_XOR */, 0 /* KP_POWER */,
		KEYV_PERCENT, 0 /* KP_LESS */, 0 /* KP_GREATER */, KEYV_AMPERSAND,
		0 /* KP_DBLAMPERSAND */, 0 /* KP_VERTICALBAR */, 0 /* KP_DBLVERTICALBAR */, KEYV_COLON,
		KEYV_HASH, 0 /* KP_SPACE */, KEYV_AT, KEYV_EXCLAIM,

		// 0xD0
		0 /* KP_MEMSTORE */, 0 /* KP_MEMRECALL */, 0 /* KP_MEMCLEAR */, 0 /* KP_MEMADD */,
		0 /* KP_MEMSUBTRACT */, 0 /* MEM_MULTIPLY */, 0 /* KP_MEMDIVIDE */, 0 /* KP_PLUSMINUS */,
		0 /* KP_CLEAR */, 0 /* KP_CLEARENTRY */, 0 /* KP_BINARY */, 0 /* KP_OCTAL */,
		0 /* KP_DECIMAL */, 0 /* KP_HEXADECIMAL */, 0, 0,

		// 0xE0
		KEYV_LCTRL, KEYV_LSHIFT, KEYV_LALT, KEYV_LMETA,
		KEYV_RCTRL, KEYV_RSHIFT, KEYV_RALT, KEYV_RMETA,
		0, 0, 0, 0, 0, 0, 0, 0,

		// 0xF0
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		// 0x100
		0, KEYV_MODE, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, KEYV_BACK, KEYV_FORWARD,

		// TODO: Add remaining keys.
	};

	// FIXME: Actually check bounds?
	// Using a simple mask for performance right now.
#if 0
	if (scancode < 0 || scancode >= SDL_NUM_SCANCODES)
		return 0;
#endif
	return sdlToGens[scancode & 0x1FF];
}

}
