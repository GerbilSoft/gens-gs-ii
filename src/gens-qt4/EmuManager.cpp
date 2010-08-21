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

// Controller devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"
#include "libgens/IO/IoTeamplayer.hpp"
#include "libgens/IO/Io4WPMaster.hpp"
#include "libgens/IO/Io4WPSlave.hpp"

// Qt includes.
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtCore/QLocale>

// Text translation macro.
#define TR(text) \
	QApplication::translate("EmuManager", (text), NULL, QApplication::UnicodeUTF8)

namespace GensQt4
{

EmuManager::EmuManager()
{
	// Reset timing information.
	m_lastTime = 0.0;
	m_frames = 0;
	
	// No ROM is loaded at startup.
	m_rom = NULL;
	
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
		// TODO: Add "Zip File Selection Dialog".
		// For now, always select the first file.
		m_rom->select_z_entry(m_rom->get_z_entry_list());
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
	
	// TODO: Clear the screen, start the idle animation, etc.
	
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
	
	// TODO: Do the controller change immediately if
	// a ROM is running and the system is paused.
	if (!m_rom)
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
	
	// TODO: Do the audio rate change immediately if
	// a ROM is running and the system is paused.
	if (!m_rom)
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
	
	// TODO: Do the audio rate change immediately if
	// a ROM is running and the system is paused.
	if (!m_rom)
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
	// TODO: Frames elapsed; autosave on pause.
	LibGens::EmuMD::AutoSaveData(m_rom, 1);
	
	// Check for controller changes.
	if (!m_qEmuRequest.isEmpty())
		processQEmuRequest();
	
	// Update the GensQGLWidget.
	if (!wasFastFrame)
		emit updateVideo();
	
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


/**
 * processQEmuRequest(): Process the Emulation Request queue.
 */
void EmuManager::processQEmuRequest(void)
{
	while (!m_qEmuRequest.isEmpty())
	{
		EmuRequest_t rq = m_qEmuRequest.dequeue();
		
		switch (rq.rqType)
		{
			case EmuRequest_t::RQT_CTRLCHANGE:
				// Controller Change.
				doCtrlChange(rq.ctrlChange.port, rq.ctrlChange.ctrlType);
				break;
			
			case EmuRequest_t::RQT_SCREENSHOT:
				// Screenshot.
				doScreenShot();
				break;
			
			case EmuRequest_t::RQT_AUDIO_RATE:
				// Audio sampling rate.
				doAudioRate(rq.audioRate);
				break;
			
			case EmuRequest_t::RQT_AUDIO_STEREO:
				// Stereo / Mono.
				doAudioStereo(rq.audioStereo);
				break;
			
			case EmuRequest_t::RQT_UNKNOWN:
			default:
				// Unknown emulation request.
				break;
		}
	}
}


/** Emulation Request Queue functions. **/


/**
 * doCtrlChange(): Do a controller change.
 * Called from processQEmuRequest().
 * @param port Controller port. (0, 1, 2) [TODO: Teamplayer/4WP]
 * @param type New controller type.
 */
void EmuManager::doCtrlChange(int port, LibGens::IoBase::IoType type)
{
	// TODO: Teamplayer/4WP support.
	LibGens::IoBase **prevDevPtr = NULL;
	switch (port)
	{
		case 0:		prevDevPtr = &LibGens::EmuMD::m_port1; break;
		case 1:		prevDevPtr = &LibGens::EmuMD::m_port2; break;
		case 2:		prevDevPtr = &LibGens::EmuMD::m_portE; break;
		default:	break;
	}
	
	if (!(*prevDevPtr))
		return;
	
	if ((*prevDevPtr)->devType() == type)
	{
		// There's no point in changing a controller
		// to the same type that it already is...
		return;
	}
	
	LibGens::IoBase *dev = NULL;
	switch (type)
	{
		case LibGens::IoBase::IOT_NONE:
			// No controller.
			dev = new LibGens::IoBase(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_3BTN:
			// 3-button controller.
			dev = new LibGens::Io3Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_6BTN:
			// 6-button controller.
			dev = new LibGens::Io6Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_2BTN:
			// 2-button controller.
			dev = new LibGens::Io2Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_MEGA_MOUSE:
			// Sega Mega Mouse.
			dev = new LibGens::IoMegaMouse(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_TEAMPLAYER:
			// Sega Teamplayer.
			dev = new LibGens::IoTeamplayer(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_4WP_MASTER:
		case LibGens::IoBase::IOT_4WP_SLAVE:
		{
			// EA 4-Way Play. (TODO)
#if 0
			LibGens::Io4WPMaster *master = new LibGens::Io4WPMaster(LibGens::EmuMD::m_port2);
			LibGens::Io4WPSlave *slave = new LibGens::Io4WPSlave(LibGens::EmuMD::m_port1);
			master->setSlaveDevice(slave);
			port2 = master;
			port1 = slave;
			m_vBackend->osd_printf(1500, "Port 1 set to EA 4-WAY PLAY (Slave).");
			m_vBackend->osd_printf(1500, "Port 2 set to EA 4-WAY PLAY (Master).");
#endif
			break;
		}
		
		default:
			break;
	}
	
	if (!dev)
	{
		// Unknown device. TODO
		return;
	}
			
	// Set the device.
	delete *prevDevPtr;
	*prevDevPtr = dev;
	
	// Print a message on the OSD.
	QString osdMsg = TR("Port %1 set to %2.");
	osdMsg = osdMsg.arg(port + 1);	// TODO: Use "E" for Port 3.
	osdMsg = osdMsg.arg(dev->devName());
	
	emit osdPrintMsg(1500, osdMsg);
	
	// Make sure there's no dangling 4WP master/slave device.
	// TODO: Do this after the emulation request queue is cleared.
#if 0
	LibGens::IoBase::IoType p1_type = LibGens::EmuMD::m_port1->devType();
	LibGens::IoBase::IoType p2_type = LibGens::EmuMD::m_port2->devType();
	bool p1_is4WP = (p1_type == LibGens::IoBase::IOT_4WP_MASTER || p1_type == LibGens::IoBase::IOT_4WP_SLAVE);
	bool p2_is4WP = (p2_type == LibGens::IoBase::IOT_4WP_MASTER || p2_type == LibGens::IoBase::IOT_4WP_SLAVE);
	
	if (p1_is4WP ^ p2_is4WP)
	{
		// Dangling 4WP device.
		if (p1_is4WP)
		{
			port1 = new LibGens::IoBase(LibGens::EmuMD::m_port1);
			delete LibGens::EmuMD::m_port1;
			LibGens::EmuMD::m_port1 = port1;
			m_vBackend->osd_printf(1500, "Port 1 set to NONE.");
		}
		else
		{
			port2 = new LibGens::IoBase(LibGens::EmuMD::m_port2);
			delete LibGens::EmuMD::m_port2;
			LibGens::EmuMD::m_port2 = port2;
			m_vBackend->osd_printf(1500, "Port 2 set to NONE.");
		}
	}
#endif
}


/**
 * doScreenShot(): Do a screenshot.
 * Called from processQEmuRequest().
 */
void EmuManager::doScreenShot(void)
{
	// TODO: Save the screenshot in a designated screenshots directory.
	// For now, it'll save in the gens-qt4 directory.
	QString romFilename = QString::fromUtf8(m_rom->filename());
	
	// Get the filename portion.
	// TODO: Do this in the Rom class?
	int dirSep = romFilename.lastIndexOf(QDir::separator());
	if (dirSep != -1)
		romFilename.remove(0, dirSep+1);
	
	// Remove the file extension.
	// TODO: Do this in the Rom class?
	int extSep = romFilename.lastIndexOf('.');
	if (extSep != -1)
		romFilename.remove(extSep, (romFilename.size() - extSep));
	
	// Add the current directory, number, and .png extension.
	// TODO: Use a designated screenshots directory.
	// TODO: Enumerate QImageWriter for supported image formats.
	const QString scrFilenamePrefix = QDir::currentPath() + QChar('/') + romFilename;
	const QString scrFilenameSuffix = ".png";
	QString scrFilename;
	int scrNumber = -1;
	do
	{
		// TODO: Figure out how to optimize this!
		scrNumber++;
		scrFilename = scrFilenamePrefix + QChar('_') +
				QString::number(scrNumber).rightJustified(3, '0') +
				scrFilenameSuffix;
	} while (QFile::exists(scrFilename));
	
	// Create the QImage.
	const uint8_t *start;
	const int startY = ((240 - LibGens::VdpIo::GetVPix()) / 2);
	const int startX = (8 + LibGens::VdpIo::GetHPixBegin());
	const int startPx = (TAB336[startY] + startX);
	int bytesPerLine;
	QImage::Format imgFormat;
	
	if (LibGens::VdpRend::m_palette.bpp() == LibGens::VdpPalette::BPP_32)
	{
		start = (const uint8_t*)(&LibGens::VdpRend::MD_Screen.u32[startPx]);
		bytesPerLine = (336 * sizeof(uint32_t));
		imgFormat = QImage::Format_RGB32;
	}
	else
	{
		start = (const uint8_t*)(&LibGens::VdpRend::MD_Screen.u16[startPx]);
		bytesPerLine = (336 * sizeof(uint16_t));
		if (LibGens::VdpRend::m_palette.bpp() == LibGens::VdpPalette::BPP_16)
			imgFormat = QImage::Format_RGB16;
		else
			imgFormat = QImage::Format_RGB555;
	}
	
	// TODO: Check for errors.
	// TODO: Store timestamp, ROM filename, etc.
	QImage img(start, LibGens::VdpIo::GetHPix(), LibGens::VdpIo::GetVPix(),
		   bytesPerLine, imgFormat);
	QImageWriter imgWriter(scrFilename, "png");
	imgWriter.write(img);
	
	// Print a message on the OSD.
	QString osdMsg = TR("Screenshot %1 saved.");
	osdMsg = osdMsg.arg(scrNumber);
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doAudioRate(): Set the audio sampling rate.
 * @param newRate New audio sampling rate.
 */
void EmuManager::doAudioRate(int newRate)
{
	if (m_audio->rate() == newRate)
		return;
	m_audio->setRate(newRate);
	
	// Print a message on the OSD.
	QString osdMsg = TR("Audio sampling rate set to %1 Hz.");
	
	// Format the sampling rate using the system locale.
	QLocale sysLocale = QLocale::system();
	osdMsg = osdMsg.arg(sysLocale.toString(newRate));
	
	// Emit the signal.
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doAudioStereo(): Set stereo or mono.
 * @param newStereo True for stereo; false for mono.
 */
void EmuManager::doAudioStereo(bool newStereo)
{
	if (m_audio->isStereo() == newStereo)
		return;
	m_audio->setStereo(newStereo);
	
	// Print a message on the OSD.
	QString osdMsg = TR("Audio set to %1.");
	osdMsg = osdMsg.arg(newStereo ? "Stereo" : "Mono");
	emit osdPrintMsg(1500, osdMsg);
}

}
