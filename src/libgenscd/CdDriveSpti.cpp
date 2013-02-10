#include "CdDriveSpti.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>
#include <cctype>

// C++ includes.
#include <string>
using std::string;

// NTDDSCSI is required for SPTI.
#include <ntddscsi.h>

// SPTI/SCSI headers.
#include <devioctl.h>
#include "genscd_scsi.h"

namespace LibGensCD
{

CdDriveSpti::CdDriveSpti(const string& filename)
	: CdDrive(filename)
	, m_hDevice(INVALID_HANDLE_VALUE)
{
	// Attempt to open the CD-ROM device.
	// Assume that filename is in the format "C:" for now.
	if (filename.size() < 1 || !isalpha(filename.at(0))) {
		// Invalid filename.
		return;
	}

	// Convert the drive letter to uppercase.
	char drive_letter = toupper(filename.at(0));

	// Create the pathname.
	const char spti_pathname[7] =
		{'\\', '\\', '.', '\\', drive_letter, ':', 0};

	// Open the specified drive using SPTI.
	// TODO: Error handling.
	m_hDevice = CreateFileA(spti_pathname,				// lpFileName
				GENERIC_READ | GENERIC_WRITE,		// dwDesiredAccess
				FILE_SHARE_READ | FILE_SHARE_WRITE,	// dwShareMode
				nullptr,				// lpSecurityAttributes
				OPEN_EXISTING,				// dwCreationDisposition,
				FILE_ATTRIBUTE_NORMAL,			// dwFlagsAndAttributes
				nullptr);				// hTemplateFile

}

CdDriveSpti::~CdDriveSpti()
{
	// Make sure the CD-ROM device file is closed.
	close();
}

/**
 * Check if the CD-ROM device file is open.
 * @return True if open; false if not.
 */
bool CdDriveSpti::isOpen(void) const
{
	return !!m_hDevice;
}

/**
 * Close the CD-ROM device file if it's open.
 */
void CdDriveSpti::close(void)
{
	if (m_hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
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
int CdDriveSpti::scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				void *out, size_t out_len,
				scsi_data_mode mode)
{
	if (!isOpen())
		return -1;	// TODO: Proper SCSI error code.

	// Based on http://www.codeproject.com/KB/system/mydvdregion.aspx
	DWORD returned;

	// Size of SCSI_PASS_THROUGH + 96 bytes for sense data.
	uint8_t cmd[sizeof(SCSI_PASS_THROUGH_DIRECT) + 96];
	memset(cmd, 0x00, sizeof(cmd));

	// Shortcut to the buffer.
	SCSI_PASS_THROUGH_DIRECT *pcmd = (SCSI_PASS_THROUGH_DIRECT*)cmd;

	// Copy the CDB to the SCSI_PASS_THROUGH structure.
	memcpy(pcmd->Cdb, cdb, cdb_len);

	// Convert scsi_data_mode to Win32 DataIn.
	uint8_t DataIn;
	switch (mode) {
		case SCSI_DATA_IN:	DataIn = SCSI_IOCTL_DATA_IN; break;
		case SCSI_DATA_OUT:	DataIn = SCSI_IOCTL_DATA_OUT; break;

		case SCSI_DATA_UNSPECIFIED:
		default:
			DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
			break;
	}

	// Initialize the other SCSI command variables.
	pcmd->DataBuffer = out;
	pcmd->DataTransferLength = out_len;
	pcmd->DataIn = DataIn;
	pcmd->CdbLength = cdb_len;
	pcmd->Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	pcmd->SenseInfoLength = (sizeof(cmd) - sizeof(SCSI_PASS_THROUGH_DIRECT));
	pcmd->SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);
	pcmd->TimeOutValue = 5; // 5-second timeout.

	DeviceIoControl(m_hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
			(LPVOID)&cmd, sizeof(cmd),
			(LPVOID)&cmd, sizeof(cmd),
			&returned, NULL);

	// TODO: Better error handling.
	DWORD err = GetLastError();
	if (err != ERROR_SUCCESS)
		fprintf(stderr, "%s(): Error: 0x%08X\n", __func__, (unsigned int)err);

	// Return the error code.
	return err;
}

}
