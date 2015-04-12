/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.cpp: I/O manager. (Button Names)                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#include "IoManager.hpp"

// C includes. (C++ namespace)
#include <cassert>

namespace LibGens
{

IoManager::ButtonName_t IoManager::ButtonName(IoType_t ioType, int btnIdx)
{
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);
	assert(btnIdx >= 0 && btnIdx < BTNI_MAX);

	// TODO: Use AND optimizations?
	switch (ioType) {
		case IOT_3BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_B:		return BTNNAME_B;
				case BTNI_C:		return BTNNAME_C;
				case BTNI_A:		return BTNNAME_A;
				case BTNI_START:	return BTNNAME_START;
				default:
					break;
			}
			break;

		case IOT_6BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_B:		return BTNNAME_B;
				case BTNI_C:		return BTNNAME_C;
				case BTNI_A:		return BTNNAME_A;
				case BTNI_START:	return BTNNAME_START;
				case BTNI_Z:		return BTNNAME_Z;
				case BTNI_Y:		return BTNNAME_Y;
				case BTNI_X:		return BTNNAME_X;
				case BTNI_MODE:		return BTNNAME_MODE;
				default:
					break;
			}
			break;

		case IOT_2BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_1:		return BTNNAME_1;
				case BTNI_2:		return BTNNAME_2;
				case BTNI_STARTPAUSE:	return BTNNAME_STARTPAUSE;
				default:
					break;
			}
			break;

		/** Miscellaneous Master System peripherals. **/

		case IOT_PADDLE:
			switch (btnIdx) {
				case BTNI_PADDLE_1:	return BTNNAME_1;
				case BTNI_PADDLE_2:	return BTNNAME_2;
				default:
					break;
			}
			break;

		case IOT_SPORTS_PAD:
			switch (btnIdx) {
				case BTNI_SPAD_1:	return BTNNAME_1;
				case BTNI_SPAD_2:	return BTNNAME_2;
				default:
					break;
			}
			break;

		/** Miscellaneous Mega Drive peripherals. **/

		case IOT_MEGA_MOUSE:
			switch (btnIdx) {
				case BTNI_MOUSE_LEFT:	return BTNNAME_MOUSE_LEFT;
				case BTNI_MOUSE_RIGHT:	return BTNNAME_MOUSE_RIGHT;
				case BTNI_MOUSE_MIDDLE:	return BTNNAME_MOUSE_MIDDLE;
				case BTNI_MOUSE_START:	return BTNNAME_MOUSE_START;
				default:
					break;
			}
			break;

		case IOT_XE_1AP:
			switch (btnIdx) {
				case BTNI_XE1AP_SELECT:	return BTNNAME_SELECT;
				case BTNI_XE1AP_START:	return BTNNAME_START;
				case BTNI_XE1AP_E2:	return BTNNAME_E2;
				case BTNI_XE1AP_E1:	return BTNNAME_E1;
				case BTNI_XE1AP_D:	return BTNNAME_D;
				case BTNI_XE1AP_C:	return BTNNAME_C;
				case BTNI_XE1AP_B:	return BTNNAME_B;
				case BTNI_XE1AP_A:	return BTNNAME_A;
				default:
					break;
			}
			break;

		/** ColecoVision. **/

		case IOT_COLECOVISION:
			switch (btnIdx) {
				case BTNI_UP:			return BTNNAME_UP;
				case BTNI_DOWN:			return BTNNAME_DOWN;
				case BTNI_LEFT:			return BTNNAME_LEFT;
				case BTNI_RIGHT:		return BTNNAME_RIGHT;
				case BTNI_CV_TL_YELLOW:		return BTNNAME_CV_TL_YELLOW;
				case BTNI_CV_TR_RED:		return BTNNAME_CV_TR_RED;
				case BTNI_CV_BLUE:		return BTNNAME_CV_BLUE;
				case BTNI_CV_PURPLE:		return BTNNAME_CV_PURPLE;
				case BTNI_CV_KEYPAD_1:		return BTNNAME_CV_KEYPAD_1;
				case BTNI_CV_KEYPAD_2:		return BTNNAME_CV_KEYPAD_2;
				case BTNI_CV_KEYPAD_3:		return BTNNAME_CV_KEYPAD_3;
				case BTNI_CV_KEYPAD_4:		return BTNNAME_CV_KEYPAD_4;
				case BTNI_CV_KEYPAD_5:		return BTNNAME_CV_KEYPAD_5;
				case BTNI_CV_KEYPAD_6:		return BTNNAME_CV_KEYPAD_6;
				case BTNI_CV_KEYPAD_7:		return BTNNAME_CV_KEYPAD_7;
				case BTNI_CV_KEYPAD_8:		return BTNNAME_CV_KEYPAD_8;
				case BTNI_CV_KEYPAD_9:		return BTNNAME_CV_KEYPAD_9;
				case BTNI_CV_KEYPAD_ASTERISK:	return BTNNAME_CV_KEYPAD_ASTERISK;
				case BTNI_CV_KEYPAD_0:		return BTNNAME_CV_KEYPAD_0;
				case BTNI_CV_KEYPAD_OCTOTHORPE:	return BTNNAME_CV_KEYPAD_OCTOTHORPE;
				default:
					break;
			}
			break;

		case IOT_NONE:
		default:
			break;
	}

	return BTNNAME_UNKNOWN;
}

int IoManager::FirstLogicalButton(IoType_t ioType)
{
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);

	switch (ioType) {
		case IOT_3BTN:		return BTNI_UP;
		case IOT_6BTN:		return BTNI_UP;
		case IOT_2BTN:		return BTNI_UP;
		case IOT_PADDLE:	return BTNI_PADDLE_1;
		case IOT_SPORTS_PAD:	return BTNI_SPAD_1;
		case IOT_MEGA_MOUSE:	return BTNI_MOUSE_LEFT;
		case IOT_XE_1AP:	return BTNI_XE1AP_A;

		case IOT_NONE:
		default:
			break;
	}

	return BTNI_UNKNOWN;
}

int IoManager::NextLogicalButton(IoType_t ioType, int btnIdx)
{
	assert(ioType >= IOT_3BTN && ioType < IOT_MAX);
	assert(btnIdx >= 0 && btnIdx < BTNI_MAX);

	switch (ioType) {
		case IOT_3BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_START;
				case BTNI_START:	return BTNI_A;
				case BTNI_A:		return BTNI_B;
				case BTNI_B:		return BTNI_C;
				case BTNI_C:
				default:
					break;
			}
			break;

		case IOT_6BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_START;
				case BTNI_START:	return BTNI_A;
				case BTNI_A:		return BTNI_B;
				case BTNI_B:		return BTNI_C;
				case BTNI_C:		return BTNI_MODE;
				case BTNI_MODE:		return BTNI_X;
				case BTNI_X:		return BTNI_Y;
				case BTNI_Y:		return BTNI_Z;
				case BTNI_Z:
				default:
					break;
			}
			break;

		case IOT_2BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_1;
				case BTNI_1:		return BTNI_2;
				case BTNI_2:		return BTNI_STARTPAUSE;
				case BTNI_STARTPAUSE:
				default:
					break;
			}
			break;

		/** Miscellaneous Master System peripherals. **/

		case IOT_PADDLE:
			switch (btnIdx) {
				case BTNI_PADDLE_1:	return BTNI_PADDLE_2;
				case BTNI_PADDLE_2:
				default:
					break;
			}
			break;

		case IOT_SPORTS_PAD:
			switch (btnIdx) {
				case BTNI_SPAD_1:	return BTNI_SPAD_2;
				case BTNI_SPAD_2:
				default:
					break;
			}
			break;

		/** Miscellaneous Mega Drive peripherals. **/

		case IOT_MEGA_MOUSE:
			switch (btnIdx) {
				case BTNI_MOUSE_LEFT:	return BTNI_MOUSE_MIDDLE;
				case BTNI_MOUSE_MIDDLE:	return BTNI_MOUSE_RIGHT;
				case BTNI_MOUSE_RIGHT:	return BTNI_MOUSE_START;
				case BTNI_MOUSE_START:
				default:
					break;
			}
			break;

		case IOT_XE_1AP:
			switch (btnIdx) {
				case BTNI_XE1AP_A:	return BTNI_XE1AP_B;
				case BTNI_XE1AP_B:	return BTNI_XE1AP_C;
				case BTNI_XE1AP_C:	return BTNI_XE1AP_D;
				case BTNI_XE1AP_D:	return BTNI_XE1AP_E1;
				case BTNI_XE1AP_E1:	return BTNI_XE1AP_E2;
				case BTNI_XE1AP_E2:	return BTNI_XE1AP_START;
				case BTNI_XE1AP_START:	return BTNI_XE1AP_SELECT;
				case BTNI_XE1AP_SELECT:
				default:
					break;
			}
			break;

		default:
			break;
	}

	return BTNI_UNKNOWN;
}

}
