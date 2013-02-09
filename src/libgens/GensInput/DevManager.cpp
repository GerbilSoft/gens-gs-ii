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

// C includes. (C++ namespace)
#include <cstring>

namespace LibGens
{

/** Static class variables. **/

/**
 * Device handler functions and parameters.
 */
DevManager::DeviceHandler_fn DevManager::ms_DevFn[MAX_DEVICE_TYPES];
void *DevManager::ms_DevParam[MAX_DEVICE_TYPES];


/**
 * Initialize DevManager.
 */
void DevManager::Init(void)
{
	// Clear ms_DevFn[] and ms_DevParam[].
	memset(ms_DevFn, 0x00, sizeof(ms_DevFn));
	memset(ms_DevParam, 0x00, sizeof(ms_DevParam));
}

/**
 * Shut down DevManager.
 */
void DevManager::End(void)
{
	// Clear ms_DevFn[] and ms_DevParam[].
	memset(ms_DevFn, 0x00, sizeof(ms_DevFn));
	memset(ms_DevParam, 0x00, sizeof(ms_DevParam));
}


/**
 * Register a device handler function.
 * @param devType Device type ID.
 * @param fn Device handler function.
 * @param param Parameter to pass to the device handler function.
 * @return 0 on success; non-zero on error.
 */
int DevManager::RegisterDeviceHandler(int devType, DeviceHandler_fn fn, void *param)
{
	// Make sure the device type ID is in range.
	if (devType < 0 || devType > MAX_DEVICE_TYPES)
		return 1;
	
	// Check if a function is already registered.
	if (ms_DevFn[devType] && ms_DevFn[devType] != fn)
		return 2;
	
	// Register the function.
	ms_DevFn[devType] = fn;
	ms_DevParam[devType] = param;
	return 0;
}

/**
 * Unregister a device handler function.
 * @param devType Device type ID.
 * @param fn Device handler function.
 * @param param Parameter specified when registering the device handler function.
 * @return 0 on success; non-zero on error.
 */
int DevManager::UnregisterDeviceHandler(int devType, DeviceHandler_fn fn, void *param)
{
	// Make sure the device type ID is in range.
	if (devType < 0 || devType > MAX_DEVICE_TYPES)
		return 1;

	// Make sure the function is registered.
	if (ms_DevFn[devType] != fn || ms_DevParam[devType] != param)
		return 2;

	// Unregister the function.
	ms_DevFn[devType] = nullptr;
	ms_DevParam[devType] = nullptr;
	return 0;
}

}
