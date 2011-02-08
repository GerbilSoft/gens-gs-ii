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

// paused_t
#include "gqt4_datatypes.h"

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
		int closeRom(void);
		
		inline bool isRomOpen(void) const
			{ return (m_rom != NULL); }
		inline paused_t paused(void) const
			{ return m_paused; }
		
		// ROM information.
		QString romName(void);
		QString sysName(void);
		
		/** Emulation settings. **/
		void setController(int port, LibGens::IoBase::IoType type);
		void screenShot(void);
		void setAudioRate(int newRate);
		void setStereo(bool newStereo);
		
		/** Savestates. **/
		int saveSlot(void) const { return m_saveSlot; }
		void saveState(void); // Save to current slot.
		void loadState(void); // Load from current slot.
		
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
		
		// Emulation requests.
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
			};
		};
		
		QQueue<EmuRequest_t> m_qEmuRequest;
		void processQEmuRequest(void);
		void doCtrlChange(int port, LibGens::IoBase::IoType type);
		
		QImage getMDScreen(void) const;
		void doScreenShot(void);
		
		void doAudioRate(int newRate);
		void doAudioStereo(bool newStereo);
		
		/** Savestates. **/
		int m_saveSlot;
		QString getSaveStateFilename(void);
		void doSaveState(const char *filename, int saveSlot);
		void doLoadState(const char *filename, int saveSlot);
		
		void doPauseRequest(paused_t newPaused);
		void doResetEmulator(bool hardReset);
	
	public slots:
		/**
		 * pauseRequest(): Toggle the paused state.
		 */
		void pauseRequest(void);
		void pauseRequest(paused_t newPaused);
		void pauseRequest(paused_t paused_set, paused_t paused_clear);
		
		/**
		 * resetEmulator(): Reset the emulator.
		 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
		 */
		void resetEmulator(bool hardReset);
	
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
		
		/**
		 * saveSlot_changed_slot(): Set the save slot number.
		 * @param slotNum Slot number, (0-9)
		 */
		void saveSlot_changed_slot(int slotNum);
		
		/**
		 * autoFixChecksum_changed_slot(): Change the Auto Fix Checksum setting.
		 * @param newAutoFixChecksum New Auto Fix Checksum setting.
		 */
		void autoFixChecksum_changed_slot(bool newAutoFixChecksum);
};

}

#endif /* __GENS_QT4_EMUMANAGER_HPP__ */
