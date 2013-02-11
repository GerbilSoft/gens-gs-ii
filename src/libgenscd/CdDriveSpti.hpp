#ifndef __LIBGENSCD_CDDRIVESPTI_HPP__
#define __LIBGENSCD_CDDRIVESPTI_HPP__

#ifndef _WIN32
#error SPTI is Win32 only.
#endif

// CdDrive base class.
#include "CdDrive.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace LibGensCD
{

class CdDriveSpti : public CdDrive
{
	public:
		CdDriveSpti(const std::string& filename);
		virtual ~CdDriveSpti();

		bool isOpen(void) const;
		void close(void);

		/**
		 * Check if a disc is present.
		 * @return True if a disc is present; false if not.
		 */
		bool isDiscPresent(void);

	protected:
		/**
		 * Send a SCSI command descriptor block to the drive.
		 * @param cdb		[in] SCSI command descriptor block.
		 * @param cdb_len	[in] Length of cdb.
		 * @param out		[out] Output buffer, or nullptr if no data is requested.
		 * @param out_len	[out] Length of out.
		 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
		 * @return 0 on success, non-zero on error. (TODO: Return SCSI sense key?)
		 */
		int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				  void *out, size_t out_len,
				  scsi_data_mode mode = SCSI_DATA_IN) override final;

		/**
		 * Check if the disc has changed since the last access.
		 * @return True if the disc has changed; false if not.
		 */
		bool hasDiscChanged(void) override final;

	private:
		// TODO: Move to private class?

		// Drive handle.
		HANDLE m_hDevice;
};

}

#endif /* __LIBGENSCD_CDDRIVESPTI_HPP__ */
