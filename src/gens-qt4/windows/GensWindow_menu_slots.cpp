/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow_menu_slots.cpp: Gens Window: Menu slots.                        *
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

/** Graphics **/

void GensWindow::on_actionGraphicsShowMenuBar_toggled(bool checked)
{
	// This will automatically update GensWindow.
	gqt4_cfg->set(QLatin1String("GensWindow/showMenuBar"), checked);
}

// TODO: Base menu function?
//void GensWindow::on_mnuGraphicsResolution_triggered(void);
// TODO: SignalMapper?
// TODO: Make auto-exclusive, and add a 'custom' item?
void GensWindow::on_actionGraphicsResolution1x_triggered(void)
{
	this->rescale(1);
}

void GensWindow::on_actionGraphicsResolution2x_triggered(void)
{
	this->rescale(2);
}

void GensWindow::on_actionGraphicsResolution3x_triggered(void)
{
	this->rescale(3);
}

void GensWindow::on_actionGraphicsResolution4x_triggered(void)
{
	this->rescale(4);
}

// TODO: Base menu function?
//void GensWindow::on_mnuGraphicsBpp_triggered(void);
// TODO: SignalMapper?
void GensWindow::on_actionGraphicsBpp15_triggered(void)
{
	this->setBpp(LibGens::MdFb::BPP_15);
}

void GensWindow::on_actionGraphicsBpp16_triggered(void)
{
	this->setBpp(LibGens::MdFb::BPP_16);
}

void GensWindow::on_actionGraphicsBpp32_triggered(void)
{
	this->setBpp(LibGens::MdFb::BPP_32);
}

// FIXME: Need to manually find the mnuGraphicsStretch QAction
// and connect it to this function.
#if 0
void GensWindow::on_mnuGraphicsStretch_triggered(void)
{
	// Cycle through stretch modes.
	Q_D(GensWindow);
	int stretch_tmp = d->vBackend->stretchMode();
	stretch_tmp = (stretch_tmp + 1) % 4;
	d->vBackend->setStretchMode((StretchMode_t)stretch_tmp);
}
#endif

// TODO: SignalMapper?
void GensWindow::on_actionGraphicsStretchNone_triggered(void)
{
	Q_D(GensWindow);
	/* TODO: Move to init function. */
	// TODO: Add action groups to init function.
	d->ui.mnuGraphicsStretch->menuAction()->setShortcut(QKeySequence(QLatin1String("Shift+F2")));
	d->vBackend->setStretchMode(STRETCH_NONE);
}

void GensWindow::on_actionGraphicsStretchHorizontal_triggered(void)
{
	Q_D(GensWindow);
	d->vBackend->setStretchMode(STRETCH_H);
}

void GensWindow::on_actionGraphicsStretchVertical_triggered(void)
{
	Q_D(GensWindow);
	d->vBackend->setStretchMode(STRETCH_V);
}

void GensWindow::on_actionGraphicsStretchFull_triggered(void)
{
	Q_D(GensWindow);
	d->vBackend->setStretchMode(STRETCH_FULL);
}

void GensWindow::on_actionGraphicsScreenshot_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->screenShot();
}

/** System **/

// FIXME: Need to manually find the mnuSystemRegion QAction
// and connect it to this function.
#if 0
void GensWindow::on_mnuSystemRegion_triggered(void)
{
	// Switch to the next region setting.
	int region = gqt4_cfg->getInt(QLatin1String("System/regionCode")) + 1;
	if (region > (int)LibGens::SysVersion::REGION_EU_PAL) {
		region = (int)LibGens::SysVersion::REGION_AUTO;
	}
	gqt4_cfg->set(QLatin1String("System/regionCode"), region);
}
#endif

void GensWindow::on_actionSystemRegionAuto_triggered(void)
{
	gqt4_cfg->set(QLatin1String("System/regionCode"), (int)LibGens::SysVersion::REGION_AUTO);
}

void GensWindow::on_actionSystemRegionJPN_triggered(void)
{
	gqt4_cfg->set(QLatin1String("System/regionCode"), (int)LibGens::SysVersion::REGION_JP_NTSC);
}

void GensWindow::on_actionSystemRegionAsia_triggered(void)
{
	gqt4_cfg->set(QLatin1String("System/regionCode"), (int)LibGens::SysVersion::REGION_ASIA_PAL);
}

void GensWindow::on_actionSystemRegionUSA_triggered(void)
{
	// TODO: Rename to JPN/ASIA/USA/EUR?
	gqt4_cfg->set(QLatin1String("System/regionCode"), (int)LibGens::SysVersion::REGION_US_NTSC);
}

void GensWindow::on_actionSystemRegionEUR_triggered(void)
{
	gqt4_cfg->set(QLatin1String("System/regionCode"), (int)LibGens::SysVersion::REGION_EU_PAL);
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

void GensWindow::on_actionSystemPause_toggled(bool checked)
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

void GensWindow::on_actionOptionsSRAM_toggled(bool checked)
{
	gqt4_cfg->set(QLatin1String("Options/enableSRam"), checked);
}

void GensWindow::on_actionOptionsControllers_triggered(void)
{
	CtrlConfigWindow::ShowSingle(this);
}

// SoundTest; remove this later.
void GensWindow::on_actionSound11_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(11025);
}

void GensWindow::on_actionSound16_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(16000);
}

void GensWindow::on_actionSound22_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(22050);
}

void GensWindow::on_actionSound32_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(32000);
}

void GensWindow::on_actionSound44_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(44100);
}

void GensWindow::on_actionSound48_triggered(void)
{
	Q_D(GensWindow);
	d->emuManager->setAudioRate(48000);
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

// TODO: Non-menu actions.

#if 0
/** VBackend properties. **/
void GensWindow::toggleFastBlur(void)
{
	Q_D(GensWindow);
	d->vBackend->setFastBlur(!d->vBackend->fastBlur());
}
#endif

}
