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

namespace GensQt4
{

/*********************************
 * GensMenuBarPrivate functions. *
 *********************************/

/**
 * syncAll(): Synchronize all menus.
 */
void GensMenuBarPrivate::syncAll(void)
{
	this->lock();
	
	// Do synchronization.
	q->stretchMode_changed_slot(gqt4_config->stretchMode());
	q->regionCode_changed_slot(gqt4_config->regionCode());
	q->stateChanged();
	
	this->unlock();
}


/**
 * syncConnect(): Connect menu synchronization slots.
 */
void GensMenuBarPrivate::syncConnect(void)
{
	QObject::connect(gqt4_config, SIGNAL(stretchMode_changed(GensConfig::StretchMode_t)),
			 q, SLOT(stretchMode_changed_slot(GensConfig::StretchMode_t)));
	QObject::connect(gqt4_config, SIGNAL(regionCode_changed(GensConfig::ConfRegionCode_t)),
			 q, SLOT(regionCode_changed_slot(GensConfig::ConfRegionCode_t)));
}


/**************************
 * GensMenuBar functions. *
 **************************/

/** Synchronization slots. **/


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
void GensMenuBar::regionCode_changed_slot(GensConfig::ConfRegionCode_t newRegionCode)
{
	int id;
	switch (newRegionCode)
	{
		case GensConfig::CONFREGION_AUTODETECT:	id = IDM_SYSTEM_REGION_AUTODETECT; break;
		case GensConfig::CONFREGION_JP_NTSC:	id = IDM_SYSTEM_REGION_JAPAN;      break;
		case GensConfig::CONFREGION_ASIA_PAL:	id = IDM_SYSTEM_REGION_ASIA;       break;
		case GensConfig::CONFREGION_US_NTSC:	id = IDM_SYSTEM_REGION_USA;        break;
		case GensConfig::CONFREGION_EU_PAL:	id = IDM_SYSTEM_REGION_EUROPE;     break;
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
	
	// System-specifc Reset CPU actions.
	// TODO: Determine the active system.
	QAction *actionCpuResetM68K = d->hashActions.value(IDM_SYSTEM_CPURESET_M68K);
	QAction *actionCpuResetZ80 = d->hashActions.value(IDM_SYSTEM_CPURESET_Z80);
	if (actionCpuResetM68K)
	{
		actionCpuResetM68K->setEnabled(isRomOpen);
		actionCpuResetM68K->setVisible(isRomOpen);
	}
	if (actionCpuResetZ80)
	{
		actionCpuResetZ80->setEnabled(isRomOpen);
		actionCpuResetZ80->setVisible(isRomOpen);
	}
	
	// Unlock menu actions.
	this->unlock();
}

}
