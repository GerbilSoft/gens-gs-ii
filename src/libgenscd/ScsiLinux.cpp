/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * ScsiLinux.cpp: Linux SCSI device handler class.                         *
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

#include "ScsiLinux.hpp"

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

// Sleep for a random amonut of time if open() fails.
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

namespace LibGensCD
{

class ScsiLinuxPrivate
{
	public:
		ScsiLinuxPrivate(ScsiLinux *q);
		~ScsiLinuxPrivate();

	private:
		ScsiLinux *const q;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		ScsiLinuxPrivate(const ScsiLinuxPrivate &);
		ScsiLinuxPrivate &operator=(const ScsiLinuxPrivate &);

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

/** ScsiLinuxPrivate **/

ScsiLinuxPrivate::ScsiLinuxPrivate(ScsiLinux *q)
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

ScsiLinuxPrivate::~ScsiLinuxPrivate()
{
	if (fd >= 0)
		close(fd);
}

/** ScsiLinux **/

ScsiLinux::ScsiLinux(const string& filename)
	: ScsiBase(filename)
	, d(new ScsiLinuxPrivate(this))
{
	// Open the specified drive.
	int fd = -1;
	for (int cnt = 20; cnt > 0; cnt--) {
		fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);
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

ScsiLinux::~ScsiLinux()
{
	// Make sure the CD-ROM device file is closed.
	close();

	delete d;
}

/**
 * Check if the CD-ROM device file is open.
 * @return True if open; false if not.
 */
bool ScsiLinux::isOpen(void) const
{
	return (d->fd >= 0);
}

/**
 * Close the CD-ROM device file if it's open.
 */
void ScsiLinux::close(void)
{
	if (d->fd >= 0) {
		::close(d->fd);
		d->fd = -1;
	}
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
void ScsiLinux::printScsiError(uint8_t op, int err, FILE *f)
{
	if (!f)
		f = stderr;

	if (err == 0) {
		return;
	} else if (err < 0) {
		// OS-specific error.
		fprintf(f, "OP=%02X, err=%d: %s\n", op, -err, strerror(-err));
	} else /*if (err > 0)*/ {
		// SCSI sense key.
		ScsiBase::printScsiError(op, err, f);
	}
}

/**
 * Check if a disc is present.
 * @return True if a disc is present; false if not.
 */
bool ScsiLinux::isDiscPresent(void)
{
	if (!isOpen())
		return false;

	return (ioctl(d->fd, CDROM_DRIVE_STATUS, CDSL_CURRENT) == CDS_DISC_OK);
}

/**
 * Check if the disc has changed since the last access.
 * @return True if the disc has changed; false if not.
 */
bool ScsiLinux::hasDiscChanged(void)
{
	int chg = ioctl(d->fd, CDROM_MEDIA_CHANGED, 0);

	// TODO: Handle chg == -1 (drive doesn't support checking).
	return (chg == 1);
}

/**
 * Send a SCSI command descriptor block to the device.
 * @param cdb		[in] SCSI command descriptor block.
 * @param cdb_len	[in] Length of cdb.
 * @param data		[in/out] Data buffer, or nullptr for SCSI_DATA_NONE operations.
 * @param data_len	[in] Length of data.
 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
 * @return 0 on success, positive for SCSI sense key, negative for OS error.
 */
int ScsiLinux::scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				void *data, size_t data_len,
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
	if (data_len > 0) {
		d->sg_io.dxferp = data;
		d->sg_io.dxfer_len = data_len;

		// Convert scsi_data_mode to SG data mode.
		switch (mode) {
			case SCSI_DATA_NONE:
			default:
				d->sg_io.dxfer_direction = SG_DXFER_NONE;
				break;
			case SCSI_DATA_IN:
				d->sg_io.dxfer_direction = SG_DXFER_FROM_DEV;
				break;
			case SCSI_DATA_OUT:
				d->sg_io.dxfer_direction = SG_DXFER_TO_DEV;
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
		return -errno;
	}

	// Check if the command succeeded.
	int ret = 0;
	if ((d->sg_io.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
		// Command failed.
		ret = -EINVAL;
		if (d->sg_io.masked_status & CHECK_CONDITION) {
			ret = ERRCODE(d->_sense.u);
			if (ret == 0)
				ret = -EINVAL;
		}
	}

	return ret;
}

}
