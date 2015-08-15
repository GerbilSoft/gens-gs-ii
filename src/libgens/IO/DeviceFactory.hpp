/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DeviceFactory.hpp: I/O device factory.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBGENS_IO_DEVICEFACTORY_HPP__
#define __LIBGENS_IO_DEVICEFACTORY_HPP__

#include "IoManager.hpp"

namespace LibGens { namespace IO {

class Device;
class DeviceFactory
{
	private:
		// Static class.
		DeviceFactory() { }
		~DeviceFactory() { }

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		DeviceFactory(const DeviceFactory &);
		DeviceFactory &operator=(const DeviceFactory &);

	public:
		/**
		 * Create a Device for the given IoType.
		 * @param ioType I/O type.
		 * @param virtPort Virtual port number. (Used to enforce constraints.)
		 * @return Device, or nullptr on error.
		 */
		static Device *createDevice(IoManager::IoType_t ioType, int virtPort);
};

} }

#endif /* __LIBGENS_IO_DEVICEFACTORY_HPP__ */
