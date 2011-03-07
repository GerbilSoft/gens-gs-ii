/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DevManager.hpp: Input Device Manager.                                   *
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

#ifndef __LIBGENS_GENSINPUT_DEVMANAGER_HPP__
#define __LIBGENS_GENSINPUT_DEVMANAGER_HPP__

#include "GensKey_t.h"

namespace LibGens
{

class DevManager
{
	public:
		static void Init(void);
		static void End(void);
		
		/**
		 * DeviceHandler_fn: Device Handler function prototype.
		 * TODO: Add MDP_FNCALL or something similar?
		 * @param key Gens keycode. (~0 for Update; return value is true on success.)
		 * @return True if the key is pressed; false if it isn't.
		 */
		typedef bool (*DeviceHandler_fn)(GensKey_t key);
		
		/**
		 * RegisterDeviceHandler(): Register a device handler function.
		 * @param devType Device type ID.
		 * @param fn Device handler function.
		 * @return 0 on success; non-zero on error.
		 */
		static int RegisterDeviceHandler(int devType, DeviceHandler_fn fn);
		
		/**
		 * UnregisterDeviceHandler(): Unregister a device handler function.
		 * @param devType Device type ID.
		 * @param fn Device handler function.
		 * @return 0 on success; non-zero on error.
		 */
		static int UnregisterDeviceHandler(int devType, DeviceHandler_fn fn);
		
		/**
		 * Update(): Update the device handlers.
		 */
		static void Update(void);
		
		/**
		 * IsKeyPressed(): Check if a key is pressed.
		 * This should ONLY be called from IoBase functions from within LibGens!
		 * @param key Gens keycode.
		 * @return True if the key is pressed; false if it isn't.
		 */
		static bool IsKeyPressed(GensKey_t key);
	
	private:
		DevManager() { }
		~DevManager() { }
		
		/**
		 * ms_DevFn[]: Device handler functions.
		 * Currently supports up to 4 device types.
		 */
		static const int MAX_DEVICE_TYPES = 4;
		static DeviceHandler_fn ms_DevFn[MAX_DEVICE_TYPES];
		
		// Key names.
		static const char *ms_KeyNames[KEYV_LAST];
};


/**
 * Update(): Update the device handlers.
 */
void DevManager::Update(void)
{
	// Call the device handlers with keycode ~0.
	for (int devType = 0; devType < MAX_DEVICE_TYPES; devType++)
	{
		if (ms_DevFn[devType])
			ms_DevFn[devType]((GensKey_t)~0);
	}
}


/**
 * IsKeyPressed(): Check if a key is pressed.
 * This should ONLY be called from IoBase functions from within LibGens!
 * @param key Gens keycode.
 * @return True if the key is pressed; false if it isn't.
 */
bool DevManager::IsKeyPressed(GensKey_t key)
{
	GensKey_u gkey;
	gkey.keycode = key;
	if (gkey.type >= MAX_DEVICE_TYPES || !ms_DevFn[gkey.type])
		return false;
	return ms_DevFn[gkey.type](key);
}

}

#endif /* __LIBGENS_GENSINPUT_DEVMANAGER_HPP__ */
