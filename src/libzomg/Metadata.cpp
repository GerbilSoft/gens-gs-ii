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

#include "Metadata.hpp"

// C includes. (C++ namespace)
#include <ctime>

// C++ includes.
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
using std::string;
using std::swap;
using std::ostringstream;

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
	// TODO: Is there a way to get 64-bit time_t on 32-bit Linux?
#ifdef _WIN32
#error TODO: Missing Win32 implementation.
#else
	// TODO: Use clock_gettime() for nanoseconds.
	ctime.seconds = time(nullptr);
	ctime.nano = 0;
#endif
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
	// Save creator information.
	MetadataPrivate::creatorInfo.creator =
		(creator ? string(creator) : string());
	MetadataPrivate::creatorInfo.creatorVersion =
		(creatorVersion ? string(creatorVersion) : string());
	MetadataPrivate::creatorInfo.creatorVcsVersion =
		(creatorVcsVersion ? string(creatorVcsVersion) : string());

       // Initialize system metadata.
	MetadataPrivate::InitSystemMetadata();
}

/**
 * Export the metadata as ZOMG.ini.
 * @return String representation of ZOMG.ini.
 */
std::string Metadata::toZomgIni(void) const
{
	ostringstream oss;

	// Write the ZOMG section header.
	oss << "[ZOMG]" << NL;

	// Write the ZOMG properties.
	// TODO: Make parts optional, e.g. Creator, Author, ROM Info.
	d->WriteValue(oss, "FileType", "Zipped Original Memory from Genesis");
	// TODO: Get the ZomgVersion from somewhere.
	d->WriteValue(oss, "Version", "0.1-DEV-UNSTABLE");
	d->WriteValue(oss, "System", d->systemId);
	d->WriteValue(oss, "Creator", d->creatorInfo.creator);
	d->WriteValue(oss, "CreatorVersion", d->creatorInfo.creatorVersion);
	d->WriteValue(oss, "CreatorVcsVersion", d->creatorInfo.creatorVcsVersion);
	d->WriteValue(oss, "OS", d->sysInfo.osVersion);
	d->WriteValue(oss, "CPU", d->sysInfo.cpu);
	d->WriteValue(oss, "Author", d->sysInfo.username);
	d->WriteValue(oss, "ROM", d->romFilename);
	d->WriteValue(oss, "ROM_CRC32", d->romCrc32, 8, true);
	// TODO: ROM size.
	d->WriteValue(oss, "Region", d->region);
	d->WriteValue(oss, "Description", d->description);
	d->WriteValue(oss, "Extensions", d->extensions);

	// Return the ZOMG.ini file.
	return oss.str();
}

/** Property get/set functions. **/
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
