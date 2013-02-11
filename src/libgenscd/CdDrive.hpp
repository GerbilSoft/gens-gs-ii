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

		std::string dev_vendor(void);
		std::string dev_model(void);
		std::string dev_firmware(void);

		/**
		 * Force a cache update.
		 * NOTE: Currently required for SPTI, since the
		 * MMC GET_EVENT_STATUS_NOTIFICATION command
		 * isn't working properly, and WM_DEVICECHANGE
		 * requires a window to receive notifications.
		 */
		void forceCacheUpdate(void);

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

		/**
		 * Send a SCSI command descriptor block to the drive.
		 * @param cdb		[in] SCSI command descriptor block.
		 * @param cdb_len	[in] Length of cdb.
		 * @param out		[out] Output buffer, or nullptr if no data is requested.
		 * @param out_len	[out] Length of out.
		 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
		 * @return 0 on success, non-zero on error. (TODO: Return SCSI sense key?)
		 */
		virtual int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
					  void *out, size_t out_len,
					  scsi_data_mode mode = SCSI_DATA_IN) = 0;

		/**
		 * Check if the disc has changed since the last access.
		 * @return True if the disc has changed; false if not.
		 */
		virtual bool hasDiscChanged(void) = 0;
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
