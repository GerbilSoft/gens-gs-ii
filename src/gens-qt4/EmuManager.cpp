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

#include <config.h>

#include "EmuManager.hpp"
#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/Util/Timing.hpp"
#include "libgens/MD/EmuMD.hpp"

// LibGens Sound Manager.
// Needed for LibGens::SoundMgr::MAX_SAMPLING_RATE.
#include "libgens/sound/SoundMgr.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

// LibGens video includes.
#include "libgens/MD/VdpPalette.hpp"
#include "libgens/MD/VdpRend.hpp"
#include "libgens/MD/VdpIo.hpp"
#include "libgens/MD/TAB336.h"

// M68K_Mem.hpp has SysRegion.
#include "libgens/cpu/M68K_Mem.hpp"

// Multi-File Archive Selection Dialog.
#include "ZipSelectDialog.hpp"

// libzomg. Needed for savestate preview images.
#include "libzomg/Zomg.hpp"

// Qt includes.
#include <QtCore/QBuffer>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QImageReader>

// Qt key handler.
// TODO: Make a class for handling non-controller input, e.g. Reset.
#include "Input/KeyHandlerQt.hpp"


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
	// Initialize timing information.
	m_lastTime = 0.0;
	m_lastTime_fps = 0.0;
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
 * @param parent Parent window for the modal dialog box.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(QWidget *parent)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"
	
	// TODO: Set the default filename.
	QString filename = QFileDialog::getOpenFileName(parent,
			tr("Open ROM"),		// Dialog title
			QString(),		// Default filename.
			tr("Sega Genesis ROM images") +
			QString::fromLatin1(
				" (*.bin *.gen *.md *.smd"
#ifdef HAVE_ZLIB
				ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
				LZMA_EXT
#endif /* HAVE_LZMA */
				RAR_EXT
				");;"
				) +
#if 0
			tr("Sega Genesis / 32X ROMs; Sega CD disc images") +
			"(*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT ");;" +
#endif
			tr("All Files") + QString::fromLatin1("(*.*)"));
	
	if (filename.isEmpty())
		return 1;
	
	// Open the selected ROM file.
	return openRom(filename);
}


/**
 * openRom(): Open a ROM file.
 * Opens a ROM file using the given filename.
 * @param filename Filename of the ROM file to open.
 * TODO: Add parameter for z_filename.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(const QString& filename)
{
	if (gqt4_emuThread || m_rom)
	{
		// Close the ROM first.
		closeRom();
		
		// HACK: Set a QTimer for 100 ms to actually load the ROM to make sure
		// the emulation thread is shut down properly.
		// If we don't do that, then loading savestates causes
		// video corruption due to a timing mismatch.
		// TODO: Figure out how to fix this.
		// TODO: Proper return code.
		openRom_int_tmr_filename = filename;
		QTimer::singleShot(100, this, SLOT(sl_openRom_int()));
		return 0;
	}
	
	// ROM isn't running. Open the ROM directly.
	return openRom_int(filename);
}


/**
 * openRom_int(): Open a ROM file. (Internal function.)
 * @param filename Filename of the ROM file to open.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom_int(const QString& filename)
{
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
	
	// Create a new MD emulation context.
	delete gqt4_emuContext;
	gqt4_emuContext = new LibGens::EmuMD(m_rom);
	m_rom->close();	// TODO: Let EmuMD handle this...
	
	if (!gqt4_emuContext->isRomOpened())
	{
		// Error loading the ROM image in EmuMD.
		// TODO: EmuMD error code constants.
		// TODO: Show an error message.
		fprintf(stderr, "Error: Initialization of gqt4_emuContext failed. (TODO: Error code.)\n");
		delete gqt4_emuContext;
		gqt4_emuContext = NULL;
		delete m_rom;
		m_rom = NULL;
		return 5;
	}
	
	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	// TODO: Use gqt4_emuContext instead?
	
	// Open audio.
	m_audio->open();
	
	// Initialize timing information.
	m_lastTime = LibGens::Timing::GetTimeD();
	m_lastTime_fps = m_lastTime;
	m_frames = 0;
	
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
		// Disconnect the emuThread's signals.
		gqt4_emuThread->disconnect();
		
		// Stop and delete the emulation thread.
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	if (gqt4_emuContext)
	{
		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		gqt4_emuContext->saveData();
		
		// Delete the emulation context.
		delete gqt4_emuContext;
		gqt4_emuContext = NULL;
		
		// Delete the Rom instance.
		// TODO: Handle this in gqt4_emuContext.
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
				return tr("Genesis");
			else
				return tr("Mega Drive");
		
		case LibGens::Rom::MDP_SYSTEM_MCD:
			if (region.region() == LibGens::SysRegion::REGION_US_NTSC)
				return tr("Sega CD");
			else
				return tr("Mega CD");
		
		case LibGens::Rom::MDP_SYSTEM_32X:
			if (region.isPal())
				return tr("32X (PAL)");
			else
				return tr("32X (NTSC)");
		
		case LibGens::Rom::MDP_SYSTEM_MCD32X:
			if (region.region() == LibGens::SysRegion::REGION_US_NTSC)
				return tr("Sega CD 32X");
			else
				return tr("Mega CD 32X");
		
		case LibGens::Rom::MDP_SYSTEM_SMS:
			return tr("Master System");
		
		case LibGens::Rom::MDP_SYSTEM_GG:
			return tr("Game Gear");
		
		case LibGens::Rom::MDP_SYSTEM_SG1000:
			return tr("SG-1000");
		
		case LibGens::Rom::MDP_SYSTEM_PICO:
			return tr("Pico");
		
		default:
			return tr("Unknown");
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
 * getSaveStateFilename(): Get the savestate filename.
 * TODO: Move savestate code to another file?
 * @return Savestate filename, or empty string if no ROM is loaded.
 */
QString EmuManager::getSaveStateFilename(void)
{
	if (!m_rom)
		return QString();
	
	// TODO: Move to another function?
	QString filename = QString::fromLatin1("%1.%2.zomg");
	filename = filename.arg(QString::fromUtf8(m_rom->filenameBaseNoExt()));
	filename = filename.arg(m_saveSlot);
	return filename;
}


/**
 * saveState(): Save the current emulation state.
 */
void EmuManager::saveState(void)
{
	if (!m_rom)
		return;
	
	// Get the savestate filename.
	QString filename = getSaveStateFilename();
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_STATE;
	rq.saveState.filename = strdup(filename.toUtf8().constData());
	rq.saveState.saveSlot = m_saveSlot;
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused)
		processQEmuRequest();
}

/**
 * loadState(): Load the emulation state from a file.
 */
void EmuManager::loadState(void)
{
	if (!m_rom)
		return;
	
	// Get the savestate filename.
	QString filename = getSaveStateFilename();
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_LOAD_STATE;
	rq.saveState.filename = strdup(filename.toUtf8().constData());
	rq.saveState.saveSlot = m_saveSlot;
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
	
	if (m_rom)
	{
		// ROM is loaded.
		QString osdMsg = tr("Save Slot %1 [%2]").arg(slotNum);
		
		// Check if the file exists.
		QString filename = getSaveStateFilename();
		if (QFile::exists(filename))
		{
			// Savestate exists.
			// TODO: Load the preview image.
			osdMsg = osdMsg.arg(tr("OCCUPIED"));
			
			// Check if the savestate has a preview image.
			LibZomg::Zomg zomg(filename.toUtf8().constData(), LibZomg::Zomg::ZOMG_LOAD);
			if (zomg.getPreviewSize() == 0)
			{
				// No preview image.
				zomg.close();
				emit osdPrintMsg(1500, osdMsg);
				emit osdShowPreview(0, QImage());
				return;
			}
			
			// Preview image found.
			QByteArray img_ByteArray;
			img_ByteArray.resize(zomg.getPreviewSize());
			int ret = zomg.loadPreview(img_ByteArray.data(), img_ByteArray.size());
			zomg.close();
			if (ret != 0)
			{
				// Error loading the preview image.
				emit osdPrintMsg(1500, osdMsg);
				emit osdShowPreview(0, QImage());
				return;
			}
			
			// Preview image loaded from the ZOMG file.
			
			// Convert the preview image to a QImage.
			QBuffer imgBuf(&img_ByteArray);
			imgBuf.open(QIODevice::ReadOnly);
			QImageReader imgReader(&imgBuf, "png");
			QImage img = imgReader.read();
			if (img.isNull())
			{
				// Error reading the preview image.
				emit osdPrintMsg(1500, osdMsg);
				emit osdShowPreview(0, QImage());
				return;
			}
			
			// Preview image converted to QImage.
			emit osdPrintMsg(1500, osdMsg);
			emit osdShowPreview(1500, img);
		}
		else
		{
			// Savestate doesn't exist.
			osdMsg = osdMsg.arg(tr("EMPTY"));
			emit osdPrintMsg(1500, osdMsg);
			emit osdShowPreview(0, QImage());
		}
	}
	else
	{
		// ROM is not loaded.
		QString osdMsg = tr("Save Slot %1 selected.").arg(slotNum);
		emit osdPrintMsg(1500, osdMsg);
		emit osdShowPreview(0, QImage());
	}
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
	// Make sure the emulation thread is still running.
	if (!gqt4_emuThread || gqt4_emuThread->isStopRequested())
		return;
	
	if (!wasFastFrame)
		m_frames++;
	
	double timeDiff;
	double thisTime = LibGens::Timing::GetTimeD();
	
	if (m_lastTime < 0.001)
	{
		m_lastTime = thisTime;
		m_lastTime_fps = m_lastTime;
		timeDiff = 0.0;
	}
	else
	{
		// Get the time difference.
		timeDiff = (thisTime - m_lastTime);
		
		// Check the FPS counter.
		double timeDiff_fps = (thisTime - m_lastTime_fps);
		if (timeDiff_fps >= 0.250)
		{
			// Push the current fps.
			// (Updated four times per second.)
			double fps = ((double)m_frames / timeDiff_fps);
			emit updateFps(fps);
			
			// Reset the timer and frame counter.
			m_lastTime_fps = thisTime;
			m_frames = 0;
		}
	}
	
	// Check for SRam/EEPRom autosave.
	// TODO: Frames elapsed.
	gqt4_emuContext->autoSaveData(1);
	
	// Check for requests in the emulation queue.
	if (!m_qEmuRequest.isEmpty())
		processQEmuRequest();
	
	// Update the GensQGLWidget.
	if (!wasFastFrame)
		emit updateVideo();
	
	if (m_paused)
		return;
	
	/** Auto Frame Skip **/
	// TODO: Figure out how to properly implement the old Gens method of synchronizing to audio.
#if 0
	// Check if we should do a fast frame.
	// TODO: Audio stutters a bit if the video drops below 60.0 fps.
	m_audio->wpSegWait();	// Wait for the buffer to empty.
	m_audio->write();	// Write audio.
	bool doFastFrame = !m_audio->isBufferEmpty();
#else
	// TODO: Remove the ring buffer and just use the classic SDL-esque method.
	m_audio->write();	// Write audio.
#endif
	
	// Check if we're higher or lower than the required framerate.
	bool doFastFrame = false;
	const double frameRate = (1.0 / (LibGens::M68K_Mem::ms_Region.isPal() ? 50.0 : 60.0));
	const double threshold = 0.001;
	if (timeDiff > (frameRate + threshold))
	{
		// Lower than the required framerate.
		// Do a fast frame.
		//printf("doing fast frame; ");
		doFastFrame = true;
	}
	else if (timeDiff < (frameRate - threshold))
	{
		// Higher than the required framerate.
		// Wait for the required amount of time.
		do
		{
			thisTime = LibGens::Timing::GetTimeD();
			timeDiff = (thisTime - m_lastTime);
		} while (timeDiff < frameRate);
		
		// TODO: This causes some issues...
		if (timeDiff > (frameRate + threshold))
			doFastFrame = true;
	}
	// Update the last time value.
	m_lastTime = thisTime;
	
	// Tell the emulation thread that we're ready for another frame.
	if (gqt4_emuThread)
		gqt4_emuThread->resume(doFastFrame);
}

}
