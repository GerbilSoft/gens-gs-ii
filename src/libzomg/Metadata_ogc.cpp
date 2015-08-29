/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Metadata_ogc.cpp: Metadata handler. (libogc)                            *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include <libzomg/config.libzomg.h>

#include "Metadata_p.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

// libogc
#include <gccore.h>
#include <ogcsys.h>

namespace LibZomg {

/**
 * Get the current time.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::init_ctime(void)
{
	ctime.seconds = time(nullptr);
	// FIXME: Remove nano field; never going to use it.
	ctime.nano = 0;
}

#ifdef HW_RVL
/**
 * Is this system a "Virtual Wii" on Wii U?
 * @return True if this is vWii; false if this is real Wii.
 */
static inline bool isWiiU(void)
{
	// Reference: https://github.com/FIX94/Nintendont/blob/master/loader/source/global.c
	return ( (*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE );
}

/**
 * Get a title's version number.
 * @param title_id Title ID.
 * @param pTmd Pointer to u16 to store title version.
 * @return 0 on success; non-zero on error.
 */
static int getTitleVersion(uint64_t title_id, uint16_t *pTitleVersion)
{
	// Reference: AnyTitle Deleter's detect_settings.c
	// http://wiibrew.org/wiki/AnyTitle_Deleter
	// NOTE: This won't work for IOS older than 21.
	// TODO: Implement AnyTitle Deleter's workaround?
	uint32_t tmd_size;
	int ret = ES_GetStoredTMDSize(title_id, &tmd_size);
	if (ret < 0) {
		// An error occurred.
		return ret;
	}

	if (tmd_size < sizeof(tmd) || tmd_size > 1024) {
		// TMD size is invalid.
		// TODO: Proper error code?
		return -1;
	}

	signed_blob *s_tmd = (signed_blob*)aligned_malloc(32, MAX_SIGNED_TMD_SIZE);
	if (!s_tmd) {
		// Can't allocate memory.
		// TODO: Proper error code?
		return -1;
	}

	ret = ES_GetStoredTMD(title_id, s_tmd, tmd_size);
	if (ret < 0) {
		// An error occurred.
		free(s_tmd);
		return ret;
	}

	// TMD retrieved.
	tmd *t = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
	*pTitleVersion = t->title_version;
	free(s_tmd);
	return 0;
}

/**
 * Get the display name for a system menu version number.
 * @param sysMenu_version System menu version number.
 * @return Display name, or nullptr if unknown.
 */
static const char *getSysMenuVersion(uint16_t sysMenu_version)
{
	switch (sysMenu_version) {
		// Wii
		// Reference: http://wiiubrew.org/wiki/Title_database
		// TODO: Does libogc have a database like this?
		case 33:	return "1.0";
		case 97:	return "2.0U";
		case 128:	return "2.0J";
		case 130:	return "2.0E";
		case 162:	return "2.1E";
		case 192:	return "2.2J";
		case 193:	return "2.2U";
		case 194:	return "2.2E";
		case 224:	return "3.0J";
		case 225:	return "3.0U";
		case 226:	return "3.0E";
		case 256:	return "3.1J";
		case 257:	return "3.1U";
		case 258:	return "3.1E";
		case 288:	return "3.2J";
		case 289:	return "3.2U";
		case 290:	return "3.2E";
		case 326:	return "3.3K";
		case 352:	return "3.3J";
		case 353:	return "3.3U";
		case 354:	return "3.3E";
		case 384:	return "3.4J";
		case 385:	return "3.4U";
		case 386:	return "3.4E";
		case 390:	return "3.5K";
		case 416:	return "4.0J";
		case 417:	return "4.0U";
		case 418:	return "4.0E";
		case 448:	return "4.1J";
		case 449:	return "4.1U";
		case 450:	return "4.1E";
		case 454:	return "4.1K";
		case 480:	return "4.2J";
		case 481:	return "4.2U";
		case 482:	return "4.2E";
		case 483:	return "4.2K";
		case 512:	return "4.3J";
		case 513:	return "4.3U";
		case 514:	return "4.3E";
		case 518:	return "4.3K";

		// vWii
		// Reference: http://wiiubrew.org/wiki/Title_database
		// NOTE: These are all listed as 4.3.
		// NOTE 2: vWii also has 512, 513, and 514.
		case 608:	return "4.3J";
		case 609:	return "4.3U";
		case 610:	return "4.3E";

		// Unknown version.
		default:		return nullptr;
	}
}
#endif /* defined(HW_RVL) */

/**
 * Initialize system-specific metadata.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::InitSystemMetadata(void)
{
	// OS version.
	// TODO: Determine libogc version somehow.
	// May require CMake preprocessing.

#if defined(HW_RVL)
	// Temporary snprintf() buffer.
	char buf[64];
	bool got_one = false;

	ostringstream oss;
	oss << (isWiiU() ? "vWii" : "Wii");

	// Get the System Menu version.
	// FIXME: May be 7-2 on vWii.
	static const uint64_t SysMenu_TitleID = 0x0000000100000002;

	uint16_t sysMenu_version;
	int ret = getTitleVersion(SysMenu_TitleID, &sysMenu_version);
	if (ret == 0 && sysMenu_version != 0) {
		// System menu version retrieved.
		got_one = true;
		oss << " System Menu ";
		const char *sysMenuVersion = getSysMenuVersion(sysMenu_version);
		if (sysMenuVersion != nullptr) {
			oss << sysMenuVersion;
		} else {
			// Unknown system menu version.
			snprintf(buf, sizeof(buf), "v%d", sysMenu_version);
			oss << buf;
		}
	}

	// Get the IOS version.
	if (got_one) {
		oss << ", ";
	} else {
		got_one = true;
	}
	snprintf(buf, sizeof(buf), "IOS%d (v%d.%d)",
		IOS_GetVersion(), IOS_GetRevisionMajor(), IOS_GetRevisionMinor());
	oss << buf;
	sysInfo.osVersion = oss.str();
#elif defined(HW_DOL)
	// TODO: libogc version.
	// TODO: GCN IPL version?
	sysInfo.osVersion = "GCN";
#else
	#error Unsupported libogc platform.
#endif

	// NOTE: Neither Wii nor GameCube have a username.
	// Wii U has a username, but it isn't accessible in Wii mode.
	sysInfo.username = string();

	// CPU information.
	// TODO: Check CPU version?
	// http://wiibrew.org/wiki/Hardware/Broadway
	// - Broadway == 87102, Gekko == 83410
#if defined(HW_RVL)
	if (isWiiU()) {
		sysInfo.cpu = "PowerPC 750CL \"Espresso\"";
	} else {
		sysInfo.cpu = "PowerPC 750CL \"Broadway\"";
	}
#elif defined(HW_DOL)
	sysInfo.cpu = "PowerPC 750CXe \"Gekko\"";
#else
	#error Unsupported libogc platform.
#endif

	// FIXME: There should be a way to retrieve the
	// actual CPU frequency instead of hard-coding it.
	snprintf(buf, sizeof(buf), " (%d MHz)", TB_CORE_CLOCK / 1000000);
	sysInfo.cpu += buf;
}

}
