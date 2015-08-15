/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io6BTN.hpp: Sega Mega Drive 6-button controller.                        *
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

#ifndef __LIBGENS_IO_IO6BTN_HPP__
#define __LIBGENS_IO_IO6BTN_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class Io6BTN : public Device
{
	public:
		Io6BTN();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Io6BTN(const Io6BTN &);
		Io6BTN &operator=(const Io6BTN &);

	public:
		/**
		 * Reset Device data that only affects the device
		 * and not the emulation-side registers.
		 *
		 * Should be overridden by subclasses that have
		 * device-specific data.
		 */
		virtual void resetDev(void) final;

		/**
		 * Update the I/O device.
		 * Runs the internal device update.
		 */
		virtual void update(void) final;

		/**
		 * One scanline worth of time has passed.
		 * Needed for some devices that reset after a period of time,
		 * e.g. 6BTN controllers.
		 */
		virtual void update_onScanline(void) final;

	private:
		// Scanline counter.
		int scanlines;
};

} }

#endif /* __LIBGENS_IO_IO6BTN_HPP__ */
