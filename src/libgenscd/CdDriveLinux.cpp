#include "CdDriveLinux.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>
#include <cctype>
#include <climits>

// C++ includes.
#include <string>
using std::string;

// open()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// SCSI and CD-ROM IOCTLs.
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <linux/cdrom.h>
#include "genscd_scsi.h"

// Sleep for a random amonut of time if open() fails.
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

namespace LibGensCD
{

CdDriveLinux::CdDriveLinux(const string& filename)
	: CdDrive(filename)
	, m_fd(-1)
{
	// TODO: Check if the device is mounted.
	bool isMounted = false;

	// Open the specified drive.
	int fd = -1;
	for (int cnt = 20; cnt > 0; cnt--) {
		fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK | (isMounted ? 0 : O_EXCL));
		if (fd >= 0 || errno != EBUSY)
			break;

		// Sleep for a random duration.
		// TODO: udev uses nanosleep(), do we want to use it?
		useconds_t usec = (100 * 1000) + (rand() % 100 * 1000);
		usleep(usec);
	}

	if (fd < 0) {
		// Error opening the device file.
		return;
	}

	// Device file is opened.
	m_fd = fd;
}

CdDriveLinux::~CdDriveLinux()
{
	// Make sure the CD-ROM device file is closed.
	close();
}

/**
 * Check if the CD-ROM device file is open.
 * @return True if open; false if not.
 */
bool CdDriveLinux::isOpen(void) const
{
	return (m_fd >= 0);
}

/**
 * Close the CD-ROM device file if it's open.
 */
void CdDriveLinux::close(void)
{
	if (m_fd >= 0) {
		::close(m_fd);
		m_fd = -1;
	}
}

/**
 * Check if a disc is present.
 * @return True if a disc is present; false if not.
 */
bool CdDriveLinux::isDiscPresent(void)
{
	if (!isOpen())
		return false;

	return (ioctl(m_fd, CDROM_DRIVE_STATUS, CDSL_CURRENT) == CDS_DISC_OK);
}

/**
 * Send a SCSI command descriptor block to the device.
 * @param cdb		[in] SCSI command.
 * @param cdb_len	[in] Length of cdb.
 * @param out		[out] Buffer for data received from the SCSI device.
 * @param out_len	[in] Length of out.
 * @param mode		[in] Data mode. IN == receive from SCSI; OUT == send to SCSI.
 * @return 0 on success; non-zero on error.
 */
int CdDriveLinux::scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				void *out, size_t out_len,
				scsi_data_mode mode)
{
	
	if (!isOpen())
		return -1;	// TODO: Proper SCSI error code.

	union {
		struct request_sense s;
		uint8_t u[18];
	} _sense;

	// Temporary buffer for the cdb.
	uint8_t cdb_tmp[16];
	if (cdb_len > sizeof(cdb_tmp)) {
		// WARNING: Truncating the CDB.
		cdb_len = sizeof(cdb_tmp);
	}
	memcpy(cdb_tmp, cdb, cdb_len);

	// TODO: Initialie this elsewhere?
	struct sg_io_hdr sg_io;
	memset(&sg_io, 0x00, sizeof(sg_io));
	sg_io.interface_id = 'S';
	sg_io.mx_sb_len = sizeof(_sense);
	sg_io.sbp = _sense.u;
	sg_io.cmd_len = cdb_len;
	sg_io.cmdp = cdb_tmp;
	sg_io.flags = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;

	// Determine the output buffer information.
	if (out_len > 0) {
		sg_io.dxferp = out;
		sg_io.dxfer_len = out_len;

		// Convert scsi_data_mode to SG data mode.
		switch (mode) {
			case SCSI_DATA_NONE:
				sg_io.dxfer_direction = SG_DXFER_NONE;
				break;
			case SCSI_DATA_IN:
				sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
				break;
			case SCSI_DATA_OUT:
				sg_io.dxfer_direction = SG_DXFER_TO_DEV;
				break;
			case SCSI_DATA_UNSPECIFIED:
			default:
				sg_io.dxfer_direction = SG_DXFER_TO_FROM_DEV;
				break;
		}
	} else {
		// No output buffer.
		sg_io.dxfer_direction = SG_DXFER_NONE;
	}

	// Run the ioctl.
	if (ioctl(m_fd, SG_IO, &sg_io)) {
		// ioctl failed.
		// TODO: Return a proper error code...
		return -1;
	}

	// Check if the command succeeded.
	int ret = 0;
	if ((sg_io.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
		// Command failed.
		ret = -1;
		if (sg_io.masked_status & CHECK_CONDITION) {
			ret = ERRCODE(_sense.u);
			if (ret == 0)
				ret = -1;
		}
	}

	return ret;
}

}
