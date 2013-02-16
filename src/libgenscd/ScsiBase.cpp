/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * ScsiBase.cpp: SCSI device handler base class.                           *
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

#include "ScsiBase.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

// C++ includes.
#include <string>
using std::string;

// TODO: Byteorder headers from LibGens.
// Assuming LE host for now.
#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		(((x) & 0x0000FF00UL) << 8) | \
		(((x) & 0x00FF0000UL) >> 8))

#define be16_to_cpu(x)	__swab16(x)
#define be32_to_cpu(x)	__swab32(x)
#define le16_to_cpu(x)	(x)
#define le32_to_cpu(x)	(x)

#define cpu_to_be16(x)	__swab16(x)
#define cpu_to_be32(x)	__swab32(x)
#define cpu_to_le16(x)	(x)
#define cpu_to_le32(x)	(x)

// SCSI commands.
#include "scsi_protocol.h"

#define PRINT_SCSI_ERROR(op, err) \
	do { \
		fprintf(stderr, "SCSI error: OP=%02X, ERR=%02X, SK=%01X ASC=%02X\n", \
			op, err, SK(err), ASC(err)); \
	} while (0)

namespace LibGensCD
{

class ScsiBasePrivate
{
	public:
		ScsiBasePrivate(ScsiBase *q, string filename);

	private:
		ScsiBase *const q;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		ScsiBasePrivate(const ScsiBasePrivate &);
		ScsiBasePrivate &operator=(const ScsiBasePrivate &);

	public:
		// Device filename.
		std::string filename;
};

/** CdDrivePrivate **/

ScsiBasePrivate::ScsiBasePrivate(ScsiBase *q, string filename)
	: q(q)
	, filename(filename)
{ }

/** ScsiBase **/

ScsiBase::ScsiBase(const string& filename)
	: d(new ScsiBasePrivate(this, filename))
{ }

ScsiBase::~ScsiBase()
{
	/**
	 * NOTE: close() is a virtual function.
	 * We can't call it from the destructor.
	 * 
	 * Call close() in the subclass's destructor.
	 */

	delete d;
}

/**
 * Print the description of an error code returned by a ScsiBase function.
 * This may be positive for a SCSI sense key,
 * negative for an OS error, or 0 for no error.
 * (No message is printed for 0.)
 * @param op SCSI operation code.
 * @param err Error code, as returned by a ScsiBase function.
 * @param f File handle for fprintf(). (If nullptr, uses stderr.)
 */
void ScsiBase::printScsiError(uint8_t op, int err, FILE *f)
{
	if (!f)
		f = stderr;

	if (err == 0) {
		return;
	} else if (err < 0) {
		// OS-specific error.
		// This should've been handled by a subclass...
		fprintf(f, "OP=%02X, OS-specific error %d\n", op, err);
	} else /*if (err > 0)*/ {
		// SCSI sense key.
		// TODO: Convert values to text?
		fprintf(f, "OP=%02X, SK=%01X ASC=%02X ASCQ=%02X\n",
			op, SK(err), ASC(err), ASCQ(err));
	}
}

/** SCSI command wrappers. **/

/**
 * INQUIRY: Get device identification information.
 * @param resp	[out] Buffer for INQUIRY response.
 * @return 0 on success; SCSI SENSE KEY on error.
 */
int ScsiBase::inquiry(SCSI_RESP_INQUIRY_STD *resp)
{
	SCSI_CDB_INQUIRY cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Set SCSI operation type.
	cdb.OpCode = SCSI_OP_INQUIRY;
	cdb.AllocLen = (uint16_t)cpu_to_be16(sizeof(*resp));
	cdb.Control = 0;

	// Send the SCSI CDB.
	return scsi_send_cdb(&cdb, sizeof(cdb), resp, sizeof(*resp), ScsiBase::SCSI_DATA_IN);
}

/**
 * READ TOC: Read the CD-ROM Table of Contents.
 * @param toc		[out] Buffer for Table of Contents.
 * @param numTracks	[out, opt] Number of tracks.
 * @return 0 on success; SCSI SENSE KEY on error.
 */
int ScsiBase::readToc(SCSI_CDROM_TOC *toc, int *numTracks)
{
	SCSI_CDB_READ_TOC cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// TOC response includes a data length in addition to toc.
	SCSI_RESP_READ_TOC resp;
	memset(&resp, 0x00, sizeof(resp));

	// Attempt to read the TOC.
	cdb.OpCode = SCSI_OP_READ_TOC;
	cdb.MSF = SCSI_BIT_READ_TOC_MSF_LBA;	// LBA addressing only.
	cdb.Format = 0; // Standard TOC for CD-ROMs.
	cdb.TrackSessionNumber = 0;
	cdb.AllocLen = (uint16_t)cpu_to_be16(sizeof(resp));
	cdb.Control = 0;

	int err = scsi_send_cdb(&cdb, sizeof(cdb), &resp, sizeof(resp), ScsiBase::SCSI_DATA_IN);
	if (err != 0) {
		// An error occurred.
		return err;
	}

	// Byteswap the Table of Contents data length.
	resp.DataLen = be16_to_cpu(resp.DataLen);

	// Copy the track numbers.
	toc->FirstTrackNumber = resp.toc.FirstTrackNumber;
	toc->LastTrackNumber = resp.toc.LastTrackNumber;

	// Determine how many tracks we have.
	int count;
	if (resp.toc.FirstTrackNumber == 0 && resp.toc.LastTrackNumber == 0) {
		// No tracks.
		// TODO: What if a disc has only one track with number 0?
		count = 0;
	} else {
		// Calculate the number of tracks.
		count = (resp.toc.LastTrackNumber - resp.toc.FirstTrackNumber + 1);
		if (count < 0)
			count = 0;
		else if (count > (int)(sizeof(resp.toc.Tracks)/sizeof(resp.toc.Tracks[0])))
			count = (int)(sizeof(resp.toc.Tracks)/sizeof(resp.toc.Tracks[0]));

		// Make sure num_tracks doesn't exceed the data length.
		const int num_tracks_data = ((resp.DataLen - 2) / 4);
		if (count > num_tracks_data)
			count = num_tracks_data;

		// Copy and byteswap the TOC.
		for (int track = 0; track < count; track++) {
			toc->Tracks[track].rsvd1 = resp.toc.Tracks[track].rsvd1;
			toc->Tracks[track].ControlADR = resp.toc.Tracks[track].ControlADR;
			toc->Tracks[track].TrackNumber = resp.toc.Tracks[track].TrackNumber;
			toc->Tracks[track].rsvd2 = resp.toc.Tracks[track].rsvd2;

			toc->Tracks[track].StartAddress =
				be32_to_cpu(resp.toc.Tracks[track].StartAddress);
		}
	}

	// Save the number of tracks.
	if (numTracks)
		*numTracks = count;

	// TOC has been read.
	return 0;
}

/**
 * GET CONFIGURATION: Get the MMC configuration.
 * This function only returns the header.
 * @param resp	[out] Buffer for GET CONFIGURATION response header.
 * @return 0 on success; SCSI SENSE KEY on error.
 */
int ScsiBase::getConfiguration(SCSI_RESP_GET_CONFIGURATION_HEADER *resp)
{
	SCSI_CDB_GET_CONFIGURATION cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Get the current configuration.
	cdb.OpCode = SCSI_OP_GET_CONFIGURATION;
	cdb.AllocLen = cpu_to_be16(sizeof(*resp));
	cdb.Control = 0;

	int err = scsi_send_cdb(&cdb, sizeof(cdb), resp, sizeof(*resp), ScsiBase::SCSI_DATA_IN);
	if (err != 0) {
		// An error occurred.
		return err;
	}

	// Byteswap the fields.
	resp->DataLength = be32_to_cpu(resp->DataLength);
	resp->CurrentProfile = (uint16_t)be16_to_cpu(resp->CurrentProfile);

	// Configuration retrieved.
	return 0;
}

/**
 * READ DISC INFORMATION: Read the disc information.
 * This function returns STANDARD disc information, without OPC data.
 * @param resp	[out] Buffer for READ DISC INFORMATION response.
 * @return 0 on success; SCSI SENSE KEY on error.
 */
int ScsiBase::readDiscInformation(SCSI_RESP_READ_DISC_INFORMATION_STANDARD *resp)
{
	SCSI_CDB_READ_DISC_INFORMATION cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Get the disc information.
	cdb.OpCode = SCSI_OP_READ_DISC_INFORMATION;
	cdb.AllocLen = cpu_to_be16(sizeof(*resp));
	cdb.Control = 0;

	return scsi_send_cdb(&cdb, sizeof(cdb), resp, sizeof(*resp), ScsiBase::SCSI_DATA_IN);
}

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
int ScsiBase::readCD(uint32_t lba, uint32_t block_count,
		        void *out, size_t out_len,
			uint8_t sector_type,
			ReadCDRawSectorMode raw_mode)
{
	if (block_count > 0xFFFFFF) {
		// READ CD's block count is 24-bit, but the
		// requested block count exceeds 24 bits.
		// TODO: Determine the proper KCQ.
		// We're using Illegal Request - invalid field parameter - invalid parm value.
		return 0x52602;
	}

	SCSI_CDB_READ_CD cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Read a block from the CD.
	cdb.OpCode = SCSI_OP_READ_CD;
	cdb.SectorType = sector_type;
	cdb.StartingLBA = cpu_to_be32(lba);
	cdb.TransferLen[0] = ((block_count >> 16) & 0xFF);
	cdb.TransferLen[1] = ((block_count >> 8)  & 0xFF);
	cdb.TransferLen[2] = ((block_count >> 0)  & 0xFF);

	// Should we read the full sector?
	if (raw_mode & RCDRAW_FULL) {
		// Full 2352-byte sector.
		cdb.Flags =
			SCSI_BIT_READ_CD_FLAGS_EDC_ECC |
			SCSI_BIT_READ_CD_FLAGS_USER_DATA |
			SCSI_BIT_READ_CD_FLAGS_HEADER |
			SCSI_BIT_READ_CD_FLAGS_SUBHEADER |
			SCSI_BIT_READ_CD_FLAGS_SYNCH_FIELD;
	} else {
		// User data only.
		cdb.Flags = SCSI_BIT_READ_CD_FLAGS_USER_DATA;
	}

	// Should we transfer subchannels?
	if (raw_mode & RCDRAW_SUB_RW) {
		// All subchannels.
		cdb.Subchannels = SCSI_READ_CD_SUBCHANNELS_R_W;
	} else if (raw_mode & RCDRAW_SUB_PQ) {
		// P-Q subchannels.
		cdb.Subchannels = SCSI_READ_CD_SUBCHANNELS_Q;
	} else {
		// No subchannels.
		cdb.Subchannels = 0;
	}

	// Send the CDB.
	return scsi_send_cdb(&cdb, sizeof(cdb), out, out_len, ScsiBase::SCSI_DATA_IN);
}

}
