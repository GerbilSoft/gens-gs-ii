/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow_menu_slots.cpp: Gens Window: Menu slots.                        *
 * These are functions run when menu items are selected.                      *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2015 by David Korth.                                    *
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

#include "GensWindow.hpp"
#include "gqt4_main.hpp"

// C includes. (C++ namespace)
#include <cassert>

// LibGens includes.
#include "libgens/EmuContext/SysVersion.hpp"
using LibGens::SysVersion;

// Other windows.
#include "windows/AboutDialog.hpp"
#include "windows/CtrlConfigWindow.hpp"
#include "windows/GeneralConfigWindow.hpp"
#include "windows/McdControlWindow.hpp"

#include "GensWindow_p.hpp"
namespace GensQt4 {

/** File **/

void GensWindow::on_actionFileOpenROM_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->openRom();
}

void GensWindow::on_actionFileCloseROM_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->closeRom();
}

void GensWindow::on_actionFileSaveState_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->saveState();
}

void GensWindow::on_actionFileLoadState_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->loadState();
}

void GensWindow::on_actionFileGeneralConfiguration_triggered(void)
{
	GeneralConfigWindow::ShowSingle(this);
}

void GensWindow::on_actionFileSegaCDControlPanel_triggered(void)
{
	McdControlWindow::ShowSingle(this);
}

void GensWindow::on_actionFileQuit_triggered(void)
{
	Q_D(GensWindow);
	setIdleThreadAllowed(false);
	d->emuManager->closeRom();
	QuitGens();
	close();
}

/** File, Recent ROMs **/

void GensWindow::mnu_actionFileRecentROMs_triggered(int idx)
{
	// Load the selected Recent ROM.
	// TODO: Return a pointer?
	const RecentRom_t rom = gqt4_cfg->recentRomsEntry(idx);
	if (!rom.filename.isEmpty()) {
		// Recent ROM index is valid.
		// Load the ROM.
		Q_D(GensWindow);
		d->emuManager->openRom(rom.filename, rom.z_filename);
	}
}

/** Graphics **/

void GensWindow::on_actionGraphicsShowMenuBar_triggered(bool checked)
{
	// This will automatically update GensWindow.
	gqt4_cfg->set(QLatin1String("GensWindow/showMenuBar"), checked);
}

// TODO: Base menu function?
// TODO: Make auto-exclusive, and add a 'custom' item?
//void GensWindow::on_mnuGraphicsResolution_triggered(void);
void GensWindow::map_actionGraphicsResolution_triggered(int scale)
{
	this->rescale(4);
	assert(scale >= 1 && scale <= 4);
	this->rescale(scale);
}

// TODO: Base menu function?
//void GensWindow::on_mnuGraphicsBpp_triggered(void);
void GensWindow::map_actionGraphicsBpp_triggered(int bpp)
 {
	assert(bpp >= LibGens::MdFb::BPP_15 &&
	       bpp <  LibGens::MdFb::BPP_MAX);
	this->setBpp((LibGens::MdFb::ColorDepth)bpp);
}

void GensWindow::mnu_mnuGraphicsStretch_triggered(void)
{
	// Cycle through stretch modes.
	Q_D(GensWindow);
	int stretch_tmp = d->vBackend->stretchMode();
	stretch_tmp = (stretch_tmp + 1) % 4;
	d->vBackend->setStretchMode((StretchMode_t)stretch_tmp);
}

void GensWindow::map_actionGraphicsStretch_triggered(int stretchMode)
{
	assert(stretchMode >= STRETCH_NONE && stretchMode <= STRETCH_FULL);
	Q_D(GensWindow);
	d->vBackend->setStretchMode((StretchMode_t)stretchMode);
}

void GensWindow::on_actionGraphicsScreenshot_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->screenShot();
}

/** System **/

void GensWindow::mnu_mnuSystemRegion_triggered(void)
{
	// Switch to the next region setting.
	int region = gqt4_cfg->getInt(QLatin1String("System/regionCode")) + 1;
	if (region > SysVersion::REGION_EU_PAL) {
		region = SysVersion::REGION_AUTO;
	}
	gqt4_cfg->set(QLatin1String("System/regionCode"), region);
}

void GensWindow::map_actionSystemRegion_triggered(int region)
{
	// TODO: Rename to JPN/ASIA/USA/EUR?
	assert(region >= SysVersion::REGION_AUTO && region <= SysVersion::REGION_EU_PAL);
	gqt4_cfg->set(QLatin1String("System/regionCode"), region);
}

void GensWindow::on_actionSystemHardReset_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetEmulator(true);
}

void GensWindow::on_actionSystemSoftReset_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetEmulator(false);
}

void GensWindow::on_actionSystemPause_triggered(bool checked)
{
	Q_D(GensWindow);
	d->emuManager->pauseRequest(checked);
}

void GensWindow::on_actionSystemResetM68K_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetCpu(EmuManager::RQT_CPU_M68K);
}

void GensWindow::on_actionSystemResetS68K_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetCpu(EmuManager::RQT_CPU_S68K);
}

void GensWindow::on_actionSystemResetMSH2_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetCpu(EmuManager::RQT_CPU_MSH2);
}

void GensWindow::on_actionSystemResetSSH2_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetCpu(EmuManager::RQT_CPU_SSH2);
}

void GensWindow::on_actionSystemResetZ80_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->resetCpu(EmuManager::RQT_CPU_Z80);
}

/** Options **/

void GensWindow::on_actionOptionsSRAM_triggered(bool checked)
{
	gqt4_cfg->set(QLatin1String("Options/enableSRam"), checked);
}

void GensWindow::on_actionOptionsControllers_triggered(void)
{
	CtrlConfigWindow::ShowSingle(this);
}

// SoundTest; remove this later.
void GensWindow::map_actionSound_triggered(int freq)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(freq);
}

void GensWindow::on_actionSoundMono_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setStereo(false);
}

void GensWindow::on_actionSoundStereo_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setStereo(true);
}

/** Help **/

void GensWindow::on_actionHelpAbout_triggered(void)
{
	AboutDialog::ShowSingle(this);
}

/** Non-Menu Actions **/
// FIXME: Not being triggered in the UI...

void GensWindow::on_actionNoMenuFastBlur_triggered(bool checked)
{
	Q_D(GensWindow);
	d->vBackend->setFastBlur(checked);
}

void GensWindow::map_actionNoMenuSaveSlot_triggered(int saveSlot)
{
	assert(saveSlot >= 0 && saveSlot <= 9);
	gqt4_cfg->set(QLatin1String("Savestates/saveSlot"), saveSlot);
}

void GensWindow::on_actionNoMenuSaveSlotPrev_triggered(void)
{
	int saveSlot = gqt4_cfg->getInt(QLatin1String("Savestates/saveSlot"));
	saveSlot = ((saveSlot + 9) % 10);
	gqt4_cfg->set(QLatin1String("Savestates/saveSlot"), saveSlot);
}

void GensWindow::on_actionNoMenuSaveSlotNext_triggered(void)
{
	int saveSlot = gqt4_cfg->getInt(QLatin1String("Savestates/saveSlot"));
	saveSlot = ((saveSlot + 1) % 10);
	gqt4_cfg->set(QLatin1String("Savestates/saveSlot"), saveSlot);
}

void GensWindow::on_actionNoMenuSaveStateAs_triggered(void)
{
	// TODO (wasn't implemented in GensActions)
}

void GensWindow::on_actionNoMenuLoadStateFrom_triggered(void)
{
	// TODO (wasn't implemented in GensActions)
}

}
