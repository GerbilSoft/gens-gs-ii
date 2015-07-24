/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow_menu_sync.cpp: Gens Window: Menu synchronization.               *
 * These are functions and slots used to synchronize the menu state.          *
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

// Qt includes.
#include <QtCore/QSignalMapper>
#include <QtGui/QActionGroup>

// LibGens includes.
#include "libgens/Util/MdFb.hpp"
#include "libgens/MD/SysVersion.hpp"
using LibGens::MdFb;
using LibGens::SysVersion;

// Recent ROMs menu.
#include "widgets/RecentRomsMenu.hpp"

#include "GensWindow_p.hpp"
namespace GensQt4 {

/**
 * NOTE: QAction::triggered() is NOT emitted if
 * QAction::setChecked() is called. Therefore,
 * locking/unlocking is unnecessary.
 */

/** GensWindowPrivate **/

/**
 * Synchronize all menus.
 */
void GensWindowPrivate::syncAll(void)
{
	syncRecent();
	syncShowMenuBar();

	// Run some synchronization slots.
	stretchMode_changed_slot_int(
		(StretchMode_t)gqt4_cfg->getInt(QLatin1String("Graphics/stretchMode")));
	regionCode_changed_slot_int(
		(SysVersion::RegionCode_t)gqt4_cfg->getInt(QLatin1String("System/regionCode")));
	enableSRam_changed_slot_int(
		gqt4_cfg->get(QLatin1String("Options/enableSRam")).toBool());
	updateMenusForStateChanged();
}

/**
 * Synchronize the "Recent ROMs" menu.
 */
void GensWindowPrivate::syncRecent(void)
{
	// If there aren't any ROMs in the list, disable the action.
	bool enabled = false;
	if (recentRomsMenu && !recentRomsMenu->actions().isEmpty()) {
		// Recent ROMs are available.
		enabled = true;
	}
	ui.actionFileRecentROMs->setEnabled(enabled);
}

/**
 * Synchronize the "Show Menu Bar" item.
 */
void GensWindowPrivate::syncShowMenuBar(void)
{
	ui.actionGraphicsShowMenuBar->setChecked(
		gqt4_cfg->get(QLatin1String("GensWindow/showMenuBar")).toBool());
}

/**
 * Update menus in response to a stateChanged() event.
 */
void GensWindowPrivate::updateMenusForStateChanged(void)
{
	// Update various menu items.
	// NOTE: Locking is not necessary, since we're using
	// QAction::triggered(). This signal is only emitted
	// if the user clicks the menu item.

	const bool isRomOpen = (this->emuManager && this->emuManager->isRomOpen());
	const bool isPaused = (isRomOpen ? this->emuManager->paused().paused_manual : false);

	// Update menu actions.
	ui.actionSystemPause->setEnabled(isRomOpen);
	ui.actionSystemPause->setChecked(isPaused);

	// Simple enable-if-ROM-open actions.
	ui.actionFileCloseROM->setEnabled(isRomOpen);
	ui.actionFileSaveState->setEnabled(isRomOpen);
	ui.actionFileLoadState->setEnabled(isRomOpen);
	ui.actionGraphicsScreenshot->setEnabled(isRomOpen);
	ui.actionSystemHardReset->setEnabled(isRomOpen);
	ui.actionSystemSoftReset->setEnabled(isRomOpen);

	// Z80. (Common for all systems.)
	ui.actionSystemResetZ80->setEnabled(isRomOpen);
	ui.actionSystemResetZ80->setVisible(isRomOpen);

	// Main 68000. (MD, MCD, 32X, MCD32X)
	// TODO: Change title to "Main 68000" when Sega CD is enabled?
	ui.actionSystemResetM68K->setEnabled(isRomOpen);
	ui.actionSystemResetM68K->setVisible(isRomOpen);

	// Sub 68000. (MCD, MCD32X)
	// TODO: Identify active system.
	ui.actionSystemResetS68K->setEnabled(false);
	ui.actionSystemResetS68K->setVisible(false);

	// Master and Slave SH2. (32X, MCD32X)
	// TODO: Identify active system.
	ui.actionSystemResetMSH2->setEnabled(false);
	ui.actionSystemResetMSH2->setVisible(false);
	ui.actionSystemResetSSH2->setEnabled(false);
	ui.actionSystemResetSSH2->setVisible(false);
}

/** Menu synchronization slots. **/

/**
 * Recent ROMs menu has been updated.
 */
void GensWindow::recentRoms_updated(void)
{
	// Synchronize the "Recent ROMs" menu action.
	Q_D(GensWindow);
	d->syncRecent();
}

/**
 * Stretch mode has changed.
 * (Internal version.)
 * @param stretchMode New stretch mode.
 */
void GensWindowPrivate::stretchMode_changed_slot_int(StretchMode_t stretchMode)
{
	assert(stretchMode >= STRETCH_NONE && stretchMode <= STRETCH_FULL);

	QAction *action = nullptr;
	switch (stretchMode) {
		case STRETCH_NONE:
			action = ui.actionGraphicsStretchNone;
			break;
		case STRETCH_H:
			action = ui.actionGraphicsStretchHorizontal;
			break;
		case STRETCH_V:
			action = ui.actionGraphicsStretchVertical;
			break;
		case STRETCH_FULL:
			action = ui.actionGraphicsStretchFull;
			break;
		default:
			break;
	}
	if (!action)
		return;

	// Set the stretch mode.
	action->setChecked(true);
}

/**
 * Stretch mode has changed.
 * (WRAPPER slot for use with ConfigStore.)
 * @param stretchMode (StretchMode_t) New stretch mode.
 */
void GensWindow::stretchMode_changed_slot(const QVariant &stretchMode)
{
	// Wrapper for stretchMode_changed_slot_int().
	Q_D(GensWindow);
	d->stretchMode_changed_slot_int((StretchMode_t)stretchMode.toInt());
}

/**
 * Region code has changed.
 * @param regionCode New region code.
 */
void GensWindowPrivate::regionCode_changed_slot_int(LibGens::SysVersion::RegionCode_t regionCode)
{
	assert(regionCode >= LibGens::SysVersion::REGION_AUTO &&
	       regionCode <= LibGens::SysVersion::REGION_EU_PAL);
	QAction *action = nullptr;
	switch (regionCode) {
		case LibGens::SysVersion::REGION_AUTO:
			action = ui.actionSystemRegionAuto;
			break;
		case LibGens::SysVersion::REGION_JP_NTSC:
			action = ui.actionSystemRegionJPN;
			break;
		case LibGens::SysVersion::REGION_ASIA_PAL:
			action = ui.actionSystemRegionAsia;
			break;
		case LibGens::SysVersion::REGION_US_NTSC:
			action = ui.actionSystemRegionUSA;
			break;
		case LibGens::SysVersion::REGION_EU_PAL:
			action = ui.actionSystemRegionEUR;
			break;
		default:
			break;
	}
	if (!action)
		return;

	// Set the region code.
	action->setChecked(true);
}

/**
 * Region code has changed.
 * (WRAPPER slot for use with ConfigStore.)
 * @param regionCode (RegionCode_t) New region code.
 */
void GensWindow::regionCode_changed_slot(const QVariant &regionCode)
{
	// Wrapper for regionCode_changed_slot_int().
	Q_D(GensWindow);
	d->regionCode_changed_slot_int(
		(LibGens::SysVersion::RegionCode_t)regionCode.toInt());
}

/**
 * Enable SRam/EEPRom setting has changed.
 * @param enableSRam New Enable SRam/EEPRom setting.
 */
void GensWindowPrivate::enableSRam_changed_slot_int(bool enableSRam)
{
	// Set the check state.
	ui.actionOptionsSRAM->setChecked(enableSRam);
}

/**
 * Enable SRam/EEPRom setting has changed.
 * (WRAPPER slot for use with ConfigStore.)
 * @param enableSRam (bool) New Enable SRam/EEPRom setting.
 */
void GensWindow::enableSRam_changed_slot(const QVariant &enableSRam)
{
	// Wrapper for enableSRam_changed_slot_int().
	Q_D(GensWindow);
	d->enableSRam_changed_slot_int(enableSRam.toBool());
}

/**
 * "Show Menu Bar" setting has changed.
 * @param showMenuBar New "Show Menu Bar" setting.
 */
void GensWindow::showMenuBar_changed_slot(const QVariant &showMenuBar)
{
	Q_UNUSED(showMenuBar)

	// Synchronize the "Show Menu Bar" item.
	Q_D(GensWindow);
	d->syncShowMenuBar();

	// Update the menu bar visibility.
	// TODO: Use the QAction state instead of
	// storing it separately?
	d->cfg_showMenuBar = showMenuBar.toBool();
	d->updateMenuBarVisibility();
}

}
