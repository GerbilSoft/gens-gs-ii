/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * CdDrive.hpp: CD-ROM drive handler.                                      *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __LIBGENSCD_CDDRIVE_HPP__
#define __LIBGENSCD_CDDRIVE_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// Disc and drive type definitions.
#include "DiscType.h"

namespace LibGensCD
{

class CdDrivePrivate;

class CdDrive
{
	public:
		CdDrive(const std::string& filename);
		virtual ~CdDrive();

	private:
		friend class CdDrivePrivate;
		CdDrivePrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		CdDrive(const CdDrive &);
		CdDrive &operator=(const CdDrive &);

	public:
		bool isOpen(void) const;
		void close(void);
		std::string filename(void) const;

		std::string dev_vendor(void);
		std::string dev_model(void);
		std::string dev_firmware(void);

		/**
		 * Force a cache update.
		 * NOTE: Currently required for SPTI, since the
		 * MMC GET_EVENT_STATUS_NOTIFICATION command
		 * isn't working properly, and WM_DEVICECHANGE
		 * requires a window to receive notifications.
		 */
		void forceCacheUpdate(void);

		/**
		 * Check if this is actually a CD-ROM drive.
		 * @return True if this is a CD-ROM drive; false if not.
		 */
		bool isCdromDrive(void);

		/**
		 * Check if a disc is present.
		 * @return True if a disc is present; false if not.
		 */
		bool isDiscPresent(void);

		/**
		 * Get the current disc type.
		 * @return Disc type.
		 */
		CD_DiscType_t getDiscType(void);

		/**
		 * Get the current drive type.
		 * @return Drive type.
		 */
		CD_DriveType_t getDriveType(void);

		/**
		 * Get the disc label.
		 * @return Disc label.
		 */
		std::string getDiscLabel(void);

	public:
		/** Disc type queries. **/
		bool isAudioCD(void);	// Does the CD have audio tracks only?
		bool isDataCD(void);	// Does the CD have data tracks?
		bool isMixedCD(void);	// Does the CD have both audio and data tracks?
		bool isBlankCD(void);	// Is the CD blank?
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
