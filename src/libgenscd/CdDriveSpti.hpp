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
		int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				  void *out, size_t out_len,
				  scsi_data_mode mode = SCSI_DATA_IN) override;

	private:
		// TODO: Move to private class?

		// Drive handle.
		HANDLE m_hDevice;
};

}

#endif /* __LIBGENSCD_CDDRIVE_HPP__ */
