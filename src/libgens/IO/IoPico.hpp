/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoPico.hpp: Sega Pico controller.                                       *
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

#ifndef __LIBGENS_IO_IOPICO_HPP__
#define __LIBGENS_IO_IOPICO_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class IoPico : public Device
{
	public:
		IoPico();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoPico(const IoPico &);
		IoPico &operator=(const IoPico &);

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

	public:
		/** Pico-specific functions. **/

		// Page 0 = title; pages 1-6 = regular pages.
		// FIXME: "Huckle & Lowly's Busiest Day Ever" doesn't have
		// page 5; page 6 is the "paint" section. May need to
		// add an "unusable page" feature.
		static const uint8_t PICO_MAX_PAGES = 7;

		/**
		 * Get the current page number.
		 * @return Page number. (0 == title; 1-6 == regular page)
		 */
		uint8_t picoCurPageNum(void) const;

		/**
		 * Set the current page number.
		 * @param pg Page number. (0 == title; 1-6 == regular page)
		 */
		void setPicoCurPageNum(uint8_t pg);

		/**
		 * Get the current page register value.
		 * @return Page register value.
		 */
		uint8_t picoCurPageReg(void) const;

		/**
		 * Read a Pico I/O port related to controller input.
		 * This maps to odd addresses in the range:
		 * - [800003, 80000D]
		 * @param address Address.
		 * @param d_out Data output.
		 * @return 0 on success; non-zero if address is invalid.
		 */
		int picoReadIO(uint32_t address, uint8_t *d_out) const;

	private:
		// Page number.
		uint8_t m_page_num;

		// Adjusted X/Y coordinates.
		int m_adj_x, m_adj_y;
};

} }

#endif /* __LIBGENS_IO_IOPICO_HPP__ */
