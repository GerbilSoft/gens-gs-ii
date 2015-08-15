/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuManager.hpp: Emulation manager.                                      *
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

#ifndef __GENS_QT4_EMUMANAGER_HPP__
#define __GENS_QT4_EMUMANAGER_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtGui/QImage>

// LibGens includes.
#include "libgens/Rom.hpp"
#include "libgens/IO/IoManager.hpp"

// LibGensKeys: Key Manager
#include "libgenskeys/KeyManager.hpp"

// paused_t
#include "gqt4_datatypes.h"

// Video Backend.
#include "VBackend/VBackend.hpp"

namespace GensQt4 {

// Audio backend.
class GensPortAudio;

class EmuManager : public QObject
{
	Q_OBJECT

	public:
		EmuManager(QObject *parent = 0, VBackend *vBackend = 0);
		~EmuManager();

		int openRom(void);
		int openRom(const QString &filename, const QString &z_filename = QString());
		int loadRom(LibGens::Rom *rom);

		/**
		 * Close the open ROM file and stop emulation.
		 */
		int closeRom(void);

		// Emulation status and properties.
		inline bool isRomOpen(void) const
			{ return (m_rom != nullptr); }
		inline paused_t paused(void) const
			{ return m_paused; }
		inline int saveSlot(void) const
			{ return m_saveSlot; }

		// ROM information.
		QString romName(void);	// Active ROM name.
		QString sysName(void);	// System name for the active ROM, based on ROM region.

		/** Rom class passthrough functions. **/

		inline LibGens::Rom::RomFormat romFormat(void) const
		{
			if (!m_rom)
				return LibGens::Rom::RFMT_UNKNOWN;
			return m_rom->romFormat();
		}

		inline LibGens::Rom::MDP_SYSTEM_ID sysId(void) const
		{
			if (!m_rom)
				return LibGens::Rom::MDP_SYSTEM_UNKNOWN;
			return m_rom->sysId();
		}

		/**
		 * Update video on the current VBackend.
		 */
		void updateVideo(void);

	signals:
		void updateFps(double fps);
		void stateChanged(void);		// Emulation state changed. Update the Gens title.

		/**
		 * Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString &msg);

		/**
		 * Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osdShowPreview(int duration, const QImage& img);

	protected:
		// Load ROM.
		// HACK: Works around the threading issue when opening a new ROM without closing the old one.
		// TODO: Fix the threading issue!
		int loadRom_int(LibGens::Rom *rom);
		LibGens::Rom *m_loadRom_int_tmr_rom;

		/**
		 * Determine the LibGens region code to use.
		 * @param confRegionCode Current GensConfig region code.
		 * @param mdHexRegionCode ROM region code, in MD hex format.
		 * @param regionCodeOrder Region code order for auto-detection. (MSN == highest priority)
		 * @return LibGens region code to use.
		 */
		static LibGens::SysVersion::RegionCode_t GetLgRegionCode(
			LibGens::SysVersion::RegionCode_t confRegionCode,
			int mdHexRegionCode, uint16_t regionCodeOrder);

		/**
		 * Close the open ROM file and stop emulation.
		 * @param emitStateChanged If true, emits the stateChanged() signal after the ROM is closed.
		 */
		int closeRom(bool emitStateChanged);

		// Timing management.
		LibGens::Timing m_timing;
		uint64_t m_lastTime;		// Last time a frame was updated.
		uint64_t m_lastTime_fps;	// Last time value used for FPS counter.
		int m_frames;

		// ROM object.
		LibGens::Rom *m_rom;

		// Audio backend.
		GensPortAudio *m_audio;

		// Paused state.
		paused_t m_paused;

		/** Savestates. **/
		int m_saveSlot;

		/**
		 * Get the savestate filename.
		 * TODO: Move savestate code to another file?
		 * NOTE: Returned filename uses Qt directory separators. ('/')
		 * @return Savestate filename, or empty string if no ROM is loaded.
		 */
		QString getSaveStateFilename(void);

	protected slots:
		// Frame done signal from EmuThread.
		void emuFrameDone(bool wasFastFrame);

		// Calls openRom_int() with the stored filename.
		// HACK: Works around the threading issue when opening a new ROM without closing the old one.
		void sl_loadRom_int(void)
		{
			loadRom_int(m_loadRom_int_tmr_rom);
			m_loadRom_int_tmr_rom = nullptr;
		}

	/** Key Manager. **/
	// NOTE: Not owned by EmuManager.
	protected:
		LibGensKeys::KeyManager *m_keyManager;
	public:
		// TODO: Better way to handle this.
		LibGensKeys::KeyManager *keyManager(void) const
			{ return m_keyManager; }
		void setKeyManager(LibGensKeys::KeyManager *keyManager)
			{ m_keyManager = keyManager; }

	/** Video Backend. **/
	public:
		void setVBackend(VBackend *vBackend);
		VBackend *vBackend(void)
			{ return m_vBackend; }
		void updateVBackend(void);
	private:
		// Video Backend.
		VBackend *m_vBackend;
	private slots:
		void vBackend_destroyed(QObject *obj);

	/** Temporary source FB for ROM close handling. **/
	public:
		/**
		 * Get the MdFb from the last closed ROM.
		 * @return MdFb.
		 */
		LibGens::MdFb *romClosedFb(void);

		/**
		 * Reset the last closed ROM MdFb.
		 */
		void clearRomClosedFb(void);
	private:
		LibGens::MdFb *m_romClosedFb;

	/** Translatable string functions. **/

	public:
		/**
		 * Get a string identifying a given LibGens region code.
		 * @param region Region code.
		 * @return Region code string, or empty string on error.
		 */
		static QString LgRegionCodeStr(LibGens::SysVersion::RegionCode_t region);

		/**
		 * StrMD(): Get a string identifying a given region code. (MD hex code)
		 * @param region Region code.
		 * @return Region code string, or empty string on error.
		 */
		static QString LgRegionCodeStrMD(int region);

		/**
		 * Get the system name for the specified system ID and region.
		 * @param sysID System ID.
		 * @param region Region.
		 * @return System name, or empty string on error.
		 */
		static QString SysName(LibGens::Rom::MDP_SYSTEM_ID sysId,
					LibGens::SysVersion::RegionCode_t region);

		/**
		 * Get the localized system name for the specified system ID.
		 * @param sysID System ID.
		 * @return Localized system name, or empty string on error.
		 */
		static QString SysName_l(LibGens::Rom::MDP_SYSTEM_ID sysId);

		/**
		 * RomFormat(): Get the ROM format name for the specified ROM format ID.
		 * @param romFormat ROM format ID.
		 * @return ROM format name, or empty string on error.
		 */
		QString RomFormat(LibGens::Rom::RomFormat romFormat);

	/** Emulation Request Struct. **/

	public:
		// RQT_RESET_CPU indexes.
		// TODO: Use MDP CPU indexes.
		enum ResetCpuIndex
		{
			RQT_CPU_M68K		= 0,
			RQT_CPU_Z80		= 1,
			RQT_CPU_S68K		= 2,
			RQT_CPU_MSH2		= 3,
			RQT_CPU_SSH2		= 4,
		};

	protected:
		/**
		 * EmuRequest_t: Emulation Request struct.
		 */
		struct EmuRequest_t
		{
			enum RequestType
			{
				RQT_UNKNOWN = 0,
				RQT_SCREENSHOT,
				RQT_AUDIO_RATE,
				RQT_AUDIO_STEREO,
				RQT_SAVE_STATE,
				RQT_LOAD_STATE,
				RQT_PAUSE_EMULATION,
				RQT_RESET,
				RQT_AUTOFIX_CHANGE,
				RQT_PALETTE_SETTING,
				RQT_SAVE_SLOT,
				RQT_RESET_CPU,
				RQT_REGION_CODE,
				RQT_ENABLE_SRAM,
			};

			// RQT_PALETTE_SETTING types.
			enum PaletteSettingType
			{
				RQT_PS_UNKNOWN = 0,
				
				// These aren't really "palette" settings...
				// TODO: Rename to VdpSettingType?
				RQT_PS_INTERLACEDMODE,
				RQT_PS_BORDERCOLOREMULATION,
				RQT_PS_NTSCV30ROLLING,
				RQT_PS_SPRITELIMITS,
				RQT_PS_ZEROLENGTHDMA,
				RQT_PS_VSCROLLBUG,
				RQT_PS_UPDATEPALETTEINVBLANKONLY,
				RQT_PS_ENABLEINTERLACEDMODE
			};

			RequestType rqType;
			union
			{
				struct
				{
					int port;
					LibGens::IoManager::IoType_t ctrlType;
				} ctrlChange;

				int audioRate;
				bool audioStereo;

				// Savestates.
				struct
				{
					// NOTE: filename must be deleted after use!
					QString *filename;
					int saveSlot;
				} saveState;

				// Paused settings.
				paused_t newPaused;

				// Emulator Reset.
				bool hardReset;

				// Auto Fix Checksum.
				bool autoFixChecksum;

				// Palette Settings.
				struct
				{
					PaletteSettingType ps_type;
					int ps_val;
				} PaletteSettings;

				// Reset CPU.
				ResetCpuIndex cpu_idx;

				// Region code.
				LibGens::SysVersion::RegionCode_t region;

				// Enable/disable SRam.
				bool enableSRam;
			};
		};

		/**
		 * Emulation Request Queue.
		 */
		QQueue<EmuRequest_t> m_qEmuRequest;

	/** Emulation Request Queue: Submission functions. **/

	public slots:
		/** Emulation settings. **/
		void screenShot(void);
		void setAudioRate(int newRate);
		void setStereo(bool newStereo);
		void resetCpu(int cpu_idx);

		/** Savestates. **/
		void saveState(void); // Save to current slot.
		void loadState(void); // Load from current slot.

		/**
		 * Toggle the paused state.
		 */
		void pauseRequest(bool newManualPaused);
		void pauseRequest(paused_t newPaused);
		void pauseRequest(paused_t paused_set, paused_t paused_clear);

	protected slots:
		/**
		 * Set the save slot number.
		 * @param saveSlot (int) Save slot number, (0-9)
		 */
		void saveSlot_changed_slot(const QVariant &saveSlot);

		/**
		 * Change the Auto Fix Checksum setting.
		 * @param autoFixChecksum (bool) New Auto Fix Checksum setting.
		 */
		void autoFixChecksum_changed_slot(const QVariant &autoFixChecksum);

		/**
		 * Region code has changed.
		 * @param regionCode (int) New region code setting.
		 */
		void regionCode_changed_slot(const QVariant &regionCode); // LibGens::SysVersion::RegionCode_t

		/**
		 * Region code auto-detection order has changed.
		 * @param regionCodeOrder (uint16_t) New region code auto-detection order setting.
		 */
		void regionCodeOrder_changed_slot(const QVariant &regionCodeOrder);

		/**
		 * Enable SRam/EEPRom setting has changed.
		 * @param enableSRam (bool) New Enable SRam/EEPRom setting.
		 */
		void enableSRam_changed_slot(const QVariant &enableSRam);

	public slots:
		/**
		 * Reset the emulator.
		 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
		 */
		void resetEmulator(bool hardReset);

		/**
		 * changePaletteSetting(): Change a palette setting.
		 * @param type Type of palette setting.
		 * @param val New value.
		 */
		void changePaletteSetting(EmuRequest_t::PaletteSettingType type, int val);

	protected slots:
		/** Graphics settings. **/
		void interlacedMode_changed_slot(const QVariant &interlacedMode);

		/** VDP settings. **/
		void borderColorEmulation_changed_slot(const QVariant &borderColorEmulation);
		void ntscV30Rolling_changed_slot(const QVariant &ntscV30Rolling);
		void spriteLimits_changed_slot(const QVariant &spriteLimits);
		void zeroLengthDMA_changed_slot(const QVariant &zeroLengthDMA);
		void vscrollBug_changed_slot(const QVariant &vscrollBug);
		void updatePaletteInVBlankOnly_changed_slot(const QVariant &updatePaletteInVBlankOnly);
		void enableInterlacedMode_changed_slot(const QVariant &enableInterlacedMode);

	/** Emulation Request Queue: Processing functions. **/

	private:
		void processQEmuRequest(void);

		QImage getMDScreen(void) const;
		void doScreenShot(void);

		void doAudioRate(int newRate);
		void doAudioStereo(bool newStereo);

		/** Savestates. **/
		void doSaveState(QString filename, int saveSlot);
		void doLoadState(QString filename, int saveSlot);
		void doSaveSlot(int newSaveSlot);

		void doPauseRequest(paused_t newPaused);
		void doResetEmulator(bool hardReset);

		void doChangePaletteSetting(EmuRequest_t::PaletteSettingType type, int val);
		void doResetCpu(ResetCpuIndex cpu_idx);

		void doRegionCode(LibGens::SysVersion::RegionCode_t region);

		void doEnableSRam(bool enableSRam);
};

/**
 * Close the open ROM file and stop emulation.
 */
inline int EmuManager::closeRom(void)
	{ return closeRom(true); }

/** Temporary source FB for ROM close handling. **/

/**
 * Get the MdFb from the last closed ROM.
 * @return MdFb.
 */
inline LibGens::MdFb *EmuManager::romClosedFb(void)
	{ return m_romClosedFb; }

/**
 * Reset the last closed ROM MdFb.
 */
inline void EmuManager::clearRomClosedFb(void)
{
	if (m_romClosedFb) {
		m_romClosedFb->unref();
		m_romClosedFb = nullptr;
	}
}

}

#endif /* __GENS_QT4_EMUMANAGER_HPP__ */
