/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuManager.hpp: Emulation manager.                                      *
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

#ifndef __GENS_QT4_EMUMANAGER_HPP__
#define __GENS_QT4_EMUMANAGER_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtGui/QImage>

// LibGens includes.
#include "libgens/Rom.hpp"
#include "libgens/IO/IoBase.hpp"
#include "libgens/MD/SysVersion.hpp"

// paused_t
#include "gqt4_datatypes.h"

// InterlacedMode
// TODO: Move somewhere else?
#include "GensConfig.hpp"

namespace GensQt4
{

// Audio backend.
class GensPortAudio;

class EmuManager : public QObject
{
	Q_OBJECT
	
	public:
		EmuManager();
		~EmuManager();
		
		// TODO: Move the parent argument to EmuManager()?
		int openRom(QWidget *parent = 0);
		int openRom(const QString& filename);
		int loadRom(LibGens::Rom *rom);
		
		/**
		 * closeRom(): Close the open ROM file and stop emulation.
		 */
		int closeRom(void);
		
		// Emulation status and properties.
		inline bool isRomOpen(void) const
			{ return (m_rom != NULL); }
		inline paused_t paused(void) const
			{ return m_paused; }
		inline int saveSlot(void) const
			{ return m_saveSlot; }
		
		// ROM information.
		QString romName(void);	// Active ROM name.
		QString sysName(void);	// System name for the active ROM, based on ROM region.
		static QString SysName(LibGens::Rom::MDP_SYSTEM_ID sysId,
					LibGens::SysVersion::RegionCode_t region);
		static QString SysName_l(LibGens::Rom::MDP_SYSTEM_ID sysId);
		
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
		 * LgRegionCodeStr(): Get a string identifying a given LibGens region code.
		 * @param region Region code.
		 * @return Region code string, or empty string on error.
		 */
		static QString LgRegionCodeStr(LibGens::SysVersion::RegionCode_t region);
		
		/**
		 * LgRegionCodeStr(): Get a string identifying a given GensConfig region code.
		 * TODO: Combine ConfRegionCode_t with RegionCode_t.
		 * @param region Region code.
		 * @return Region code string, or empty string on error.
		 */
		static QString GcRegionCodeStr(GensConfig::ConfRegionCode_t region);
		
		/**
		 * LgRegionCodeStrMD(): Get a string identifying a given region code. (MD hex code)
		 * @param region Region code.
		 * @return Region code string, or empty string on error.
		 */
		static QString LgRegionCodeStrMD(int region);
	
	signals:
		void updateFps(double fps);
		void stateChanged(void);		// Emulation state changed. Update the Gens title.
		void updateVideo(void);			// Update the video widget in GensWindow.
		
		/**
		 * osdPrintMsg(): Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString& msg);
		
		/**
		 * osdShowPreview(): Show a preview image on the OSD.
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
		 * GetLgRegionCode(): Determine the LibGens region code to use.
		 * @param confRegionCode Current GensConfig region code.
		 * @param mdHexRegionCode ROM region code, in MD hex format.
		 * @param regionCodeOrder Region code order for auto-detection. (MSN == highest priority)
		 * @return LibGens region code to use.
		 */
		static LibGens::SysVersion::RegionCode_t GetLgRegionCode(
			GensConfig::ConfRegionCode_t confRegionCode,
			int mdHexRegionCode, uint16_t regionCodeOrder);
		
		/**
		 * closeRom(): Close the open ROM file and stop emulation.
		 * @param emitStateChanged If true, emits the stateChanged() signal after the ROM is closed.
		 */
		int closeRom(bool emitStateChanged);
		
		// Timing management.
		double m_lastTime;	// Last time a frame was updated.
		double m_lastTime_fps;	// Last time value used for FPS counter.
		int m_frames;
		
		// ROM object.
		LibGens::Rom *m_rom;
		
		// Audio backend.
		GensPortAudio *m_audio;
		
		// Paused state.
		paused_t m_paused;
		
		// Savestates.
		int m_saveSlot;
		QString getSaveStateFilename(void);
	
	protected slots:
		// Frame done signal from EmuThread.
		void emuFrameDone(bool wasFastFrame);
		
		// Calls openRom_int() with the stored filename.
		// HACK: Works around the threading issue when opening a new ROM without closing the old one.
		void sl_loadRom_int(void)
		{
			loadRom_int(m_loadRom_int_tmr_rom);
			m_loadRom_int_tmr_rom = NULL;
		}
	
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
				RQT_UNKNOWN		= 0,
				RQT_CTRLCHANGE		= 1,
				RQT_SCREENSHOT		= 2,
				RQT_AUDIO_RATE		= 3,
				RQT_AUDIO_STEREO	= 4,
				RQT_SAVE_STATE		= 5,
				RQT_LOAD_STATE		= 6,
				RQT_PAUSE_EMULATION	= 7,
				RQT_RESET		= 8,
				RQT_AUTOFIX_CHANGE	= 9,
				RQT_PALETTE_SETTING	= 10,
				RQT_SAVE_SLOT		= 11,
				RQT_RESET_CPU		= 12,
				RQT_REGION_CODE		= 13,
			};
			
			// RQT_PALETTE_SETTING types.
			enum PaletteSettingType
			{
				RQT_PS_UNKNOWN		= 0,
				RQT_PS_CONTRAST		= 1,
				RQT_PS_BRIGHTNESS	= 2,
				RQT_PS_GRAYSCALE	= 3,
				RQT_PS_INVERTED		= 4,
				RQT_PS_COLORSCALEMETHOD	= 5,
				RQT_PS_INTERLACEDMODE	= 6,
			};
			
			RequestType rqType;
			union
			{
				struct
				{
					int port;
					LibGens::IoBase::IoType ctrlType;
				} ctrlChange;
				
				int audioRate;
				bool audioStereo;
				
				// Savestates.
				struct
				{
					char *filename;
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
				GensConfig::ConfRegionCode_t region;
			};
		};
		
		/**
		 * m_qEmuRequest: Emulation Request Queue.
		 */
		QQueue<EmuRequest_t> m_qEmuRequest;
	
	/** Emulation Request Queue: Submission functions. **/
	
	public slots:
		/** Emulation settings. **/
		void setController(int port, LibGens::IoBase::IoType type);
		void screenShot(void);
		void setAudioRate(int newRate);
		void setStereo(bool newStereo);
		void resetCpu(int cpu_idx);
		
		/** Savestates. **/
		void saveState(void); // Save to current slot.
		void loadState(void); // Load from current slot.
		
		/**
		 * pauseRequest(): Toggle the paused state.
		 */
		void pauseRequest(bool newManualPaused);
		void pauseRequest(paused_t newPaused);
		void pauseRequest(paused_t paused_set, paused_t paused_clear);
	
	protected slots:
		/**
		 * saveSlot_changed_slot(): Set the save slot number.
		 * @param newSaveSlot Save slot number, (0-9)
		 */
		void saveSlot_changed_slot(int newSaveSlot);
		
		/**
		 * autoFixChecksum_changed_slot(): Change the Auto Fix Checksum setting.
		 * @param newAutoFixChecksum New Auto Fix Checksum setting.
		 */
		void autoFixChecksum_changed_slot(bool newAutoFixChecksum);
	
		/**
		 * regionCode_changed_slot(): Region code has changed.
		 * @param newRegionCode New region code setting.
		 */
		void regionCode_changed_slot(GensConfig::ConfRegionCode_t newRegionCode);
		
		/**
		 * regionCodeOrder_changed_slot(): Region code auto-detection order has changed.
		 * @param newRegionCodeOrder New region code auto-detection order setting.
		 */
		void regionCodeOrder_changed_slot(uint16_t newRegionCodeOrder);
	
	public slots:
		/**
		 * resetEmulator(): Reset the emulator.
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
		// TODO: Verify that this doesn't break on Mac OS X.
		void contrast_changed_slot(int newContrast)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_CONTRAST, newContrast); }
		void brightness_changed_slot(int newBrightness)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_BRIGHTNESS, newBrightness); }
		void grayscale_changed_slot(bool newGrayscale)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_GRAYSCALE, (int)newGrayscale); }
		void inverted_changed_slot(bool newInverted)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_INVERTED, (int)newInverted); }
		void colorScaleMethod_changed_slot(int newColorScaleMethod)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_COLORSCALEMETHOD, newColorScaleMethod); }
		void interlacedMode_changed_slot(GensConfig::InterlacedMode_t newInterlacedMode)
			{ changePaletteSetting(EmuRequest_t::RQT_PS_INTERLACEDMODE, (int)newInterlacedMode); }
	
	/** Emulation Request Queue: Processing functions. **/
	
	private:
		void processQEmuRequest(void);
		void doCtrlChange(int port, LibGens::IoBase::IoType type);
		
		QImage getMDScreen(void) const;
		void doScreenShot(void);
		
		void doAudioRate(int newRate);
		void doAudioStereo(bool newStereo);
		
		/** Savestates. **/
		void doSaveState(const char *filename, int saveSlot);
		void doLoadState(const char *filename, int saveSlot);
		void doSaveSlot(int newSaveSlot);
		
		void doPauseRequest(paused_t newPaused);
		void doResetEmulator(bool hardReset);
		
		void doChangePaletteSetting(EmuRequest_t::PaletteSettingType type, int val);
		void doResetCpu(ResetCpuIndex cpu_idx);
		
		void doRegionCode(GensConfig::ConfRegionCode_t region);
};


/**
 * closeRom(): Close the open ROM file and stop emulation.
 */
inline int EmuManager::closeRom(void)
	{ return closeRom(true); }

}

#endif /* __GENS_QT4_EMUMANAGER_HPP__ */
