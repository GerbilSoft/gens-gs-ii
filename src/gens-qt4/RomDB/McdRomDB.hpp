/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * McdRomDB.h: Sega CD Boot ROM database.                                  *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth                                  *
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

#ifndef __GENSQT4_MCD_ROM_DB_HPP__
#define __GENSQT4_MCD_ROM_DB_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>

namespace GensQt4 {

class McdRomDB : public QObject
{
	Q_OBJECT

	// Static class.
	// TODO: Make it non-static, and load the database
	// from a file?
	protected:
		McdRomDB();
		~McdRomDB();

	private:
		typedef QObject super;
	private:
		Q_DISABLE_COPY(McdRomDB)

	public:
		// Region codes.
		// Matches the country code found in later MD games.
		// TODO: Move this somewhere else and make it not MCD-specific.
		enum MCD_RegionCode_t {
			// Bit flags.
			MCD_REGION_JAPAN	= 0x01,
			MCD_REGION_ASIA		= 0x02,
			MCD_REGION_USA		= 0x04,
			MCD_REGION_EUROPE	= 0x08,
			
			MCD_REGION_INVALID	= -1
		};

		// ROM status.
		enum MCD_RomStatus_t {
			RomStatus_Recommended	= 0x00,
			RomStatus_Supported	= 0x01,
			RomStatus_Unsupported	= 0x02,
			RomStatus_Broken	= 0x03,
			
			RomStatus_MAX
		};

		// Boot ROM information class.
		// QString functions provide translated strings
		// based on the current language setting.
		class McdBootRomInfo {
			public:
				McdBootRomInfo();
			protected:
				friend class McdRomDB;
				McdBootRomInfo(int romId);

			public:
				/**
				 * Is this ROM information valid?
				 * @return True if valid; false if not.
				 */
				bool isValid(void) const
					{ return (m_romId >= 0); }

				/**
				 * Get this Boot ROM's description.
				 * @return Boot ROM description.
				 */
				QString description(void) const;

				/**
				 * Get this Boot ROM's notes.
				 * @return Boot ROM notes.
				 */
				QString notes(void) const;

				/**
				 * Get this Boot ROM's region code.
				 * This includes all hardware regions accepted by this Boot ROM.
				 * @return Boot ROM region code.
				 */
				int region(void) const;

				/**
				 * Get this Boot ROM's primary region code.
				 * @return Boot ROM primary region code
				 */
				int primaryRegion(void) const;

				/**
				 * Get this Boot ROM's support status.
				 * @return ROM support status.
				 */
				MCD_RomStatus_t supportStatus(void) const;

			private:
				int m_romId;	// -1 for invalid.
		};

		/**
		* ROM fixup data.
		* lg_mcd_rom_InitSP: Initial SP. (0xFFFFFD00)
		* lg_mcd_rom_InitHINT: Initial HINT vector. (0xFFFFFD0C)
		*/
		static const uint8_t InitSP[4];
		static const uint8_t InitHINT[4];

		/**
		 * Look up a Sega CD Boot ROM using its CRC32.
		 * @param rom_crc32 ROM CRC32.
		 * @return Boot ROM information.
		 */
		static McdBootRomInfo findByCrc32(uint32_t rom_crc32);

		/**
		 * Get a string describing a primary region code.
		 * @param region_code Primary region code.
		 * @return Region code string, or NULL if the region code is invalid.
		 */
		static QString regionCodeString(int region_code);
};

}

#endif /* __GENSQT4_MCD_ROM_DB_HPP__ */
