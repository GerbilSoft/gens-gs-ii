/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuManager.cpp: Emulation manager.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "EmuManager.hpp"
#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/Util/Timing.hpp"
#include "libgens/MD/EmuMD.hpp"

// LibGens Sound Manager.
// Needed for LibGens::SoundMgr::MAX_SAMPLING_RATE.
#include "libgens/sound/SoundMgr.hpp"

// LibGens video includes.
#include "libgens/MD/VdpPalette.hpp"
#include "libgens/MD/VdpRend.hpp"
#include "libgens/MD/VdpIo.hpp"
#include "libgens/MD/TAB336.h"

// M68K_Mem.hpp has SysRegion.
#include "libgens/cpu/M68K_Mem.hpp"

// Multi-File Archive Selection Dialog.
#include "ZipSelectDialog.hpp"

// Qt includes.
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtCore/QBuffer>

// Qt key handler.
// TODO: Make a class for handling non-controller input, e.g. Reset.
#include "Input/KeyHandlerQt.hpp"

// Text translation macro.
#define TR(text) \
	QApplication::translate("EmuManager", (text), NULL, QApplication::UnicodeUTF8)

namespace GensQt4
{

// Reset keys.
// TODO: Make these customizable.
// TODO: Make a class for handling non-controller input, e.g. Reset.
// TODO: Add modifier keyvals or something.
// For now, Tab == soft reset; Shift-Tab == hard reset
static const GensKey_t ms_ResetKey = KEYV_TAB;

EmuManager::EmuManager()
{
	// Reset timing information.
	m_lastTime = 0.0;
	m_frames = 0;
	
	// No ROM is loaded at startup.
	m_rom = NULL;
	m_paused = false;
	
	// TODO: Load the last save slot from the configuration file.
	m_saveSlot = 0;
	
	// Create the Audio Backend.
	// TODO: Allow selection of all available audio backend classes.
	// NOTE: Audio backends are NOT QWidgets!
	m_audio = new GensPortAudio();
}

EmuManager::~EmuManager()
{
	// Delete the audio backend.
	m_audio->close();
	delete m_audio;
	m_audio = NULL;
	
	// Delete the ROM.
	// TODO
	
	m_paused = false;
}


/**
 * openRom(): Open a ROM file.
 * Prompts the user to select a ROM file, then opens it.
 * TODO: Add a parameter to open a filename directly, e.g. from ROM History.
 * @param parent Parent window for the modal dialog box.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(QWidget *parent)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"
	
	QString filename = QFileDialog::getOpenFileName(parent,
			TR("Open ROM"),		// Dialog title
			"",			// Default filename.
			TR("Sega Genesis ROM images") +
			" (*.bin *.gen"
#ifdef HAVE_ZLIB
			ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
			LZMA_EXT
#endif /* HAVE_LZMA */
			RAR_EXT
			");;" +
#if 0
			TR("Sega Genesis / 32X ROMs; Sega CD disc images") +
			"(*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT ");;" +
#endif
			TR("All Files") + "(*.*)");
	
	if (filename.isEmpty())
		return 1;
	
	if (gqt4_emuThread || m_rom)
	{
		// Close the ROM first.
		closeRom();
	}
	
	// Open the file using the LibGens::Rom class.
	// TODO: This won't work for KIO...
	m_rom = new LibGens::Rom(filename.toUtf8().constData());
	if (!m_rom->isOpen())
	{
		// Couldn't open the ROM file.
		fprintf(stderr, "Error opening ROM file. (TODO: Get error information.)\n");
		delete m_rom;
		m_rom = NULL;
		return 2;
	}
	
	// Check if this is a multi-file ROM archive.
	if (m_rom->isMultiFile())
	{
		// Multi-file ROM archive.
		// Prompt the user to select a file.
		ZipSelectDialog *zipsel = new ZipSelectDialog();
		zipsel->setFileList(m_rom->get_z_entry_list());
		int ret = zipsel->exec();
		if (ret != QDialog::Accepted || zipsel->selectedFile() == NULL)
		{
			// Dialog was rejected.
			delete m_rom;
			m_rom = NULL;
			return 6;
		}
		
		// Get the selected file.
		m_rom->select_z_entry(zipsel->selectedFile());
		delete zipsel;
	}
	
	printf("ROM information: format == %d, system == %d\n", m_rom->romFormat(), m_rom->sysId());
	
	if (m_rom->sysId() != LibGens::Rom::MDP_SYSTEM_MD)
	{
		// Only MD ROM images are supported.
		fprintf(stderr, "ERROR: Only Sega Genesis / Mega Drive ROM images are supported right now.\n");
		delete m_rom;
		m_rom = NULL;
		return 3;
	}
	
	if (m_rom->romFormat() != LibGens::Rom::RFMT_BINARY)
	{
		// Only binary ROM images are supported.
		fprintf(stderr, "ERROR: Only binary ROM images are supported right now.\n");
		delete m_rom;
		m_rom = NULL;
		return 4;
	}
	
	// Load the ROM image in EmuMD.
	int ret = LibGens::EmuMD::SetRom(m_rom);
	m_rom->close();
	
	if (ret != 0)
	{
		// Error loading the ROM image in EmuMD.
		// TODO: EmuMD error code constants.
		// TODO: Show an error message.
		fprintf(stderr, "Error: LibGens:EmuMD::Set_Rom(m_rom) returned %d.\n", ret);
		delete m_rom;
		m_rom = NULL;
		return 5;
	}
	
	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	
	// Open audio.
	m_audio->open();
	
	// Start the emulation thread.
	m_paused = false;
	gqt4_emuThread = new EmuThread();
	QObject::connect(gqt4_emuThread, SIGNAL(frameDone(bool)),
			 this, SLOT(emuFrameDone(bool)));
	gqt4_emuThread->start();
	
	// Update the Gens title.
	emit stateChanged();
	return 0;
}


/**
 * closeRom(): Close the open ROM file and stop emulation.
 */
int EmuManager::closeRom(void)
{
	if (gqt4_emuThread)
	{
		// Stop the emulation thread.
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	if (m_rom)
	{
		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		LibGens::EmuMD::SaveData(m_rom);
		
		// Delete the Rom instance.
		delete m_rom;
		m_rom = NULL;
	}
	
	// Close audio.
	m_audio->close();
	m_paused = false;
	
	// TODO: Start the idle animation thread if an idle animation is specified.
	// For now, just clear the screen.
	LibGens::VdpIo::Reset();
	emit updateVideo();
	
	// Update the Gens title.
	emit stateChanged();
	return 0;
}


/**
 * romName(): Get the ROM name.
 * @return ROM name, or empty string if no ROM is loaded.
 */
QString EmuManager::romName(void)
{
	if (!m_rom)
		return QString();
	
	// TODO: This is MD/MCD/32X only!
	
	// Check the active system region.
	const char *s_romName;
	if (LibGens::M68K_Mem::ms_Region.isEast())
	{
		// East (JP). Return the domestic ROM name.
		s_romName = m_rom->romNameJP();
		if (!s_romName || s_romName[0] == 0x00)
		{
			// Domestic ROM name is empty.
			// Return the overseas ROM name.
			s_romName = m_rom->romNameUS();
		}
	}
	else
	{
		// West (US/EU). Return the overseas ROM name.
		s_romName = m_rom->romNameUS();
		if (!s_romName || s_romName[0] == 0x00)
		{
			// Overseas ROM name is empty.
			// Return the domestic ROM name.
			s_romName = m_rom->romNameJP();
		}
	}
	
	// Return the ROM name.
	if (!s_romName)
		return QString();
	return QString::fromUtf8(s_romName);
}


/**
 * sysName(): Get the system name.
 * @return System name, or empty string if no ROM is loaded.
 */
QString EmuManager::sysName(void)
{
	if (!m_rom)
		return QString();
	
	// Check the system ID.
	const LibGens::SysRegion &region = LibGens::M68K_Mem::ms_Region;
	
	switch (m_rom->sysId())
	{
		case LibGens::Rom::MDP_SYSTEM_MD:
			// Genesis / Mega Drive.
			if (region.region() == LibGens::SysRegion::REGION_US_NTSC)
				return TR("Genesis");
			else
				return TR("Mega Drive");
		
		case LibGens::Rom::MDP_SYSTEM_MCD:
			if (region.region() == LibGens::SysRegion::REGION_US_NTSC)
				return TR("Sega CD");
			else
				return TR("Mega CD");
		
		case LibGens::Rom::MDP_SYSTEM_32X:
			if (region.isPal())
				return TR("32X (PAL)");
			else
				return TR("32X (NTSC)");
		
		case LibGens::Rom::MDP_SYSTEM_MCD32X:
			if (region.region() == LibGens::SysRegion::REGION_US_NTSC)
				return TR("Sega CD 32X");
			else
				return TR("Mega CD 32X");
		
		case LibGens::Rom::MDP_SYSTEM_SMS:
			return TR("Master System");
		
		case LibGens::Rom::MDP_SYSTEM_GG:
			return TR("Game Gear");
		
		case LibGens::Rom::MDP_SYSTEM_SG1000:
			return TR("SG-1000");
		
		case LibGens::Rom::MDP_SYSTEM_PICO:
			return TR("Pico");
		
		default:
			return "Unknown";
	}
}


/**
 * setController(): Set a controller type.
 * @param port Controller port. (0, 1, 2)
 * TODO: Teamplayer/4WP support.
 * @param type Controller type.
 */
void EmuManager::setController(int port, LibGens::IoBase::IoType type)
{
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_CTRLCHANGE;
	rq.ctrlChange.port = port;
	rq.ctrlChange.ctrlType = type;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused)
		processQEmuRequest();
}


/**
 * screenShot(): Request a screenshot.
 */
void EmuManager::screenShot(void)
{
	if (!m_rom)
	{
		// Screenshots are kinda useless if
		// no ROM is running.
		return;
	}
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SCREENSHOT;
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused)
		processQEmuRequest();
}


/**
 * setAudioRate(): Set the audio sampling rate.
 * @param newRate New audio sampling rate.
 */
void EmuManager::setAudioRate(int newRate)
{
	if (newRate > LibGens::SoundMgr::MAX_SAMPLING_RATE)
		return;
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUDIO_RATE;
	rq.audioRate = newRate;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused)
		processQEmuRequest();
}


/**
 * setStereo(): Set stereo or mono.
 * @param newStereo True for stereo; false for mono.
 */
void EmuManager::setStereo(bool newStereo)
{
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUDIO_STEREO;
	rq.audioStereo = newStereo;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused)
		processQEmuRequest();
}


/**
 * saveState(): Save the current emulation state.
 * TODO: Save to save slot based on filename and slot number.
 */
void EmuManager::saveState(void)
{
	if (!m_rom)
		return;
	
	// Create the savestate filename.
	// TODO: Move to another function?
	QString saveStateFilename = QString("%1.%2.zomg");
	saveStateFilename = saveStateFilename.arg(QString::fromUtf8(m_rom->filenameBaseNoExt()));
	saveStateFilename = saveStateFilename.arg(m_saveSlot);
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_STATE;
	rq.filename = strdup(saveStateFilename.toUtf8().constData());
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused)
		processQEmuRequest();
}

/**
 * loadState(): Load the emulation state from a file.
 * TODO: Load from save slot based on filename and slot number.
 */
void EmuManager::loadState(void)
{
	if (!m_rom)
		return;
	
	// Create the savestate filename.
	// TODO: Move to another function?
	QString saveStateFilename = QString("%1.%2.zomg");
	saveStateFilename = saveStateFilename.arg(QString::fromUtf8(m_rom->filenameBaseNoExt()));
	saveStateFilename = saveStateFilename.arg(m_saveSlot);
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_LOAD_STATE;
	rq.filename = strdup(saveStateFilename.toUtf8().constData());
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused)
		processQEmuRequest();
}


/**
 * pauseRequest(): Toggle the paused state.
 */
void EmuManager::pauseRequest(void)
{
	if (!m_rom)
		return;
	
	if (m_paused)
	{
		// Unpause the ROM immediately.
		// TODO: Reset the FPS counter?
		m_paused = false;
		m_audio->open();	// TODO: Add a resume() function.
		if (gqt4_emuThread)
			gqt4_emuThread->resume(false);
		emit stateChanged();
	}
	else
	{
		// Queue the pause request.
		EmuRequest_t rq;
		rq.rqType = EmuRequest_t::RQT_PAUSE_TOGGLE;
		m_qEmuRequest.enqueue(rq);
	}
}


/**
 * setSaveSlot(): Set the save slot number.
 * @param slotNum Slot number, (0-9)
 */
void EmuManager::setSaveSlot(int slotNum)
{
	// TODO: Should this use the emulation request queue?
	// TODO: Check if save slot is occupied; load preview.
	if (slotNum < 0 || slotNum > 9)
		return;
	m_saveSlot = slotNum;
	
	QString osdMsg = TR("Save Slot %1: [TODO]").arg(slotNum);
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * resetEmulator(): Reset the emulator.
 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
 */
void EmuManager::resetEmulator(bool hardReset)
{
	if (!m_rom)
		return;
	
	// Queue the reset request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_RESET;
	rq.hardReset = hardReset;
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused)
		processQEmuRequest();
}


/**
 * emuFrameDone(): Emulation thread is finished rendering a frame.
 * @param wasFastFrame The frame was rendered "fast", i.e. no VDP updates.
 */
void EmuManager::emuFrameDone(bool wasFastFrame)
{
	if (!wasFastFrame)
		m_frames++;
	
	if (m_lastTime < 0.001)
		m_lastTime = LibGens::Timing::GetTimeD();
	else
	{
		double thisTime = LibGens::Timing::GetTimeD();
		if ((thisTime - m_lastTime) >= 0.250)
		{
			// Push the current fps.
			// (Updated four times per second.)
			double fps = ((double)m_frames / (thisTime - m_lastTime));
			emit updateFps(fps);
			
			// Reset the timer and frame counter.
			m_lastTime = thisTime;
			m_frames = 0;
		}
	}
	
	// Check for SRam/EEPRom autosave.
	// TODO: Frames elapsed.
	LibGens::EmuMD::AutoSaveData(m_rom, 1);
	
	// Check for requests in the emulation queue.
	if (!m_qEmuRequest.isEmpty())
		processQEmuRequest();
	
	// Update the GensQGLWidget.
	if (!wasFastFrame)
		emit updateVideo();
	
	if (m_paused)
		return;
	
	/** Auto Frame Skip **/
	// Check if we should do a fast frame.
	// TODO: Audio stutters a bit if the video drops below 60.0 fps.
	m_audio->wpSegWait();	// Wait for the buffer to empty.
	m_audio->write();	// Write audio.
	bool doFastFrame = !m_audio->isBufferEmpty();
	
	// Tell the emulation thread that we're ready for another frame.
	if (gqt4_emuThread)
		gqt4_emuThread->resume(doFastFrame);
}

}
