/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * EmuManager.cpp: Emulation manager.                                         *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2011 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include <gens-qt4/config.gens-qt4.h>

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
#include "libgens/Vdp/Vdp.hpp"
#include "libgens/Vdp/VdpPalette.hpp"

// M68K_Mem.hpp has SysRegion.
#include "libgens/cpu/M68K_Mem.hpp"

// Multi-File Archive Selection Dialog.
#include "ZipSelectDialog.hpp"

// libzomg. Needed for savestate preview images.
#include "libzomg/Zomg.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// Qt key handler.
#include "Input/KeyHandlerQt.hpp"

// Filename case-sensitivity.
// TODO: Determine this from QAbstractFileEngine instead of hard-coding it!
// TODO: Share this with the RecentRoms class.
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
#define FILENAME_CASE_SENSITIVE Qt::CaseInsensitive
#else
#define FILENAME_CASE_SENSITIVE Qt::CaseSensitive
#endif


namespace GensQt4
{

EmuManager::EmuManager(QObject *parent, VBackend *vBackend)
	: QObject(parent)
	, m_vBackend(vBackend)
	, m_romClosedFb(nullptr)
{
	// Initialize timing information.
	m_lastTime = 0.0;
	m_lastTime_fps = 0.0;
	m_frames = 0;

	// No ROM is loaded at startup.
	m_rom = nullptr;
	m_paused.data = 0;

	// If a video backend is specified, connect its destroyed() signal.
	if (m_vBackend) {
		connect(m_vBackend, SIGNAL(destroyed(QObject*)),
			this, SLOT(vBackend_destroyed(QObject*)));
	}

	// Save slot.
	// TODO: Move saveSlot validation intn ConfigStore
	m_saveSlot = gqt4_cfg->getInt(QLatin1String("Savestates/saveSlot")) % 10;

	// TODO: Load initial VdpPalette settings.

	// Create the Audio Backend.
	// TODO: Allow selection of all available audio backend classes.
	// NOTE: Audio backends are NOT QWidgets!
	m_audio = new GensPortAudio();

	// Configuration settings.
	gqt4_cfg->registerChangeNotification(QLatin1String("Savestates/saveSlot"),
					this, SLOT(saveSlot_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("autoFixChecksum"),
					this, SLOT(autoFixChecksum_changed_slot(QVariant)));

	// Graphics settings.
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/interlacedMode"),
					this, SLOT(interlacedMode_changed_slot(QVariant)));

	// VDP settings.
	gqt4_cfg->registerChangeNotification(QLatin1String("VDP/borderColorEmulation"),
					this, SLOT(borderColorEmulation_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("VDP/ntscV30Rolling"),
					this, SLOT(ntscV30Rolling_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("VDP/spriteLimits"),
					this, SLOT(spriteLimits_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("VDP/zeroLengthDMA"),
					this, SLOT(zeroLengthDMA_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("VDP/vscrollBug"),
					this, SLOT(vscrollBug_changed_slot(QVariant)));

	// Region code settings.
	gqt4_cfg->registerChangeNotification(QLatin1String("System/regionCode"),
					this, SLOT(regionCode_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("System/regionCodeOrder"),
					this, SLOT(regionCodeOrder_changed_slot(QVariant)));

	// Emulation options. (Options menu)
	gqt4_cfg->registerChangeNotification(QLatin1String("Options/enableSRam"),
					this, SLOT(enableSRam_changed_slot(QVariant)));
}

EmuManager::~EmuManager()
{
	// Delete the audio backend.
	m_audio->close();
	delete m_audio;
	m_audio = nullptr;
	
	// Delete the ROM.
	// TODO
	
	// TODO: Do we really need to clear this?
	m_paused.data = 0;
	
	// Unreference the last-closed ROM framebuffer.
	if (m_romClosedFb)
		m_romClosedFb->unref();
}


/**
 * Open a ROM file.
 * Prompts the user to select a ROM file, then opens it.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(void)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"

	// TODO: Set the default filename.
	QString filename = QFileDialog::getOpenFileName(nullptr,
			tr("Open ROM"),		// Dialog title
			QString(),		// Default filename.
			tr("Sega Genesis ROM images") +
			QLatin1String(
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
			tr("All Files") + QLatin1String(" (*.*)"));

	if (filename.isEmpty())
		return -1;

	// Convert the filename to native separators.
	filename = QDir::toNativeSeparators(filename);

	// Open the ROM file.
	return openRom(filename);
}


/**
 * Open a ROM file.
 * @param filename ROM filename. (Must have native separators!)
 * @param z_filename Internal archive filename. (If not specified, user will be prompted.)
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(QString filename, QString z_filename)
{
	// Open the file using the LibGens::Rom class.
	// TODO: This won't work for KIO...
	LibGens::Rom *rom = new LibGens::Rom(filename.toUtf8().constData());
	if (!rom->isOpen()) {
		// Couldn't open the ROM file.
		fprintf(stderr, "Error opening ROM file. (TODO: Get error information.)\n");
		delete rom;
		return -2;
	}

	// Check if this is a multi-file ROM archive.
	if (rom->isMultiFile()) {
		// Multi-file ROM archive.
		const mdp_z_entry_t *z_entry;

		if (z_filename.isEmpty()) {
			// Prompt the user to select a file.
			ZipSelectDialog *zipsel = new ZipSelectDialog();
			zipsel->setFileList(rom->get_z_entry_list());
			int ret = zipsel->exec();
			if (ret != QDialog::Accepted || zipsel->selectedFile() == nullptr) {
				// Dialog was rejected.
				delete rom;
				delete zipsel;
				return -3;
			}

			// Get the selected file.
			z_entry = zipsel->selectedFile();
			delete zipsel;
		} else {
			// Search for the specified filename in the z_entry list.
			for (z_entry = rom->get_z_entry_list();
			     z_entry; z_entry = z_entry->next)
			{
				const QString z_entry_filename = QString::fromUtf8(z_entry->filename);
				if (!z_filename.compare(z_entry_filename, FILENAME_CASE_SENSITIVE))
					break;
			}
			
			if (!z_entry) {
				// z_filename not found.
				// TODO: Show an error message.
				fprintf(stderr, "Error opening ROM file: z_filename not found.\n");
				delete rom;
				return -4;
			}
		}

		// Get the selected file.
		z_filename = QString::fromUtf8(z_entry->filename);
		rom->select_z_entry(z_entry);
	}

	// Add the ROM file to the Recent ROMs list.
	// TODO: Don't do this if the ROM couldn't be loaded.
	gqt4_cfg->recentRomsUpdate(filename, z_filename, rom->sysId());

	// Load the selected ROM file.
	return loadRom(rom);
}


/**
 * loadRom(): Load a ROM file.
 * Loads a ROM file previously opened via LibGens::Rom().
 * @param rom [in] Previously opened ROM object.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::loadRom(LibGens::Rom *rom)
{
	if (gqt4_emuThread || m_rom)
	{
		// Close the ROM first.
		// Don't emit a stateChanged() signal, since
		// we're opening a ROM immediately afterwards.
		closeRom(false);
		
		// HACK: Set a QTimer for 100 ms to actually load the ROM to make sure
		// the emulation thread is shut down properly.
		// If we don't do that, then loading savestates causes
		// video corruption due to a timing mismatch.
		// TODO: Figure out how to fix this.
		// TODO: Proper return code.
		m_loadRom_int_tmr_rom = rom;
		QTimer::singleShot(100, this, SLOT(sl_loadRom_int()));
		return 0;
	}
	
	// ROM isn't running. Open the ROM directly.
	return loadRom_int(rom);
}


/**
 * Load a ROM file. (Internal function.)
 * Loads a ROM file previously opened via LibGens::Rom().
 * @param rom [in] Previously opened ROM object.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::loadRom_int(LibGens::Rom *rom)
{
	if (m_rom)
		return -1;

	// Check if this is a multi-file ROM archive.
	if (rom->isMultiFile() && !rom->isRomSelected()) {
		// Multi-file ROM archive, but a ROM hasn't been selected.
		return -2;
	}

	// Make sure the ROM is supported.
	const QChar chrBullet(0x2022);  // U+2022: BULLET
	const QChar chrNewline(L'\n');
	const QChar chrSpace(L' ');

	// Check the system ID.
	if (rom->sysId() != LibGens::Rom::MDP_SYSTEM_MD) {
		// Only MD ROM images are supported.
		const LibGens::Rom::MDP_SYSTEM_ID errSysId = rom->sysId();
		delete rom;

		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what system the ROM is for...)
		QMessageBox::critical(nullptr,
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error title)
				tr("Unsupported System"),
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is designed for a system that"
				   " is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what system the ROM image is for.
				tr("Selected ROM's system: %1").arg(SysName_l(errSysId)) +
				chrNewline + chrNewline +
				//: List of systems that Gens/GS II currently supports.
				tr("Supported systems:") + chrNewline +
				chrBullet + chrSpace + SysName_l(LibGens::Rom::MDP_SYSTEM_MD)
				);

		return 3;
	}

	// Check the ROM format.
	if (rom->romFormat() != LibGens::Rom::RFMT_BINARY) {
		// Only binary ROM images are supported.r
		LibGens::Rom::RomFormat errRomFormat = rom->romFormat();
		delete rom;

		// Get the ROM format.
		QString sRomFormat = RomFormat(errRomFormat);
		if (sRomFormat.isEmpty()) {
			//: Unknown ROM format. (EmuManager::RomFormat() returned an empty string.)
			sRomFormat = tr("(unknown)", "rom-format");
		}

		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what format the ROM was in...)
		QMessageBox::critical(nullptr,
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error title)
				tr("Unsupported ROM Format"),
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is in a format that is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what format the ROM image is in.
				tr("Selected ROM image format: %1").arg(sRomFormat) +
				chrNewline + chrNewline +
				//: List of ROM formats that Gens/GS II currently supports.
				tr("Supported ROM formats:") + chrNewline +
				chrBullet + chrSpace + RomFormat(LibGens::Rom::RFMT_BINARY)
				);

		return 4;
	}

	// Determine the system region code.
	const LibGens::SysVersion::RegionCode_t cfg_region =
				(LibGens::SysVersion::RegionCode_t)gqt4_cfg->getInt(QLatin1String("System/regionCode"));

	const LibGens::SysVersion::RegionCode_t lg_region = GetLgRegionCode(
				cfg_region, rom->regionCode(),
				(uint16_t)gqt4_cfg->getUInt(QLatin1String("System/regionCodeOrder")));

	if (cfg_region == LibGens::SysVersion::REGION_AUTO) {
		// Print the auto-detected region.
		const QString detect_str = LgRegionCodeStr(lg_region);
		if (!detect_str.isEmpty()) {
			//: OSD message indicating the auto-detected ROM region.
			const QString auto_str = tr("ROM region detected as %1.", "osd");
			emit osdPrintMsg(1500, auto_str.arg(detect_str));
		}
	}

	// Autofix Checksum.
	// NOTE: This must be set *before* creating the emulation context!
	// Otherwise, it won't work until Hard Reset.
	LibGens::EmuContext::SetAutoFixChecksum(
			gqt4_cfg->get(QLatin1String("autoFixChecksum")).toBool());

	// Create a new MD emulation context.
	// FIXME: Delete gqt4_emuContext after VBackend is finished using it. (MEMORY LEAK)
	m_vBackend->setEmuContext(nullptr);
	delete gqt4_emuContext;
	gqt4_emuContext = new LibGens::EmuMD(rom, lg_region);
	m_vBackend->setEmuContext(gqt4_emuContext);
	rom->close();	// TODO: Let EmuMD handle this...

	if (!gqt4_emuContext->isRomOpened()) {
		// Error loading the ROM image in EmuMD.
		// TODO: EmuMD error code constants.
		// TODO: Show an error message.
		fprintf(stderr, "Error: Initialization of gqt4_emuContext failed. (TODO: Error code.)\n");
		m_vBackend->setEmuContext(nullptr);
		delete gqt4_emuContext;
		gqt4_emuContext = nullptr;
		delete rom;
		return 5;
	}

	// Save the Rom class pointer as m_rom.
	m_rom = rom;

	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	// TODO: Use gqt4_emuContext instead?

	// Open audio.
	m_audio->open();

	// Initialize timing information.
	m_lastTime = LibGens::Timing::GetTimeD();
	m_lastTime_fps = m_lastTime;
	m_frames = 0;

	// Initialize controllers.
	gqt4_cfg->m_ctrlConfig->updateIoManager(gqt4_emuContext->m_ioManager);
	gqt4_cfg->m_ctrlConfig->clearDirty();

	// Set the EmuContext settings.
	// TODO: Load these in EmuContext directly?
	gqt4_emuContext->setSaveDataEnable(gqt4_cfg->get(QLatin1String("Options/enableSRam")).toBool());

	// TODO: The following should be set in the specific EmuContext.

	// Initialize the graphics settings.
	LibGens::Vdp::VdpEmuOptions.intRendMode =
			(LibGens::VdpTypes::IntRend_Mode_t)gqt4_cfg->getInt(QLatin1String("Graphics/interlacedMode"));

	// Initialize the VDP settings.
	LibGens::Vdp::VdpEmuOptions.borderColorEmulation =
			gqt4_cfg->get(QLatin1String("VDP/borderColorEmulation")).toBool();
	LibGens::Vdp::VdpEmuOptions.ntscV30Rolling =
			gqt4_cfg->get(QLatin1String("VDP/ntscV30Rolling")).toBool();
	LibGens::Vdp::VdpEmuOptions.spriteLimits =
			gqt4_cfg->get(QLatin1String("VDP/spriteLimits")).toBool();
	LibGens::Vdp::VdpEmuOptions.zeroLengthDMA =
			gqt4_cfg->get(QLatin1String("VDP/zeroLengthDMA")).toBool();
	LibGens::Vdp::VdpEmuOptions.vscrollBug =
			gqt4_cfg->get(QLatin1String("VDP/vscrollBug")).toBool();

	// Start the emulation thread.
	m_paused.data = 0;
	gqt4_emuThread = new EmuThread();
	QObject::connect(gqt4_emuThread, SIGNAL(frameDone(bool)),
			 this, SLOT(emuFrameDone(bool)));
	gqt4_emuThread->start();

	// Update the Gens title.
	emit stateChanged();
	return 0;
}


/**
 * Determine the LibGens region code to use.
 * @param confRegionCode Current GensConfig region code.
 * @param mdHexRegionCode ROM region code, in MD hex format.
 * @param regionCodeOrder Region code order for auto-detection. (MSN == highest priority)
 * @return LibGens region code to use.
 */
LibGens::SysVersion::RegionCode_t EmuManager::GetLgRegionCode(
		LibGens::SysVersion::RegionCode_t confRegionCode,
		int mdHexRegionCode, uint16_t regionCodeOrder)
{
	if (confRegionCode >= LibGens::SysVersion::REGION_JP_NTSC &&
	    confRegionCode <= LibGens::SysVersion::REGION_EU_PAL)
	{
		// Valid regoin code.
		return confRegionCode;
	}
	
	// Attempt to auto-detect the region from the ROM image.
	int regionMatch = 0;
	int orderTmp = regionCodeOrder;
	for (int i = 0; i < 4; i++, orderTmp <<= 4)
	{
		int orderN = ((orderTmp >> 12) & 0xF);
		if (mdHexRegionCode & orderN)
		{
			// Found a match.
			regionMatch = orderN;
			break;
		}
	}
	
	if (regionMatch == 0)
	{
		// No region matched.
		// Use the highest-priority region.
		regionMatch = ((regionCodeOrder >> 12) & 0xF);
	}
	
	switch (regionMatch & 0xF)
	{
		default:
		case 0x4:	return LibGens::SysVersion::REGION_US_NTSC;
		case 0x8:	return LibGens::SysVersion::REGION_EU_PAL;
		case 0x1:	return LibGens::SysVersion::REGION_JP_NTSC;
		case 0x2:	return LibGens::SysVersion::REGION_ASIA_PAL;
	}
}


/**
 * Close the open ROM file and stop emulation.
 * @param emitStateChanged If true, emits the stateChanged() signal after the ROM is closed.
 */
int EmuManager::closeRom(bool emitStateChanged)
{
	if (!isRomOpen())
		return 0;

	if (gqt4_emuThread) {
		// Disconnect the emuThread's signals.
		gqt4_emuThread->disconnect();

		// Stop and delete the emulation thread.
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = nullptr;
	}

	if (gqt4_emuContext) {
		if (emitStateChanged) {
			LibGens::MdFb *prevFb = m_romClosedFb;

			const int introStyle = gqt4_cfg->getInt(QLatin1String("Intro_Effect/introStyle"));
			if (introStyle != 0) {
				// Intro Effect is enabled.
				// Save the previous source framebuffer.

				m_romClosedFb = gqt4_emuContext->m_vdp->MD_Screen->ref();
				m_romClosedBpp = gqt4_emuContext->m_vdp->m_palette.bpp();
			}

			// Unreference the last previous source framebuffer.
			if (prevFb)
				prevFb->unref();
		}

		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		gqt4_emuContext->saveData();

		// Delete the emulation context.
		// FIXME: Delete gqt4_emuContext after VBackend is finished using it. (MEMORY LEAK)
		m_vBackend->setEmuContext(nullptr);
		delete gqt4_emuContext;
		gqt4_emuContext = nullptr;

		// Delete the Rom instance.
		// TODO: Handle this in gqt4_emuContext.
		delete m_rom;
		m_rom = nullptr;
	}

	// Close audio.
	m_audio->close();
	m_paused.data = 0;

	// Only clear the screen if we're emitting stateChanged().
	// If we're not emitting stateChanged(), this usually means
	// we're loading a new ROM immediately afterwards, so
	// clearing the screen is a waste of time.
	if (emitStateChanged) {
		// Update the Gens title.
		emit stateChanged();
	}

	return 0;
}


/**
 * Get the ROM name.
 * @return ROM name, or empty string if no ROM is loaded.
 */
QString EmuManager::romName(void)
{
	if (!m_rom || !gqt4_emuContext)
		return QString();

	// TODO: This is MD/MCD/32X only!
	// TODO: Multi-context spport.

	// Check the active system region.
	std::string s_romName;
	if (gqt4_emuContext->versionRegisterObject()->isEast()) {
		// East (JP). Return the domestic ROM name.
		s_romName = m_rom->romNameJP();
		if (s_romName.empty()) {
			// Domestic ROM name is empty.
			// Return the overseas ROM name.
			s_romName = m_rom->romNameUS();
		}
	} else {
		// West (US/EU). Return the overseas ROM name.
		s_romName = m_rom->romNameUS();
		if (s_romName.empty()) {
			// Overseas ROM name is empty.
			// Return the domestic ROM name.
			s_romName = m_rom->romNameJP();
		}
	}

	if (s_romName.empty()) {
		// ROM name is empty.
		// This might be an unlicensed and/or pirated ROM.

		// Return the ROM filename, without extensions.
		// TODO: Also the zfilename?
		return QString::fromUtf8(m_rom->filenameBaseNoExt().c_str());
	}

	// Return the ROM name.
	return QString::fromUtf8(s_romName.c_str());
}


/**
 * sysName(): Get the system name for the active ROM, based on ROM region.
 * @return System name, or empty string if unknown or no ROM is loaded.
 */
QString EmuManager::sysName(void)
{
	if (!m_rom || !gqt4_emuContext)
		return QString();
	
	return SysName(m_rom->sysId(),
			gqt4_emuContext->versionRegisterObject()->region());
}


/**
 * Get the savestate filename.
 * TODO: Move savestate code to another file?
 * NOTE: Returned filename uses Qt directory separators. ('/')
 * @return Savestate filename, or empty string if no ROM is loaded.
 */
QString EmuManager::getSaveStateFilename(void)
{
	if (!m_rom)
		return QString();
	
	const QString filename =
		gqt4_cfg->configPath(PathConfig::GCPATH_SAVESTATES) +
		QString::fromUtf8(m_rom->filenameBaseNoExt().c_str()) +
		QChar(L'.') + QString::number(m_saveSlot) +
		QLatin1String(".zomg");
	return filename;
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

	// Check for controller configuration updates.
	if (gqt4_cfg->m_ctrlConfig->isDirty()) {
		// Update the controller ports.
		gqt4_cfg->m_ctrlConfig->updateIoManager(gqt4_emuContext->m_ioManager);
		gqt4_cfg->m_ctrlConfig->clearDirty();
	}

	// Update the Video Backend.
	if (!wasFastFrame)
		updateVBackend();
	
	// If emulation is paused, don't resume the emulation thread.
	if (m_paused.data)
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
	const double frameRate = (1.0 /
			(gqt4_emuContext->versionRegisterObject()->isPal() ? 50.0 : 60.0));
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
			
			// NOTE: Putting usleep(0) here reduces CPU usage,
			// but increases stuttering.
			// TODO: Fix framedropping entirely.
			//usleep(0);
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

/** Video Backend. **/

/**
 * Set the Video Backend.
 * @param vBackend Video Backend.
 */
void EmuManager::setVBackend(VBackend *vBackend)
{
	if (m_vBackend)
	{
		// Disconnect the destroyed() signal from the current VBackend.
		disconnect(m_vBackend, SIGNAL(destroyed(QObject*)),
			   this, SLOT(vBackend_destroyed(QObject*)));
	}
	
	// Save the new Video Backend.
	m_vBackend = vBackend;
	
	if (m_vBackend)
	{
		// Connect the destroyed() signal to the new VBackend.
		connect(m_vBackend, SIGNAL(destroyed(QObject*)),
			this, SLOT(vBackend_destroyed(QObject*)));
	}
}

/**
 * Video Backend was destroyed.
 * @param obj VBackend object.
 */
void EmuManager::vBackend_destroyed(QObject *obj)
{
	if (m_vBackend == obj)
		m_vBackend = nullptr;
}

/**
 * Update the Video Backend.
 */
void EmuManager::updateVBackend(void)
{
	if (!m_vBackend)
		return;

	m_vBackend->setMdScreenDirty();
	m_vBackend->setVbDirty();

	if (gqt4_emuContext) {
		const LibGens::Vdp *vdp = gqt4_emuContext->m_vdp;
		m_vBackend->vbUpdate(vdp->MD_Screen, vdp->m_palette.bpp());
	} else {
		// TODO: Get color depth from ConfigStore.
		m_vBackend->vbUpdate(nullptr, LibGens::VdpPalette::BPP_32);
	}
}

}
