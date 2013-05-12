/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContext.hpp: Emulation context base class.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_EMUCONTEXT_HPP__
#define __LIBGENS_EMUCONTEXT_HPP__

// Controller I/O manager.
#include "IO/IoManager.hpp"

// ROM image class.
#include "Rom.hpp"

// SRam and EEPRom.
#include "Save/SRam.hpp"
#include "Save/EEPRom.hpp"

// Region code.
// TODO: Make the region code non-console-specific.
#include "MD/SysVersion.hpp"

// VDP.
#include "Vdp/Vdp.hpp"

// C++ includes.
#include <string>

namespace LibGens
{

class EmuContext
{
	public:
		EmuContext(Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
		EmuContext(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region = SysVersion::REGION_US_NTSC);
		virtual ~EmuContext();

	private:
		void init(MdFb *fb, Rom *rom, SysVersion::RegionCode_t region);

	public:	
		// Get the current EmuContext instance.
		static EmuContext *Instance(void);

		/**
		 * saveData(): Save SRam/EEPRom.
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int saveData(void);

		/**
		 * autoSaveData(): AutoSave SRam/EEPRom.
		 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int autoSaveData(int framesElapsed);

		/**
		 * softReset(): Perform a soft reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int softReset(void) = 0;

		/**
		 * hardReset(): Perform a hard reset.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int hardReset(void) = 0;

		/**
		 * setRegion(): Set the region code.
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

		// SRam / EEPRom access.
		// TODO: Providing pointers like this is bad...
		inline SRam *getSRam(void) { return &m_SRam; }
		inline EEPRom *getEEPRom(void) { return &m_EEPRom; }

		inline const SRam *getSRam(void) const { return &m_SRam; }
		inline const EEPRom *getEEPRom(void) const { return &m_EEPRom; }

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
		static inline SRam *GetSRam(void) { return m_instance->getSRam(); }
		static inline EEPRom *GetEEPRom(void) { return m_instance->getEEPRom(); }

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
		static inline const char *PathSRam(void)
			{ return ms_PathSRam.c_str(); }
		static void SetPathSRam(const char *newPathSRam);

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

		// SRam and EEPRom.
		// TODO: Add a function to reset all memory handling.
		SRam m_SRam;
		EEPRom m_EEPRom;

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

#endif /* __LIBGENS_EMUCONTEXT_HPP__ */
