#ifndef __LIBGENSCD_CDDRIVE_HPP__
#define __LIBGENSCD_CDDRIVE_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

namespace LibGensCD
{

class CdDrive
{
	public:
		CdDrive(const std::string& filename);
		virtual ~CdDrive();

		virtual bool isOpen(void) const = 0;
		virtual void close(void) = 0;

		int inquiry(void);
		bool isInquirySuccessful(void);
		std::string dev_vendor(void);
		std::string dev_model(void);
		std::string dev_firmware(void);

		uint16_t getDiscType(void);

	protected:
		enum scsi_data_mode
		{
			SCSI_DATA_IN,
			SCSI_DATA_OUT,
			SCSI_DATA_UNSPECIFIED
		};

		virtual int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
					  void *out, size_t out_len,
					  scsi_data_mode mode = SCSI_DATA_IN) = 0;

		// Device filename.
		std::string m_filename;

		enum InquiryStatus
		{
			INQ_NOT_DONE,
			INQ_SUCCESSFUL,
			INQ_FAILED
		};

		// INQUIRY results.
		// TODO: Move to private class?
		struct {
			// Inquiry status.
			InquiryStatus inq_status;

			// Peripheral device type.
			// Should be 0x05. (DTYPE_CDROM)
			uint8_t device_type;

			std::string vendor;
			std::string model;
			std::string firmware;
		} m_inq_data;
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
