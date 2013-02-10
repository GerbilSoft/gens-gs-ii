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

class CdDrivePrivate;

class CdDrive
{
	public:
		CdDrive(const std::string& filename);
		virtual ~CdDrive();

	private:
		friend class CdDrivePrivate;
		CdDrivePrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGensCD-specific version of Q_DISABLE_COPY().
		CdDrive(const CdDrive &);
		CdDrive &operator=(const CdDrive &);

	public:
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
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
