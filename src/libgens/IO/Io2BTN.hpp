/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io2BTN.hpp: Sega Master System 2-button controller.                     *
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

#ifndef __LIBGENS_IO_IO2BTN_HPP__
#define __LIBGENS_IO_IO2BTN_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class Io2BTN : public Device
{
	public:
		Io2BTN();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Io2BTN(const Io2BTN &);
		Io2BTN &operator=(const Io2BTN &);

	public:
		// Device type.
		// Should be overridden by subclasses.
		virtual IoManager::IoType_t type(void) const override;

		/**
		 * Update the I/O device.
		 * Runs the internal device update.
		 */
		virtual void update(void) override;

	protected:
		// SMS: Pause button.
		// Game Gear: Start button.
		// TODO
		bool pause;
};

} }

#endif /* __LIBGENS_IO_IO2BTN_HPP__ */
