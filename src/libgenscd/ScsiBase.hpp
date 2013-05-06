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
#include <cstdio>

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
		 * Print the description of an error code returned by a ScsiBase function.
		 * This may be positive for a SCSI sense key,
		 * negative for an OS error, or 0 for no error.
		 * (No message is printed for 0.)
		 * @param op SCSI operation code.
		 * @param err Error code, as returned by a ScsiBase function.
		 * @param f File handle for fprintf(). (If nullptr, uses stderr.)
		 */
		virtual void printScsiError(uint8_t op, int err, FILE *f = nullptr);

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
		 * Send a SCSI command descriptor block to the device.
		 * @param cdb		[in] SCSI command descriptor block.
		 * @param cdb_len	[in] Length of cdb.
		 * @param data		[in/out] Data buffer, or nullptr for SCSI_DATA_NONE operations.
		 * @param data_len	[in] Length of data.
		 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
		 * @return 0 on success, positive for SCSI sense key, negative for OS error.
		 */
		virtual int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
					  void *data, size_t data_len,
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
		 * READ: Read one or more sectors from the device.
		 * This implementation uses READ(10), which can read
		 * up to a maximum of 65,535 blocks at one time.
		 * @param lba		[in] Logical Block Address.
		 * @param block_count	[in] Number of blocks to read.
		 * @param out		[out] Buffer for output data.
		 * @param out_len	[in] Length of out.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int read(uint32_t lba, uint16_t block_count,
				void *out, size_t out_len);

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

		enum ReadCDRawSectorMode
		{
			RCDRAW_USER		= 0x00,	// User data only.
			RCDRAW_FULL		= 0x01,	// Full 2352-byte sector.
			RCDRAW_SUB_PQ		= 0x02,	// User data + P-Q subchannels.
			RCDRAW_FULL_SUB_PQ	= 0x03,	// Full sector + P-Q subchannels.
			RCDRAW_SUB_RW		= 0x04,	// User data + all subchannels.
			RCDRAW_FULL_SUB_RW	= 0x05,	// Full sector + all subchannels.
			RCDRAW_SUB_PQ_RW	= 0x06,	// User data + all subchannels.
			RCDRAW_FULL_SUB_PQ_RW	= 0x07,	// Full sector + all subchannels.
		};

		/**
		 * READ CD: Read a CD-ROM sector.
		 * This function does not verify the sector type.
		 * @param lba		[in] Logical Block Address.
		 * @param block_count	[in] Number of blocks to read.
		 * @param out		[out] Buffer for output data.
		 * @param out_len	[in] Length of out.
		 * @param sector_type	[in] Expected sector type. (SCSI_READ_CD_SECTORTYPE_*)
		 * @param raw_mode	[in] Raw sector mode.
		 * @return 0 on success; SCSI SENSE KEY on error.
		 */
		int readCD(uint32_t lba, uint32_t block_count,
			   void *out, size_t out_len,
			   uint8_t sector_type,
			   ReadCDRawSectorMode raw_mode);
};

}

#endif /* __LIBGENSCD_SCSIBASE_HPP__ */
