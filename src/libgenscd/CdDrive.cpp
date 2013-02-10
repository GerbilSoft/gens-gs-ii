#include "CdDrive.hpp"

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
#include "genscd_scsi.h"

#define PRINT_SCSI_ERROR(op, err) \
	do { \
		fprintf(stderr, "SCSI error: OP=%02X, ERR=%02X, SK=%01X ASC=%02X\n", \
			op, err, SK(err), ASC(err)); \
	} while (0)

namespace LibGensCD
{

class CdDrivePrivate
{
	public:
		CdDrivePrivate(CdDrive *q, string filename);

	private:
		CdDrive *const q;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		CdDrivePrivate(const CdDrivePrivate &);
		CdDrivePrivate &operator=(const CdDrivePrivate &);

	public:
		// Device filename.
		std::string filename;

		// TODO: Cache TOC, etc.
		// Update cache if the media status changes.

		enum InquiryStatus
		{
			INQ_NOT_DONE,
			INQ_SUCCESSFUL,
			INQ_FAILED
		};

		// INQUIRY results.
		struct {
			// Inquiry status.
			InquiryStatus inq_status;

			// Peripheral device type.
			// Should be 0x05. (DTYPE_CDROM)
			uint8_t device_type;

			std::string vendor;
			std::string model;
			std::string firmware;
		} inq_data;

		int inquiry(void);
		bool isInquirySuccessful(void);

		/**
		 * Get the current feature profile, aka disc type.
		 * Uses the MMC-2 GET CONFIGURATION command.
		 * If the command isn't supported, falls back to MMC-1.
		 * @return Current feature profile, or 0xFFFF on error.
		 */
		uint16_t getCurrentFeatureProfile(void);

		/**
		 * Get the current feature profile, aka disc type.
		 * Use the MMC-1 READ DISC INFORMATION command.
		 * @return Current feature profile, or 0xFFFF on error.
		 */
		uint16_t getCurrentFeatureProfile_mmc1(void);

		/**
		 * Get a bitfield of supported disc types.
		 * @return Bitfield of supported disc types.
		 */
		uint32_t getSupportedDiscTypes(void);

		/**
		 * Convert an MMC feature profile to a CD_DiscType_t.
		 * @param featureProfile MMC feature profile.
		 * @return CD_DiscType_t.
		 */
		static CD_DiscType_t mmcFeatureProfileToDiscType(uint16_t featureProfile);

		/**
		 * Convert a bitfield of CD_DiscType_t to a CD_DriveType_t.
		 * @param discTypes Bitfield of all CD_DiscType_t disc types.
		 * @return CD_DriveType_t.
		 */
		static CD_DriveType_t discTypesToDriveType(uint32_t discTypes);

		// Table of Contents.
		struct {
			int num_tracks;
			SCSI_CDROM_TOC toc;
		} toc;

		/**
		 * Read the Table of Contents into the internal buffer.
		 * @return 0 on success; non-zero on error.
		 */
		int readToc(void);
};

/** CdDrivePrivate **/

CdDrivePrivate::CdDrivePrivate(CdDrive *q, string filename)
	: q(q)
	, filename(filename)
{
	// Clear the inquiry data.
	// TODO: Make the inquiry data a class, and put this in the constructor?
	inq_data.inq_status = INQ_NOT_DONE;
	inq_data.device_type = 0;

	// Clear the TOC.
	memset(&toc, 0x00, sizeof(toc));
}

/**
 * Run a SCSI INQUIRY command.
 * @return 0 on success; nonzero on error.
 */
int CdDrivePrivate::inquiry(void)
{
	CDB_SCSI cdb;
	SCSI_INQUIRY_STD_DATA data;

	memset(&cdb, 0x00, sizeof(cdb));
	memset(&data, 0x00, sizeof(data));

	// Inquiry hasn't been done yet.
	inq_data.inq_status = INQ_NOT_DONE;

	// Set SCSI operation type.
	cdb.OperationCode = SCSI_INQUIRY;
	cdb.AllocationLength = sizeof(data);

	// Send the SCSI CDB.
	int err = q->scsi_send_cdb(
		&cdb, sizeof(cdb),
		&data, sizeof(data),
		CdDrive::SCSI_DATA_IN);
	if (err != 0) {
		// Inquiry failed.
		// TODO: We have to request the sense data. err isn't sense data...
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		inq_data.inq_status = CdDrivePrivate::INQ_FAILED;
		return -1;
	}

	// Drive inquiry successful.
	// Get the information.
	// TODO: Validate that device type is 0x05. (DTYPE_CDROM)
	// TODO: Trim spaces in vendor, model, and firmware.
	inq_data.device_type = data.peripheral_device_type;
	inq_data.vendor = string(data.vendor_id, sizeof(data.vendor_id));
	inq_data.model = string(data.product_id, sizeof(data.product_id));
	inq_data.firmware = string(data.product_revision_level, sizeof(data.product_revision_level));

	// SCSI_INQUIRY completed successfully.
	inq_data.inq_status = CdDrivePrivate::INQ_SUCCESSFUL;

	return 0;
}

/**
 * Check if the inquiry was successful.
 * @return True if successful; false if not.
 */
bool CdDrivePrivate::isInquirySuccessful(void)
{
	if (inq_data.inq_status == INQ_NOT_DONE)
		inquiry();
	return (inq_data.inq_status == INQ_SUCCESSFUL);
}

/**
 * Get the current feature profile, aka disc type.
 * Uses the MMC-2 GET CONFIGURATION command.
 * If the command isn't supported, falls back to MMC-1.
 * @return Current feature profile, or 0xFFFF on error.
 */
uint16_t CdDrivePrivate::getCurrentFeatureProfile(void)
{
	// Make sure a disc is present.
	if (!q->isOpen() || !q->isDiscPresent())
		return 0;

	CDB_MMC_GET_CONFIGURAITON cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Feature header.
	SCSI_MMC_GET_CONFIGURATION_HEADER_DATA features;
	memset(&features, 0x00, sizeof(features));

	// Query the current profile.
	cdb.OperationCode = MMC_GET_CONFIGURATION;
	cdb.AllocationLength = cpu_to_be16(sizeof(features));
	cdb.Control = 0;

	int err = q->scsi_send_cdb(
		&cdb, sizeof(cdb),
		&features, sizeof(features),
		CdDrive::SCSI_DATA_IN);
	if (err != 0) {
		// Error occurred.
		// TODO: We have to request the sense data. err isn't sense data...
		// Let's try the MMC-1 version regardless.
		/*if (SK(err) == 0x5 && ASC(err) == 0x20)*/ {
			// Drive does not support MMC-2 commands.
			// Try the MMC-1 fallback.
			return getCurrentFeatureProfile_mmc1();
		}

		// Other error.
		// TODO: We have to request the sense data. err isn't sense data...
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		return 0;
	}

	// Get the current profile.
	uint16_t cur_profile = be16_to_cpu(features.CurrentProfile);
	return cur_profile;
}

/**
 * Get the current feature profile, aka disc type.
 * Uses the MMC-1 READ DISC INFORMATION command.
 * @return Current feature profile, or 0xFFFF on error.
 */
uint16_t CdDrivePrivate::getCurrentFeatureProfile_mmc1(void)
{
	// Make sure a disc is present.
	if (!q->isOpen() || !q->isDiscPresent())
		return 0;

	CDB_MMC_READ_DISC_INFORMATION cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Disc information data.
	SCSI_MMC_READ_DISC_INFORMATION_DATA discInfo;
	memset(&discInfo, 0x00, sizeof(discInfo));

	// Get the disc information.
	cdb.OperationCode = MMC_READ_DISC_INFORMATION;
	cdb.AllocationLength = cpu_to_be16(sizeof(discInfo));
	cdb.Control = 0;

	int err = q->scsi_send_cdb(
		&cdb, sizeof(cdb),
		&discInfo, sizeof(discInfo),
		CdDrive::SCSI_DATA_IN);
	if (err != 0) {
		// An error occurred requesting READ DISC INFORMATION.
		// This usually means that we have a CD-ROM.
		return 0x08;	// MMC CD-ROM profile.
	}

	// Determine the current profile based on the disc information.
	// MMC-1 devices don't support DVDs, so we can rule out
	// everything except CD-ROM, CD-R, and CD-RW.
	// (We're not counting MO here.)
	if (discInfo.DiscStatusFlags & 0x10) {
		// Disc is rewritable.
		return 0x0A;	// MMC CD-RW profile.
	} else if ((discInfo.DiscStatusFlags & 0x03) < 2) {
		// Disc is either empty, incomplete, or finalized.
		// This is a CD-R.
		return 0x09;	// MMC CD-R profile.
	}

	// Assume this is a CD-ROM.
	return 0x08;	// MMC CD-ROM profile.
}

/**
 * Get a bitfield of supported disc types.
 * @return Bitfield of supported disc types.
 */
uint32_t CdDrivePrivate::getSupportedDiscTypes(void)
{
	// Make sure the device file is open.
	if (!q->isOpen())
		return 0;

	CDB_MMC_GET_CONFIGURAITON cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Feature buffer.
	uint8_t features[65530];
	memset(features, 0x00, sizeof(features));

	// Query all available profiles.
	cdb.OperationCode = MMC_GET_CONFIGURATION;
	cdb.AllocationLength = (uint16_t)cpu_to_be16(sizeof(features));
	cdb.Control = 0;

	int err = q->scsi_send_cdb(
		&cdb, sizeof(cdb),
		&features, sizeof(features),
		CdDrive::SCSI_DATA_IN);
	if (err != 0) {
		// Error occurred.
		// TODO: Check the sense data.
		/*
		if (SK(err) == 0x5 && ASC(err) == 0x20) {
			// Drive does not support MMC-2 commands.
			// Determine drive type based on current disc type.
			switch (getDiscType()) {
				case DISC_TYPE_CD_RW:
					return DRIVE_TYPE_CD_RW;
				case DISC_TYPE_CD_R:
					return DRIVE_TYPE_CD_R;
				case DISC_TYPE_CDROM:
				default:
					return DRIVE_TYPE_CDROM;
			}
		}
		*/

		// Other error.
		// TODO: We have to request the sense data. err isn't sense data...
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		return 0;
	}

	// Check how many features we received.
	uint32_t len = (features[0] << 24 | features[1] << 16 | features[2] << 8 | features[3]);
	if (len > sizeof(features)) {
		// Too many features. Truncate the list
		len = sizeof(features);
	}

	// Go through all of the features.
	int discTypes = 0;
	for (unsigned int i = 8; i+4 < len; i += (4 + features[i+3])) {
		const unsigned int feature = (features[i] << 8 | features[i+1]);
		if (feature == 0x00) {
			// Feature profiles.
			const uint8_t *ptr = &features[i+4];
			const uint8_t *const ptr_end = ptr + features[i+3];

			for (; ptr < ptr_end; ptr += 4) {
				const uint16_t featureProfile = (ptr[0] << 8 | ptr[1]);
				printf("Feature Profile: %04X\n", featureProfile);
				discTypes |= mmcFeatureProfileToDiscType(featureProfile);
			}
		}
	}

	// Return the bitfield of supported disc types.
	return discTypes;
}

/**
 * Convert an MMC feature profile to a CD_DiscType_t.
 * @param featureProfile MMC feature profile.
 * @return CD_DiscType_t.
 */
CD_DiscType_t CdDrivePrivate::mmcFeatureProfileToDiscType(uint16_t featureProfile)
{
	switch (featureProfile) {
		case 0x03:	return DISC_TYPE_MO;		// (legacy) MO erasable
		case 0x04:	return DISC_TYPE_MO;		// (legacy) Optical Write-Once
		case 0x05:	return DISC_TYPE_MO;		// (legacy) AS-MO
		case 0x08:	return DISC_TYPE_CDROM;
		case 0x09:	return DISC_TYPE_CD_R;
		case 0x0A:	return DISC_TYPE_CD_RW;
		case 0x10:	return DISC_TYPE_DVD;
		case 0x11:	return DISC_TYPE_DVD_R;
		case 0x12:	return DISC_TYPE_DVD_RAM;
		case 0x13:	return DISC_TYPE_DVD_RW; 	// read-only
		case 0x14:	return DISC_TYPE_DVD_RW; 	// sequential
		case 0x15:	return DISC_TYPE_DVD_R_DL;	// sequential
		case 0x16:	return DISC_TYPE_DVD_R_DL;	// layer jump
		case 0x17:	return DISC_TYPE_DVD_RW_DL;
		case 0x1A:	return DISC_TYPE_DVD_PLUS_RW;
		case 0x1B:	return DISC_TYPE_DVD_PLUS_R;
		case 0x2A:	return DISC_TYPE_DVD_PLUS_RW_DL;
		case 0x2B:	return DISC_TYPE_DVD_PLUS_R_DL;
		case 0x40:	return DISC_TYPE_BDROM;
		case 0x41:	return DISC_TYPE_BD_R;		// sequential
		case 0x42:	return DISC_TYPE_BD_R;		// random
		case 0x43:	return DISC_TYPE_BD_RE;
		case 0x50:	return DISC_TYPE_HDDVD;
		case 0x51:	return DISC_TYPE_HDDVD_R;
		case 0x52:	return DISC_TYPE_HDDVD_RAM;
		case 0x53:	return DISC_TYPE_HDDVD_RW;
		case 0x58:	return DISC_TYPE_HDDVD_R_DL;
		case 0x5A:	return DISC_TYPE_HDDVD_RW_DL;

		case 0x00:
		default:
			return DISC_TYPE_NONE;
	}
}

/**
 * Convert a bitfield of CD_DiscType_t to a CD_DriveType_t.
 * @param discTypes Bitfield of all CD_DiscType_t disc types.
 * @return CD_DriveType_t.
 */
CD_DriveType_t CdDrivePrivate::discTypesToDriveType(uint32_t discTypes)
{
	// TODO: Find various permutations like DVD/CD-RW.
	// Also, check for multi-format DVD±RW drives.
	// For now, just get the maximum disc type.
	if (discTypes & DISC_TYPE_MO)
		return DRIVE_TYPE_MO;
	else if (discTypes & DISC_TYPE_HDDVD_RW)
		return DRIVE_TYPE_HDDVD_RW;
	else if (discTypes & DISC_TYPE_HDDVD_R)
		return DRIVE_TYPE_HDDVD_R;
	else if (discTypes & DISC_TYPE_HDDVD) {
		// TODO: Check for CD/DVD writing capabilities.
		return DRIVE_TYPE_HDDVD;
	} else if (discTypes & DISC_TYPE_BD_RE)
		return DRIVE_TYPE_BD_RE;
	else if (discTypes & DISC_TYPE_BD_R)
		return DRIVE_TYPE_BD_R;
	else if (discTypes & DISC_TYPE_BDROM) {
		// TODO: Check for CD/DVD writing capabilities.
		return DRIVE_TYPE_BDROM;
	} else if (discTypes & DISC_TYPE_DVD_PLUS_RW_DL)
		return DRIVE_TYPE_DVD_PLUS_RW_DL;
	else if (discTypes & DISC_TYPE_DVD_PLUS_R_DL)
		return DRIVE_TYPE_DVD_PLUS_R_DL;
	else if (discTypes & (DISC_TYPE_DVD_PLUS_RW | DISC_TYPE_DVD_PLUS_R)) {
		// DVD+RW was released before DVD+R.
		// Hence, there's no such thing as a DVD+R-only drive.
		return DRIVE_TYPE_DVD_PLUS_RW;
	} else if (discTypes & DISC_TYPE_DVD_RAM)
		return DRIVE_TYPE_DVD_RAM;
	else if (discTypes & DISC_TYPE_DVD_RW)
		return DRIVE_TYPE_DVD_RW;
	else if (discTypes & DISC_TYPE_DVD_R)
		return DRIVE_TYPE_DVD_R;
	else if (discTypes & DISC_TYPE_DVD) {
		if (discTypes & (DISC_TYPE_CD_R | DISC_TYPE_CD_RW))
			return DRIVE_TYPE_DVD_CD_RW;
		else
			return DRIVE_TYPE_DVD;
	} else if (discTypes & DISC_TYPE_CD_RW)
		return DRIVE_TYPE_CD_RW;
	else if (discTypes & DISC_TYPE_CD_R)
		return DRIVE_TYPE_CD_R;
	else if (discTypes & DISC_TYPE_CDROM)
		return DRIVE_TYPE_CDROM;
	else
		return DRIVE_TYPE_NONE;
}

/**
 * Read the Table of Contents into the internal buffer.
 * @return 0 on success; non-zero on error.
 */
int CdDrivePrivate::readToc(void)
{
	// TODO: Check if the medium has been changed.
	if (!q->isDiscPresent()) {
		// No disc present.
		toc.num_tracks = 0;
		toc.toc.DataLen = 0;
		toc.toc.FirstTrackNumber = 0;
		toc.toc.LastTrackNumber = 0;
		return 0;
	}

	CDB_SCSI_READ_TOC cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Attempt to read the TOC.
	cdb.OperationCode = SCSI_READ_TOC;
	cdb.MSF = SCSI_READ_TOC_FORMAT_LBA;
	cdb.Format = 0; // Standard TOC for CD-ROMs.
	cdb.TrackSessionNumber = 0;
	cdb.AllocationLength = (uint16_t)cpu_to_be16(sizeof(toc.toc));

	int err = q->scsi_send_cdb(
		&cdb, sizeof(cdb),
		&toc.toc, sizeof(toc.toc),
		CdDrive::SCSI_DATA_IN);
	if (err != 0) {
		// Error occurred.
		// TODO: We have to request the sense data. err isn't sense data...
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		toc.num_tracks = 0;
		toc.toc.DataLen = 0;
		toc.toc.FirstTrackNumber = 0;
		toc.toc.LastTrackNumber = 0;
		return -1;
	}

	// Calculate the number of tracks.
	toc.num_tracks = ((toc.toc.LastTrackNumber - toc.toc.FirstTrackNumber) + 1);
	if (toc.num_tracks < 0)
		toc.num_tracks = 0;
	else if (toc.num_tracks > (int)(sizeof(toc.toc.Tracks) / sizeof(toc.toc.Tracks[0])))
		toc.num_tracks = (int)(sizeof(toc.toc.Tracks) / sizeof(toc.toc.Tracks[0]));

	// Make sure num_tracks doesn't exceed the data length.
	const int num_tracks_data = ((toc.toc.DataLen - 2) / 4);
	if (toc.num_tracks > num_tracks_data)
		toc.num_tracks = num_tracks_data;

	// Byteswap the TOC.
	for (int track = 0; track < toc.num_tracks; track++) {
		toc.toc.Tracks[track].StartAddress =
			be32_to_cpu(toc.toc.Tracks[track].StartAddress);
	}

	// TOC has been read.
	return 0;
}

/** CdDrive **/

CdDrive::CdDrive(const string& filename)
	: d(new CdDrivePrivate(this, filename))
{
	// Nothing to do here...
	// We can't inquiry the drive yet, since drive initialization
	// is performed by the subclass.
}

CdDrive::~CdDrive()
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
 * Run a SCSI INQUIRY command.
 * WRAPPER FUNCTION for CdDrivePrivate::inquiry().
 * @return 0 on success; nonzero on error.
 */
int CdDrive::inquiry(void)
{
	return d->inquiry();
}

/**
 * Check if the inquiry was successful.
 * WRAPPER FUNCTION for CdDrivePrivate::isInquirySuccessful();
 * @return True if successful; false if not.
 */
bool CdDrive::isInquirySuccessful(void)
{
	return d->isInquirySuccessful();
}

std::string CdDrive::dev_vendor(void)
{
	if (d->inq_data.inq_status == CdDrivePrivate::INQ_NOT_DONE)
		inquiry();
	if (d->inq_data.inq_status != CdDrivePrivate::INQ_SUCCESSFUL)
		return "";

	return d->inq_data.vendor;
}

std::string CdDrive::dev_model(void)
{
	if (d->inq_data.inq_status == CdDrivePrivate::INQ_NOT_DONE)
		inquiry();
	if (d->inq_data.inq_status != CdDrivePrivate::INQ_SUCCESSFUL)
		return "";

	return d->inq_data.model;
}

std::string CdDrive::dev_firmware(void)
{
	if (d->inq_data.inq_status == CdDrivePrivate::INQ_NOT_DONE)
		inquiry();
	if (d->inq_data.inq_status != CdDrivePrivate::INQ_SUCCESSFUL)
		return "";

	return d->inq_data.firmware;
}

/**
 * Get the current disc type.
 * @return Disc type.
 */
CD_DiscType_t CdDrive::getDiscType(void)
{
	// TODO: Cache the current profile?
	return d->mmcFeatureProfileToDiscType(d->getCurrentFeatureProfile());
}

/**
 * Get the current drive type.
 * @return Drive type.
 */
CD_DriveType_t CdDrive::getDriveType(void)
{
	// TODO: Cache the current drive type?
	return d->discTypesToDriveType(d->getSupportedDiscTypes());
}

}
