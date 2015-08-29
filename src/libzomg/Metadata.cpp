/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Metadata.cpp: Metadata handler for savestates and screenshots.          *
 *                                                                         *
 * Copyright (c) 2013-2015 by David Korth.                                 *
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

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

// Ensure CPU_Flags is initialized.
#include "libcompat/cpuflags.h"

#include "Metadata.hpp"

// C includes. (C++ namespace)
#include <ctime>
#include <cstring>

// C++ includes.
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
using std::string;
using std::swap;
using std::ostringstream;

// libpng
#include <png.h>

// Platform-dependent newline constant.
#ifdef _WIN32
#define NL "\r\n"
#else
#define NL "\n"
#endif

#include "Metadata_p.hpp"
namespace LibZomg {

MetadataPrivate::SysInfo_t MetadataPrivate::sysInfo;
MetadataPrivate::CreatorInfo_t MetadataPrivate::creatorInfo;

/******************************
 * MetadataPrivate functions. *
 ******************************/

MetadataPrivate::MetadataPrivate()
	: romCrc32(0)
{
	// Get the current time.
	init_ctime();
}

/**
 * Write an INI value to an ostringstream.
 * @param key Key.
 * @param value Value.
 */
void MetadataPrivate::WriteValue(ostringstream& oss, const string &key, const string &value)
{
	oss << key << "=";
	for (size_t i = 0; i < value.length(); i++) {
		char chr = value.at(i);
		switch (chr) {
			case '\r':
				// Ignore '\r' here.
				// It'll be reinserted if the INI is loaded
				// on a Windows system.
				break;
			case '\n':
				// Escape newlines.
				oss << "\\n";
				break;
			case '\\':
				// Escape backslashes.
				oss << "\\\\";
				break;
			default:
				// Other character.
				oss << chr;
				break;
		}
	}
	oss << NL;
}

/**
 * Write an INI value to an ostringstream.
 * @param key Key.
 * @param value Value.
 * @param width Minimum width of the field.
 * @param hex If true, print in hex (with leading "0x").
 */
void MetadataPrivate::WriteValue(ostringstream& oss, const string &key, uint32_t value, int width, bool hex)
{
	std::ios_base::fmtflags old_flags = oss.flags();
	std::streamsize old_width = oss.width();
	char old_fill = oss.fill();

	oss << key << "=";
	if (hex) {
		oss << "0x";
		oss << std::hex << std::uppercase;
		// std::showbase doesn't work properly.
		// If value == 0, it doesn't do anything.
		// if value == 1, it prints 000000x1.
	}
	oss.width(width);
	oss.fill('0');
	oss << value;
	oss.flags(old_flags);
	oss.fill(old_fill);
	oss.width(old_width);
	oss << NL;
}

/**********************
 * Metadata functions. *
 **********************/

Metadata::Metadata()
	: d(new MetadataPrivate())
{ }

Metadata::~Metadata()
{
	delete d;
}

/**
 * Clear the loaded ZOMG.ini data.
 */
void Metadata::clear(void)
{
	// Swap the private class with a new instance.
	Metadata empty;
	swap(d, empty.d);
}

/**
 * Initialize the system and program metadata.
 * This function should only be run once at program startup.
 * System information will be obtained by the Metadata class.
 *
 * @param creator              	[in, opt] Emulator name.
 * @param creatorVersion       	[in, opt] Emulator version.
 * @param creatorVcsVersion    	[in, opt] Emulator's version control version, e.g. git tag.
 */
void Metadata::InitProgramMetadata(const char *creator,
				const char *creatorVersion,
				const char *creatorVcsVersion)
{
	// Ensure CPU_Flags is initialized.
	LibCompat_GetCPUFlags();

	// Save creator information.
	if (creator) {
		MetadataPrivate::creatorInfo.creator = string(creator);
	}
	if (creatorVersion) {
		MetadataPrivate::creatorInfo.creatorVersion = string(creatorVersion);
	}
	if (creatorVcsVersion) {
		MetadataPrivate::creatorInfo.creatorVcsVersion = string(creatorVcsVersion);
	}

	// Save the CPU name.
	const char *cpuName = LibCompat_GetCPUFullName();
	if (cpuName) {
		MetadataPrivate::sysInfo.cpu = string(cpuName);
	}

	// Initialize system metadata.
	MetadataPrivate::InitSystemMetadata();
}

/**
 * Export the metadata as ZOMG.ini.
 * @param metaFlags Metadata to export. (See MetadataFlags for values.)
 * @return String representation of ZOMG.ini.
 */
std::string Metadata::toZomgIni(int metaFlags) const
{
	if (metaFlags < 0) {
		// Use the default metadata flags.
		metaFlags = MetadataPrivate::MetadataFlagsDefault;
	}

	ostringstream oss;

	// Write the ZOMG section header.
	oss << "[ZOMG]" << NL;

	// Write the ZOMG properties.
	// TODO: Make parts optional, e.g. Creator, Author, ROM Info.
	d->WriteValue(oss, "FileType", "Zipped Original Memory from Genesis");
	// TODO: Get the ZomgVersion from somewhere.
	d->WriteValue(oss, "Version", "0.1-DEV-UNSTABLE");
	d->WriteValue(oss, "System", d->systemId);

	// System metadata.
	if (metaFlags & MF_Emulator) {
		d->WriteValue(oss, "Creator", d->creatorInfo.creator);
		d->WriteValue(oss, "CreatorVersion", d->creatorInfo.creatorVersion);
		d->WriteValue(oss, "CreatorVcsVersion", d->creatorInfo.creatorVcsVersion);
	}
	if (metaFlags & MF_CreationTime) {
		// TODO
		d->WriteValue(oss, "CreationTime", "" /*FormatCreationTime()*/);
	}
	if (metaFlags & MF_OSandCPU) {
		d->WriteValue(oss, "OS", d->sysInfo.osVersion);
		d->WriteValue(oss, "CPU", d->sysInfo.cpu);
	}
	if (metaFlags & MF_Author) {
		d->WriteValue(oss, "Author", d->sysInfo.username);
	}

	// ROM metadata.
	if (metaFlags & MF_RomInfo) {
		d->WriteValue(oss, "ROM", d->romFilename);
		if (d->romCrc32 != 0) {
			d->WriteValue(oss, "ROM_CRC32", d->romCrc32, 8, true);
		} else {
			d->WriteValue(oss, "ROM_CRC32", "");
		}
		// TODO
		d->WriteValue(oss, "ROM_Size", "" /*d->romSize, 1, false*/);
		d->WriteValue(oss, "Region", d->region);
	}
	d->WriteValue(oss, "Description", d->description);
	d->WriteValue(oss, "Extensions", d->extensions);

	// TODO: Remove the final NL on Windows?

	// Return the ZOMG.ini file.
	return oss.str();
}

/**
 * Export the metadata in PNG chunks.
 * @param png_ptr PNG pointer.
 * @param info_ptr PNG info pointer.
 * @param metaFlags Metadata to export. (See MetadataFlags for values.)
 * @return 0 on success; non-zero on error.
 */
int Metadata::toPngData(png_structp png_ptr, png_infop info_ptr, int metaFlags) const
{
	if (metaFlags < 0) {
		// Use the default metadata flags.
		metaFlags = MetadataPrivate::MetadataFlagsDefault;
	}

	// Text chunks are stored in the order defined by the PNG specification.
	// - http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
	// - http://www.w3.org/TR/PNG-Chunks.html
	// NOTE: We're not using zTXt here because we want the metadata
	// to be easily extracted using hex editors, scripts, etc.
	// TODO: Use iTXt for UTF-8 where applicable. (Check that libpng supports iTXt first!)
	// TODO: Flag field to indicate if Author, Emulator, and/or ROM information should be saved
	// NOTE: Text fields are defined as png_charp, so they can't be const.
	png_text txt;
	memset(&txt, 0, sizeof(txt));
	txt.compression = PNG_TEXT_COMPRESSION_NONE;

	// Author.
	if ((metaFlags & MF_Author) && !d->sysInfo.username.empty()) {
		txt.key = (png_charp)"Author";
		txt.text = (png_charp)d->sysInfo.username.c_str();
		txt.text_length = d->sysInfo.username.size();
		png_set_text(png_ptr, info_ptr, &txt, 1);
	}

	/** "Description" field. **/

	// Emulator information.
	ostringstream desc;
	if ((metaFlags & MF_Emulator) && !d->creatorInfo.creator.empty()) {
		desc << "Emulator: " << d->creatorInfo.creator << '\n';
		if (!d->creatorInfo.creatorVersion.empty()) {
			desc << "Version: " << d->creatorInfo.creatorVersion;
			if (!d->creatorInfo.creatorVcsVersion.empty()) {
				desc << " (" << d->creatorInfo.creatorVcsVersion << ')';
			}
			desc << '\n';
		}
	}

	// OS and CPU information.
	if (metaFlags & MF_OSandCPU) {
		if (!d->sysInfo.osVersion.empty()) {
			desc << "OS: " << d->sysInfo.osVersion << '\n';
		}
		if (!d->sysInfo.cpu.empty()) {
			desc << "CPU: " << d->sysInfo.cpu << '\n';
		}
	}

	// System.
	// NOTE: This is required for ZOMG.ini, so we'll
	// always save it in screenshots as well.
	if (!d->systemId.empty()) {
		desc << "System: " << d->systemId << '\n';
	}

	// ROM information.
	if (metaFlags & MF_RomInfo) {
		if (!d->romFilename.empty()) {
			desc << "ROM: " << d->romFilename << '\n';
			if (d->romCrc32 != 0) {
				char buf[16];
				snprintf(buf, sizeof(buf), "%08X", d->romCrc32);
				desc << "ROM CRC32: " << buf << "\n";
			}
		}

		/* TODO: ROM size.
		if (d->romSize > 0) {
			desc << "ROM Size: " << d->romSize << '\n';
		}
		*/
		if (!d->region.empty()) {
			desc << "Region: " << d->region << '\n';
		}
	}

	// Additional description provided by the user.
	if (!d->description.empty()) {
		// TODO: Don't append a newline if desc is empty.
		desc << '\n' << d->description;
	}

	// Write the "Description" field.
	string desc_str = desc.str();
	// Remove the trailing newline if one is present.
	if (!desc_str.empty() && desc_str.at(desc_str.size() - 1) == '\n') {
		desc_str.resize(desc_str.size() - 1);
	}
	if (!desc_str.empty()) {
		txt.key = (png_charp)"Description";
		txt.text = (png_charp)desc_str.c_str();
		txt.text_length = desc_str.size();
		png_set_text(png_ptr, info_ptr, &txt, 1);
	}

	if (metaFlags & MF_CreationTime) {
		// Save the creation time.
		// TODO: Store nanoseconds?
		png_time ctimePng;
		// TODO: Ensure libpng is using 64-bit time_t.
		png_convert_from_time_t(&ctimePng, d->ctime.seconds);
		png_set_tIME(png_ptr, info_ptr, &ctimePng);     

		// Write the current time as a string.
		// This is used by Windows 7 for "Date taken:".
		// NOTE: This must be in LOCAL time.
		// Format: "yyyy:MM:dd hh:mm:ss"
		// NOTE: PNG specification says to use RFC-822, but Windows doesn't recognize it.
		struct tm ctime_tm;
		if (localtime_r(&d->ctime.seconds, &ctime_tm)) {
			char ctime_str[24];
			snprintf(ctime_str, sizeof(ctime_str), "%04d:%02d:%02d %02d:%02d:%02d",
				 ctime_tm.tm_year+1900, ctime_tm.tm_mon+1, ctime_tm.tm_mday,
				 ctime_tm.tm_hour, ctime_tm.tm_min, ctime_tm.tm_sec);

			// FIXME: Needs to be non-const...
			txt.key = (png_charp)"Creation Time";
			txt.text = ctime_str;
			txt.text_length = strlen(ctime_str);
			png_set_text(png_ptr, info_ptr, &txt, 1);
		}
	}

	// Done.
	return 0;
}

/** File metadata functions. **/
// TODO: Use macros?

// TODO: Use bitfield with system IDs instead of a string.
string Metadata::systemId(void) const
	{ return d->systemId; }
void Metadata::setSystemId(const string &systemId)
	{ d->systemId = systemId; }

string Metadata::romFilename(void) const
	{ return d->romFilename; }
void Metadata::setRomFilename(const string &romFilename)
	{ d->romFilename = romFilename; }

uint32_t Metadata::romCrc32(void) const
	{ return d->romCrc32; }
void Metadata::setRomCrc32(uint32_t romCrc32)
	{ d->romCrc32 = romCrc32; }

string Metadata::region(void) const
	{ return d->region; }
void Metadata::setRegion(const string &region)
	{ d->region = region; }

string Metadata::description(void) const
	{ return d->description; }
void Metadata::setDescription(const string &description)
	{ d->description = description; }

string Metadata::extensions(void) const
	{ return d->extensions; }
void Metadata::setExtensions(const string &extensions)
	{ d->extensions = extensions; }

}
