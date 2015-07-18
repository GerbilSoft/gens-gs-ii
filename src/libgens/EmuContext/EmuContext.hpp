/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContext.hpp: Emulation context base class.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBGENS_EMUCONTEXT_EMUCONTEXT_HPP__
#define __LIBGENS_EMUCONTEXT_EMUCONTEXT_HPP__

// Controller I/O manager.
#include "../IO/IoManager.hpp"

// ROM image class.
#include "../Rom.hpp"

// Region code.
// TODO: Make the region code non-console-specific.
#include "SysVersion.hpp"

// VDP.
#include "../Vdp/Vdp.hpp"

// C++ includes.
#include <string>

namespace LibGens {

class EmuContext
{
	public:
		EmuContext(Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
		EmuContext(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
		virtual ~EmuContext();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		EmuContext(const EmuContext &);
		EmuContext &operator=(const EmuContext &);

	private:
		void init(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region);

	public:	
		// Get the current EmuContext instance.
		static EmuContext *Instance(void);

		/**
		 * Save SRam/EEPRom.
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		virtual int saveData(void) = 0;

		/**
		 * AutoSave SRam/EEPRom.
		 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		virtual int autoSaveData(int framesElapsed) = 0;

		/**
		 * Perform a soft reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int softReset(void) = 0;

		/**
		 * Perform a hard reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int hardReset(void) = 0;

		/**
		 * Set the region code.
		 * @param region Region code.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int setRegion(SysVersion::RegionCode_t region) = 0;

		/** Frame execution functions. **/
		virtual void execFrame(void) = 0;
		virtual void execFrameFast(void) = 0;

		// Accessors.
		inline bool isRomOpened(void)
			{ return (m_rom != nullptr); }

		// Controller I/O manager.
		// TODO: Make this non-static!
		static IoManager *m_ioManager;

		/**
		 * Read the system version register. (MD)
		 * @return MD version register.
		 */
		uint8_t readVersionRegister_MD(void) const;

		/**
		 * Get a const pointer to the version register.
		 * @return Const pointer to the version register.
		 */
		const SysVersion *versionRegisterObject(void) const;

		// TODO: Add isPal(), isNtsc(), regionCode(), etc. wrapper functions/

		// Save data enable bit.
		bool saveDataEnable(void);
		void setSaveDataEnable(bool newSaveDataEnable);

		// Static functions. Temporarily needed for SRam/EEPRom.
		static inline bool GetSaveDataEnable(void) { return m_instance->m_saveDataEnable; }

		/**
		 * Load the current state from a ZOMG file.
		 * @param filename	[in] ZOMG file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int zomgLoad(const utf8_str *filename) = 0;

		/**
		 * Save the current state to a ZOMG file.
		 * @param filename	[in] ZOMG file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int zomgSave(const utf8_str *filename) const = 0;

		/**
		 * Global settings.
		 */
		// TODO: Maybe autofix/unfix checksums in active contexts when this is changed?
		static inline bool AutoFixChecksum(void)
			{ return ms_AutoFixChecksum; }
		static inline void SetAutoFixChecksum(bool newAutoFixChecksum)
			{ ms_AutoFixChecksum = newAutoFixChecksum; }

		/**
		 * Pathnames.
		 */
		static inline std::string PathSRam(void)
			{ return ms_PathSRam; }
		static void SetPathSRam(const std::string &newPathSRam);

		static inline std::string TmssRomFilename(void)
			{ return ms_TmssRomFilename; }
		static void SetTmssRomFilename(const std::string &tmssRomFilename)
			{ ms_TmssRomFilename = tmssRomFilename; }
		static inline bool TmssEnabled(void)
			{ return ms_TmssEnabled; }
		static void SetTmssEnabled(bool tmssEnabled)
			{ ms_TmssEnabled = tmssEnabled; }

		/** VDP (TODO) **/
		Vdp *m_vdp;

		/**
		 * Get the Rom class being used by this emulator context.
		 * @return Rom class.
		 */
		const Rom *rom(void) const;

	protected:
		Rom *m_rom;
		bool m_saveDataEnable;

		/**
		 * System version register.
		 */
		SysVersion m_sysVersion;

		// Static pointer. Temporarily needed for SRam/EEPRom.
		static EmuContext *m_instance;

		/**
		 * Global settings.
		 */
		static bool ms_AutoFixChecksum;
		static std::string ms_PathSRam;
		static std::string ms_TmssRomFilename;
		static bool ms_TmssEnabled;

	private:
		static int ms_RefCount;
};

// Get the current EmuContext instance.
inline EmuContext *EmuContext::Instance(void)
	{ return m_instance; }

/**
 * Read the system version register. (MD)
 * @return MD version register.
 */
inline uint8_t EmuContext::readVersionRegister_MD(void) const
	{ return m_sysVersion.readData(); }

/**
 * Get a const pointer to the version register.
 * @return Const pointer to the version register.
 */
inline const SysVersion *EmuContext::versionRegisterObject(void) const
	{ return &m_sysVersion; }

inline bool EmuContext::saveDataEnable(void)
	{ return m_saveDataEnable; }
inline void EmuContext::setSaveDataEnable(bool newSaveDataEnable)
	{ m_saveDataEnable = newSaveDataEnable; }

/**
 * Get the Rom class being used by this emulator context.
 * @return Rom class.
 */
inline const Rom *EmuContext::rom(void) const
	{ return m_rom; }

}

#endif /* __LIBGENS_EMUCONTEXT_EMUCONTEXT_HPP__ */
