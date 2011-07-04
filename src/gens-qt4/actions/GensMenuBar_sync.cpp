/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar_sync.cpp: Gens Menu Bar class: Synchronization functions.   *
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

#include "GensMenuBar.hpp"
#include "GensMenuBar_p.hpp"
#include "GensMenuBar_menus.hpp"

// gqt4_config
#include "gqt4_main.hpp"

// Qt includes.
#include <QtCore/qglobal.h>

// Recent ROMs menu.
#include "RecentRomsMenu.hpp"

namespace GensQt4
{

/*********************************
 * GensMenuBarPrivate functions. *
 *********************************/

/**
 * syncConnect(): Connect menu synchronization slots.
 */
void GensMenuBarPrivate::syncConnect(void)
{
	QObject::connect(gqt4_config, SIGNAL(stretchMode_changed(GensConfig::StretchMode_t)),
			 q, SLOT(stretchMode_changed_slot(GensConfig::StretchMode_t)));
	QObject::connect(gqt4_config, SIGNAL(regionCode_changed(int)),
			 q, SLOT(regionCode_changed_slot(int)));	// LibGens::SysVersion::RegionCode_t
	QObject::connect(gqt4_config, SIGNAL(enableSRam_changed(bool)),
			 q, SLOT(enableSRam_changed_slot(bool)));
}


/**
 * syncAll(): Synchronize all menus.
 */
void GensMenuBarPrivate::syncAll(void)
{
	this->lock();
	
	// Do synchronization.
	syncRecent();
	q->stretchMode_changed_slot(gqt4_config->stretchMode());
	q->regionCode_changed_slot(gqt4_config->regionCode());
	q->enableSRam_changed_slot(gqt4_config->enableSRam());
	q->stateChanged();
	
	this->unlock();
}


/**
 * syncRecent(): Synchronize the "Recent ROMs" menu.
 */
void GensMenuBarPrivate::syncRecent(void)
{
	this->lock();
	
	// Find the "Recent ROMs" action.
	QAction *actionRecentRoms = hashActions.value(IDM_FILE_RECENT);
	if (!actionRecentRoms)
		goto out;
	
	// Set the submenu.
	actionRecentRoms->setMenu(recentRomsMenu);
	
	// If there aren't any ROMs in the list, disable the action.
	actionRecentRoms->setEnabled(!recentRomsMenu->actions().isEmpty());
	
out:
	this->unlock();
}


/**************************
 * GensMenuBar functions. *
 **************************/

/** Synchronization slots. **/


/**
 * recentRoms_updated(): Recent ROMs menu has been updated.
 */
void GensMenuBar::recentRoms_updated(void)
{
	// Synchronize the "Recent ROMs" menu action.
	d->syncRecent();
}


/**
 * stretchMode_changed_slot(): Stretch mode has changed.
 * @param newStretchMode New stretch mode.
 */
void GensMenuBar::stretchMode_changed_slot(GensConfig::StretchMode_t newStretchMode)
{
	int id;
	switch (newStretchMode)
	{
		case GensConfig::STRETCH_NONE:	id = IDM_GRAPHICS_STRETCH_NONE; break;
		case GensConfig::STRETCH_H:	id = IDM_GRAPHICS_STRETCH_H;    break;
		case GensConfig::STRETCH_V:	id = IDM_GRAPHICS_STRETCH_V;    break;
		case GensConfig::STRETCH_FULL:	id = IDM_GRAPHICS_STRETCH_FULL; break;
		default:
			return;
	}
	
	// Find the action.
	QAction *action = d->hashActions.value(id, NULL);
	if (!action)
		return;
	
	// Set the stretch mode.
	this->lock();
	action->setChecked(true);
	this->unlock();
}


/**
 * regionCode_changed_slot(): Region code has changed.
 * @param newRegionCode New region code.
 */
// NOTE: Uses LibGens::SysVersion::RegionCode_t, but Q_ENUMS requires a QObject for storage.
void GensMenuBar::regionCode_changed_slot(int newRegionCode)
{
	int id;
	switch (newRegionCode)
	{
		case LibGens::SysVersion::REGION_AUTO:		id = IDM_SYSTEM_REGION_AUTODETECT; break;
		case LibGens::SysVersion::REGION_JP_NTSC:	id = IDM_SYSTEM_REGION_JAPAN;      break;
		case LibGens::SysVersion::REGION_ASIA_PAL:	id = IDM_SYSTEM_REGION_ASIA;       break;
		case LibGens::SysVersion::REGION_US_NTSC:	id = IDM_SYSTEM_REGION_USA;        break;
		case LibGens::SysVersion::REGION_EU_PAL:	id = IDM_SYSTEM_REGION_EUROPE;     break;
		default:
			return;
	}
	
	// Find the action.
	QAction *action = d->hashActions.value(id, NULL);
	if (!action)
		return;
	
	// Set the region code.
	this->lock();
	action->setChecked(true);
	this->unlock();
}


/**
 * enableSRam_changed_slot(): Enable SRam/EEPRom setting has changed.
 * @param newEnableSRam New Enable SRam/EEPRom setting.
 */
void GensMenuBar::enableSRam_changed_slot(bool newEnableSRam)
{
	// Find the action.
	QAction *action = d->hashActions.value(IDM_OPTIONS_ENABLESRAM, NULL);
	if (!action)
		return;
	
	// Set the check state.
	this->lock();
	action->setChecked(newEnableSRam);
	this->unlock();
}


/**
 * stateChanged(): Emulation state has changed.
 */
void GensMenuBar::stateChanged(void)
{
	// Update various menu items.
	QAction *actionPause = d->hashActions.value(IDM_SYSTEM_PAUSE);
	
	// Lock menu actions to prevent errant signals from being emitted.
	this->lock();
	
	const bool isRomOpen = (d->emuManager && d->emuManager->isRomOpen());
	const bool isPaused = (isRomOpen ? d->emuManager->paused().paused_manual : false);
	
	// TODO: Do we really need to check for NULL actions?
	
	// Update menu actions.
	if (actionPause)
	{
		actionPause->setEnabled(isRomOpen);
		actionPause->setChecked(isPaused);
	}
	
	// Simple enable-if-ROM-open actions.
	static const int EnableIfOpen[] =
	{
		IDM_FILE_CLOSE, IDM_FILE_SAVESTATE, IDM_FILE_LOADSTATE,
		IDM_GRAPHICS_SCRSHOT, IDM_SYSTEM_HARDRESET, IDM_SYSTEM_SOFTRESET,
		0
	};
	
	for (int i = ((sizeof(EnableIfOpen)/sizeof(EnableIfOpen[0]))-2); i >= 0; i--)
	{
		QAction *actionEnableIfOpen = d->hashActions.value(EnableIfOpen[i]);
		if (actionEnableIfOpen)
			actionEnableIfOpen->setEnabled(isRomOpen);
	}
	
#ifndef Q_WS_MAC
	// Show Menu Bar.
	QAction *actionShowMenuBar = d->hashActions.value(IDM_GRAPHICS_MENUBAR);
	if (actionShowMenuBar)
		actionShowMenuBar->setChecked(gqt4_config->showMenuBar());
#endif /* Q_WS_MAC */
	
	// Z80. (Common for all systems.)
	QAction *actionCpuReset = d->hashActions.value(IDM_SYSTEM_CPURESET_Z80);
	if (actionCpuReset)
	{
		actionCpuReset->setEnabled(isRomOpen);
		actionCpuReset->setVisible(isRomOpen);
	}
	
	// Main 68000. (MD, MCD, 32X, MCD32X)
	// TODO: Change title to "Main 68000" when Sega CD is enabled?
	actionCpuReset = d->hashActions.value(IDM_SYSTEM_CPURESET_M68K);
	if (actionCpuReset)
	{
		actionCpuReset->setEnabled(isRomOpen);
		actionCpuReset->setVisible(isRomOpen);
	}
	
	// Sub 68000. (MCD, MCD32X)
	// TODO: Identify active system.
	actionCpuReset = d->hashActions.value(IDM_SYSTEM_CPURESET_S68K);
	if (actionCpuReset)
	{
		actionCpuReset->setEnabled(false);
		actionCpuReset->setVisible(false);
	}
	
	// Master and Slave SH2. (32X, MCD32X)
	// TODO: Identify active system.
	actionCpuReset = d->hashActions.value(IDM_SYSTEM_CPURESET_MSH2);
	if (actionCpuReset)
	{
		actionCpuReset->setEnabled(false);
		actionCpuReset->setVisible(false);
	}
	actionCpuReset = d->hashActions.value(IDM_SYSTEM_CPURESET_SSH2);
	if (actionCpuReset)
	{
		actionCpuReset->setEnabled(false);
		actionCpuReset->setVisible(false);
	}
	
	// Unlock menu actions.
	this->unlock();
}

}
