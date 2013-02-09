#include "CdDrive.hpp"

// C includes. (C++ namespace)
#include <cstring>

// C++ includes.
#include <string>
using std::string;

// SCSI commands.
#include "genscd_scsi.h"

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
	CDB_SCSI inquiry;
	SCSI_INQUIRY_STD_DATA data;

	memset(&inquiry, 0x00, sizeof(inquiry));
	memset(&data, 0x00, sizeof(data));

	// Inquiry hasn't been done yet.
	m_inq_data.inq_status = INQ_NOT_DONE;

	// Set SCSI operation type.
	inquiry.OperationCode = SCSI_INQUIRY;
	inquiry.AllocationLength = sizeof(data);

	// Send the SCSI CDB.
	if (scsi_send_cdb(&inquiry, sizeof(inquiry), &data, sizeof(data))) {
		// Inquiry failed.
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

}
