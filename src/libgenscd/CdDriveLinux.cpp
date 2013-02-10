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

class CdDriveLinuxPrivate
{
	public:
		CdDriveLinuxPrivate(CdDriveLinux *q);
		~CdDriveLinuxPrivate();

	private:
		CdDriveLinux *const q;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		CdDriveLinuxPrivate(const CdDriveLinuxPrivate &);
		CdDriveLinuxPrivate &operator=(const CdDriveLinuxPrivate &);

	public:
		// Drive handle.
		int fd;

		// SG_IO struct.
		struct sg_io_hdr sg_io;

		// REQUEST SENSE buffer.
		union {
			struct request_sense s;
			uint8_t u[18];
		} _sense;
};

/** CdDriveLinuxPrivate **/

CdDriveLinuxPrivate::CdDriveLinuxPrivate(CdDriveLinux *q)
	: q(q)
	, fd(-1)
{
	// Initialize sg_io.
	memset(&sg_io, 0x00, sizeof(sg_io));
	sg_io.interface_id = 'S';
	sg_io.mx_sb_len = sizeof(_sense);
	sg_io.sbp = _sense.u;
	sg_io.flags = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
}

CdDriveLinuxPrivate::~CdDriveLinuxPrivate()
{
	if (fd >= 0)
		close(fd);
}

/** CdDriveLinux **/

CdDriveLinux::CdDriveLinux(const string& filename)
	: CdDrive(filename)
	, d(new CdDriveLinuxPrivate(this))
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
	d->fd = fd;
}

CdDriveLinux::~CdDriveLinux()
{
	// Make sure the CD-ROM device file is closed.
	close();

	delete d;
}

/**
 * Check if the CD-ROM device file is open.
 * @return True if open; false if not.
 */
bool CdDriveLinux::isOpen(void) const
{
	return (d->fd >= 0);
}

/**
 * Close the CD-ROM device file if it's open.
 */
void CdDriveLinux::close(void)
{
	if (d->fd >= 0) {
		::close(d->fd);
		d->fd = -1;
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

	return (ioctl(d->fd, CDROM_DRIVE_STATUS, CDSL_CURRENT) == CDS_DISC_OK);
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

	// Temporary buffer for the cdb.
	uint8_t cdb_tmp[16];
	if (cdb_len > sizeof(cdb_tmp)) {
		// WARNING: Truncating the CDB.
		cdb_len = sizeof(cdb_tmp);
	}
	memcpy(cdb_tmp, cdb, cdb_len);

	// Set the cdb pointers.
	d->sg_io.cmd_len = cdb_len;
	d->sg_io.cmdp = cdb_tmp;

	// Determine the output buffer information.
	if (out_len > 0) {
		d->sg_io.dxferp = out;
		d->sg_io.dxfer_len = out_len;

		// Convert scsi_data_mode to SG data mode.
		switch (mode) {
			case SCSI_DATA_NONE:
				d->sg_io.dxfer_direction = SG_DXFER_NONE;
				break;
			case SCSI_DATA_IN:
				d->sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
				break;
			case SCSI_DATA_OUT:
				d->sg_io.dxfer_direction = SG_DXFER_TO_DEV;
				break;
			case SCSI_DATA_UNSPECIFIED:
			default:
				d->sg_io.dxfer_direction = SG_DXFER_TO_FROM_DEV;
				break;
		}
	} else {
		// No output buffer.
		d->sg_io.dxferp = nullptr;
		d->sg_io.dxfer_len = 0;
		d->sg_io.dxfer_direction = SG_DXFER_NONE;
	}

	// Run the ioctl.
	if (ioctl(d->fd, SG_IO, &d->sg_io)) {
		// ioctl failed.
		// TODO: Return a proper error code...
		return -1;
	}

	// Check if the command succeeded.
	int ret = 0;
	if ((d->sg_io.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
		// Command failed.
		ret = -1;
		if (d->sg_io.masked_status & CHECK_CONDITION) {
			ret = ERRCODE(d->_sense.u);
			if (ret == 0)
				ret = -1;
		}
	}

	return ret;
}

}
