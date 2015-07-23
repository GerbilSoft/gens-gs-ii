/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoMasterTap.hpp: Sega Master System custom multitap adapter.            *
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

/**
 * Master Tap adapter for "BOoM"
 * References:
 * - http://www.smspower.org/Homebrew/BOoM-SMS
 * - http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png
 */

#ifndef __LIBGENS_IO_IOMASTERTAP_HPP__
#define __LIBGENS_IO_IOMASTERTAP_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class IoMasterTap : public Device
{
	public:
		IoMasterTap();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoMasterTap(const IoMasterTap &);
		IoMasterTap &operator=(const IoMasterTap &);

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

		/**
		 * Set a sub-device.
		 * Used for multitaps.
		 * @param virtPort Virtual port number. (0-3)
		 * @param ioDevice I/O device.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int setSubDevice(int virtPort, Device *ioDevice) final;

	private:
		/**
		 * Connected controllers.
		 * NOTE: This object does NOT own these IoDevices.
		 */
		Device *pads[4];

		// Scanline counter.
		int scanlines;
};

} }

#endif /* __LIBGENS_IO_IOMASTERTAP_HPP__ */
