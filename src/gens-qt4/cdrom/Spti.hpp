/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * Spti.hpp: Win32 SPTI wrapper.                                           *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

// TODO: Move this to libgens later.

#ifndef __GENS_QT4_CDROM_SPTI_HPP__
#define __GENS_QT4_CDROM_SPTI_HPP__

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// NTDDSCSI is required for SPTI.
// TODO: Create a separate SPTI class for Win32.
#include <ntddscsi.h>

// C++ includes.
#include <stdint.h>
#include <string>

namespace GensQt4
{

class Spti
{
	public:
		Spti(char drive_letter);
		~Spti();
		
		void close(void);
		
		inline bool isOpen(void)
			{ return (m_hDevice != INVALID_HANDLE_VALUE); }
		
		/** SCSI functions. **/
		int scsiInquiry(void);
		
		/** Wrapper functions. **/
		bool isMediumPresent(void);
		
		/** SCSI inquiry functions. **/
		uint8_t inqDevType(void)
			{ return m_inq_dev_type; }
		const char *inqVendor(void)
			{ return m_inq_vendor.c_str(); }
		const char *inqModel(void)
			{ return m_inq_model.c_str(); }
		const char *inqFirmware(void)
			{ return m_inq_firmware.c_str(); }
	
	protected:
		// Drive handle.
		HANDLE m_hDevice;
		
		// Send SCSI command block.
		int scsiSendCdb(void *cdb, unsigned char cdb_length,
					void *buffer, unsigned int buffer_length,
					int data_in = SCSI_IOCTL_DATA_IN);
		
		// Trim spaces.
		static void TrimEndSpaces(char *buf, int len);
		
		/** SCSI inquiry data. **/
		uint8_t m_inq_dev_type;		// Device type.
		std::string m_inq_vendor;	// Vendor.
		std::string m_inq_model;	// Model.
		std::string m_inq_firmware;	// Firmware revision.
};

}

#endif /* __GENS_QT4_CDROM_SPTI_HPP__ */
