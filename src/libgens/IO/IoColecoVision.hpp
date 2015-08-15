/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoColecoVision.hpp: ColecoVision controller.                            *
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

/**
 * Reference:
 * - http://arcarc.xmission.com/Web%20Archives/Deathskull%20%28May-2006%29/games/tech/cvcont.html
 */

#ifndef __LIBGENS_IO_COLECOVISION_HPP__
#define __LIBGENS_IO_COLECOVISION_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class IoColecoVision : public Device
{
	public:
		IoColecoVision();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoColecoVision(const IoColecoVision &);
		IoColecoVision &operator=(const IoColecoVision &);

	public:
		/**
		 * Update the I/O device.
		 * Runs the internal device update.
		 */
		virtual void update(void) final;

	private:
		// TODO: Quadrature encoder.
};

} }

#endif /* __LIBGENS_IO_COLECOVISION_HPP__ */
