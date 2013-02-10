#ifndef __LIBGENSCD_CDDRIVE_HPP__
#define __LIBGENSCD_CDDRIVE_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// Disc and drive type definitions.
#include "DiscType.h"

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

		/**
		 * Check if a disc is present.
		 * @return True if a disc is present; false if not.
		 */
		virtual bool isDiscPresent(void) = 0;

		/**
		 * Get the current disc type.
		 * @return Disc type.
		 */
		CD_DiscType_t getDiscType(void);

		/**
		 * Get the current drive type.
		 * @return Drive type.
		 */
		CD_DriveType_t getDriveType(void);

	protected:
		enum scsi_data_mode
		{
			SCSI_DATA_NONE,
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
		CD_DiscType_t mmcFeatureProfileToDiscType(uint16_t featureProfile);

		/**
		 * Convert a bitfield of CD_DiscType_t to a CD_DriveType_t.
		 * @param discTypes Bitfield of all CD_DiscType_t disc types.
		 * @return CD_DriveType_t.
		 */
		CD_DriveType_t discTypesToDriveType(uint32_t discTypes);
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
