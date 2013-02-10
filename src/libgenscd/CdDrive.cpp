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

CdDrive::CdDrive(const string& filename)
	: m_filename(filename)
{
	// Nothing to do here...
	// We can't inquiry the drive yet, since drive initialization
	// is performed by the subclass.

	// Clear the inquiry data.
	// TODO: Make the inquiry data a class, and put this in the constructor?
	m_inq_data.inq_status = INQ_NOT_DONE;
	m_inq_data.device_type = 0;
}

CdDrive::~CdDrive()
{ }

/**
 * Run a SCSI INQUIRY command.
 * @return 0 on success; nonzero on error.
 */
int CdDrive::inquiry(void)
{
	CDB_SCSI cdb;
	SCSI_INQUIRY_STD_DATA data;

	memset(&cdb, 0x00, sizeof(cdb));
	memset(&data, 0x00, sizeof(data));

	// Inquiry hasn't been done yet.
	m_inq_data.inq_status = INQ_NOT_DONE;

	// Set SCSI operation type.
	cdb.OperationCode = SCSI_INQUIRY;
	cdb.AllocationLength = sizeof(data);

	// Send the SCSI CDB.
	int err = scsi_send_cdb(&cdb, sizeof(cdb), &data, sizeof(data), SCSI_DATA_IN);
	if (err != 0) {
		// Inquiry failed.
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		m_inq_data.inq_status = INQ_FAILED;
		return -1;
	}

	// Drive inquiry successful.
	// Get the information.
	// TODO: Validate that device type is 0x05. (DTYPE_CDROM)
	// TODO: Trim spaces in vendor, model, and firmware.
	m_inq_data.device_type = data.peripheral_device_type;
	m_inq_data.vendor = string(data.vendor_id, sizeof(data.vendor_id));
	m_inq_data.model = string(data.product_id, sizeof(data.product_id));
	m_inq_data.firmware = string(data.product_revision_level, sizeof(data.product_revision_level));

	// SCSI_INQUIRY completed successfully.
	m_inq_data.inq_status = INQ_SUCCESSFUL;
	return 0;
}

/**
 * Check if the inquiry was successful.
 * @return True if successful; false if not.
 */
bool CdDrive::isInquirySuccessful(void)
{
	if (m_inq_data.inq_status == INQ_NOT_DONE)
		inquiry();
	return (m_inq_data.inq_status == INQ_SUCCESSFUL);
}

std::string CdDrive::dev_vendor(void)
{
	if (m_inq_data.inq_status == INQ_NOT_DONE)
		inquiry();
	if (m_inq_data.inq_status != INQ_SUCCESSFUL)
		return "";

	return m_inq_data.vendor;
}

std::string CdDrive::dev_model(void)
{
	if (m_inq_data.inq_status == INQ_NOT_DONE)
		inquiry();
	if (m_inq_data.inq_status != INQ_SUCCESSFUL)
		return "";

	return m_inq_data.model;
}

std::string CdDrive::dev_firmware(void)
{
	if (m_inq_data.inq_status == INQ_NOT_DONE)
		inquiry();
	if (m_inq_data.inq_status != INQ_SUCCESSFUL)
		return "";

	return m_inq_data.firmware;
}

/**
 * Get the current feature profile, aka disc type.
 * Uses the MMC-2 GET CONFIGURATION command.
 * If the command isn't supported, falls back to MMC-1.
 * @return Current feature profile, or 0xFFFF on error.
 */
uint16_t CdDrive::getCurrentFeatureProfile(void)
{
	CDB_MMC_GET_CONFIGURAITON cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Feature header.
	SCSI_MMC_GET_CONFIGURATION_HEADER_DATA features;
	memset(&features, 0x00, sizeof(features));

	// Query the current profile.
	cdb.OperationCode = MMC_GET_CONFIGURATION;
	cdb.AllocationLength = sizeof(features);
	cdb.Control = 0;

	int err = scsi_send_cdb(&cdb, sizeof(cdb), &features, sizeof(features), SCSI_DATA_IN);
	if (err != 0) {
		// Error occurred.
		if (SK(err) == 0x5 && ASC(err) == 0x20) {
			// Drive does not support MMC-2 commands.
			// Try the MMC-1 fallback.
			return getCurrentFeatureProfile_mmc1();
		}

		// Other error.
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
uint16_t CdDrive::getCurrentFeatureProfile_mmc1(void)
{
	// TODO: Check if a disc is present first.

	CDB_MMC_READ_DISC_INFORMATION cdb;
	memset(&cdb, 0x00, sizeof(cdb));

	// Disc information data.
	SCSI_MMC_READ_DISC_INFORMATION_DATA discInfo;
	memset(&discInfo, 0x00, sizeof(discInfo));

	// Query the current profile.
	cdb.OperationCode = MMC_GET_CONFIGURATION;
	cdb.AllocationLength = sizeof(discInfo);
	cdb.Control = 0;

	int err = scsi_send_cdb(&cdb, sizeof(cdb), &discInfo, sizeof(discInfo), SCSI_DATA_IN);
	if (err != 0) {
		// Error occurred.
		PRINT_SCSI_ERROR(cdb.OperationCode, err);
		return 0;
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
 * Get the current disc type.
 * @return Disc type.
 */
CD_DiscType_t CdDrive::getDiscType(void)
{
	// TODO: Cache the current profile?
	const uint16_t cur_profile = getCurrentFeatureProfile();

	switch (cur_profile) {
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

}
