/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DevManager.cpp: Input Device Manager.                                   *
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

#include "DevManager.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

/** Static class variables. **/

/**
 * ms_DevFn[]: Device handler functions.
 * Currently supports up to 4 device types.
 */
DevManager::DeviceHandler_fn DevManager::ms_DevFn[4];


/**
 * Init(): Initialize DevManager.
 */
void DevManager::Init(void)
{
	// Clear ms_DevFn[].
	memset(ms_DevFn, 0x00, sizeof(ms_DevFn));
}

/**
 * End(): Shut down DevManager.
 */
void DevManager::End(void)
{
	// Clear ms_DevFn[].
	memset(ms_DevFn, 0x00, sizeof(ms_DevFn));
}


/**
 * RegisterDeviceHandler(): Register a device handler function.
 * @param devType Device type ID.
 * @param fn Device handler function.
 * @return 0 on success; non-zero on error.
 */
int DevManager::RegisterDeviceHandler(int devType, DeviceHandler_fn fn)
{
	// Make sure the device type ID is in range.
	if (devType < 0 || devType > MAX_DEVICE_TYPES)
		return 1;
	
	// Check if a function is already registered.
	if (ms_DevFn[devType] && ms_DevFn[devType] != fn)
		return 2;
	
	// Register the function.
	ms_DevFn[devType] = fn;
	return 0;
}

/**
 * UnregisterDeviceHandler(): Unregister a device handler function.
 * @param devType Device type ID.
 * @param fn Device handler function.
 * @return 0 on success; non-zero on error.
 */
int DevManager::UnregisterDeviceHandler(int devType, DeviceHandler_fn fn)
{
	// Make sure the device type ID is in range.
	if (devType < 0 || devType > MAX_DEVICE_TYPES)
		return 1;
	
	// Make sure the function is registered.
	if (ms_DevFn[devType] != fn)
		return 2;
	
	// Unregister the function.
	ms_DevFn[devType] = NULL;
	return 0;
}

}
