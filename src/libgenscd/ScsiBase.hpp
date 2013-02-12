/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * ScsiBase.hpp: SCSI device handler base class.                           *
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

#ifndef __LIBGENSCD_SCSIBASE_HPP__
#define __LIBGENSCD_SCSIBASE_HPP__

// C includes.
#include <stdint.h>
#include <cstddef>

// C++ includes.
#include <string>

// SCSI commands.
#include "scsi_protocol.h"

namespace LibGensCD
{

class ScsiBasePrivate;

class ScsiBase
{
	public:
		ScsiBase(const std::string& filename);
		virtual ~ScsiBase();

	private:
		friend class ScsiBasePrivate;
		ScsiBasePrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		ScsiBase(const ScsiBase &);
		ScsiBase &operator=(const ScsiBase &);

	public:
		virtual bool isOpen(void) const = 0;
		virtual void close(void) = 0;

		/**
		 * Check if a disc is present.
		 * @return True if a disc is present; false if not.
		 */
		virtual bool isDiscPresent(void) = 0;

		/**
		 * Check if the disc has changed since the last access.
		 * @return True if the disc has changed; false if not.
		 */
		virtual bool hasDiscChanged(void) = 0;

		enum scsi_data_mode
		{
			SCSI_DATA_NONE,
			SCSI_DATA_IN,
			SCSI_DATA_OUT
		};

		/**
		 * Send a SCSI command descriptor block to the drive.
		 * @param cdb		[in] SCSI command descriptor block.
		 * @param cdb_len	[in] Length of cdb.
		 * @param out		[out] Output buffer, or nullptr if no data is requested.
		 * @param out_len	[out] Length of out.
		 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
		 * @return 0 on success, non-zero on error. (TODO: Return SCSI sense key?)
		 */
		virtual int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
					  void *out, size_t out_len,
					  scsi_data_mode mode = SCSI_DATA_IN) = 0;

	public:
		/** SCSI command wrappers. **/

		/**
		 * INQUIRY: Get device identification information.
		 * @param resp	[out] Buffer for INQUIRY response.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int inquiry(SCSI_RESP_INQUIRY_STD *resp);

		/**
		 * READ TOC: Read the CD-ROM Table of Contents.
		 * @param toc		[out] Buffer for Table of Contents.
		 * @param numTracks	[out, opt] Number of tracks.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int readToc(SCSI_CDROM_TOC *toc, int *numTracks = nullptr);

		/**
		 * GET CONFIGURATION: Get the MMC configuration.
		 * This function only returns the header.
		 * @param resp	[out] Buffer for GET CONFIGURATION response header.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int getConfiguration(SCSI_RESP_GET_CONFIGURATION_HEADER *resp);

		/**
		 * READ DISC INFORMATION: Read the disc information.
		 * This function returns STANDARD disc information, without OPC data.
		 * @param resp	[out] Buffer for READ DISC INFORMATION response.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int readDiscInformation(SCSI_RESP_READ_DISC_INFORMATION_STANDARD *resp);
};

}

#endif /* __LIBGENSCD_SCSIBASE_HPP__ */
