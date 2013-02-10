#ifndef __LIBGENSCD_CDDRIVELINUX_HPP__
#define __LIBGENSCD_CDDRIVELINUX_HPP__

#ifndef __linux__
#error CdDriveLinux is Linux only.
#endif

// CdDrive base class.
#include "CdDrive.hpp"

namespace LibGensCD
{

class CdDriveLinuxPrivate;

class CdDriveLinux : public CdDrive
{
	public:
		CdDriveLinux(const std::string& filename);
		virtual ~CdDriveLinux();

	private:
		friend class CdDriveLinuxPrivate;
		CdDriveLinuxPrivate *const d;

	public:
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
};

}

#endif /* __LIBGENSCD_CDDRIVELINUX_HPP__ */
