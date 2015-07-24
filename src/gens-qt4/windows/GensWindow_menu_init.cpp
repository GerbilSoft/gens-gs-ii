/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow_menu_init.cpp: Gens Window: Menu initialization.                *
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

// NOTE: Before committing: check the LATEST UI file!
#include "GensWindow.hpp"

#include "GensQApplication.hpp"
#include "gqt4_main.hpp"

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

// Gens Menu shortcuts.
#include "GensMenuShortcuts.hpp"

#include "GensWindow_p.hpp"
namespace GensQt4 {

/** GensWindowPrivate **/

/**
 * Initialize the menu bar.
 */
void GensWindowPrivate::initMenuBar(void)
{
	Q_Q(GensWindow);

	// Initialize roles for Mac OS X.
	// We don't want any heuristics to be enabled by default.
	// NOTE: Not limiting this to OS X in case Qt adds
	// support for other platforms later on.
	// FIXME: q->actions() is empty. Process menus and non-menu actions manually.
	foreach (QAction *action, q->actions()) {
		action->setMenuRole(QAction::NoRole);
	}

	// Set menu roles for specific actions manually.
	ui.actionFileQuit->setMenuRole(QAction::QuitRole);
	ui.actionFileGeneralConfiguration->setMenuRole(QAction::PreferencesRole);
	// TODO: Hide the "Help" menu?
	ui.actionHelpAbout->setMenuRole(QAction::AboutRole);
	// TODO: Set standard shortcuts?
	// TODO: Rename "Quit" to "Exit" on Windows.

	// Hide the "Show Menu Bar" item if we're using a global menu bar.
	// TODO: On Linux, this may change at runtime...
	if (isGlobalMenuBar()) {
		// NOTE: QAction::setVisible(false) does NOT work.
		// Remove the QAction from the menu.
		// TODO: Re-add if it changes at runtime.
		ui.mnuGraphics->removeAction(ui.actionGraphicsShowMenuBar);
	}

	// Create the "Recent ROMs" menu.
	recentRomsMenu = new RecentRomsMenu(q, gqt4_cfg->recentRomsObject());
	ui.actionFileRecentROMs->setMenu(recentRomsMenu);
	QObject::connect(recentRomsMenu, SIGNAL(updated()),
			 q, SLOT(recentRoms_updated()));
	QObject::connect(recentRomsMenu, SIGNAL(triggered(int)),
			 q, SLOT(mnu_actionFileRecentROMs_triggered(int)));

	// Create QSignalMappers and QActionGroups for submenus
	// with lots of similar items.
	QSignalMapper *mapper;
	QActionGroup *actgrp;

	// QSignalMapper macros.
	#define initMapper(_slot) do { \
		mapper = new QSignalMapper(q); \
		QObject::connect(mapper, SIGNAL(mapped(int)), \
			q, (_slot), Qt::UniqueConnection); \
	} while (0)
	#define doMapping(_widget, _id) do { \
		mapper->setMapping((_widget), (_id)); \
		QObject::connect((_widget), SIGNAL(triggered()), mapper, \
			SLOT(map()), Qt::UniqueConnection); \
	} while (0)

	// QActionGroup macros.
	#define initActGrp() do { \
		actgrp = new QActionGroup(q); \
		actgrp->setExclusive(true); \
	} while (0)
	#define doMappingExc(_widget, _id) do { \
		doMapping((_widget), (_id)); \
		actgrp->addAction(_widget); \
	} while (0)

	// Macros for connecting the base item for submenus.
	// _menu = menu item object
	// _name = name for the QAction
	// _slot = slot to connect triggered() to.
	#define doBaseMenu(_menu, _name, _slot) do { \
		QAction *action = (_menu)->menuAction(); \
		action->setObjectName(QLatin1String(_name)); \
		QObject::connect(action, SIGNAL(triggered()), \
			q, (_slot), Qt::UniqueConnection); \
	} while (0)

	// Graphics, Resolution.
	initMapper(SLOT(map_actionGraphicsResolution_triggered(int)));
	doMapping(ui.actionGraphicsResolution1x, 1);
	doMapping(ui.actionGraphicsResolution2x, 2);
	doMapping(ui.actionGraphicsResolution3x, 3);
	doMapping(ui.actionGraphicsResolution4x, 4);
	// TODO: QActionGroup?

	// Graphics, Color Depth.
	initMapper(SLOT(map_actionGraphicsBpp_triggered(int)));
	initActGrp();
	doMappingExc(ui.actionGraphicsBpp15, MdFb::BPP_15);
	doMappingExc(ui.actionGraphicsBpp16, MdFb::BPP_16);
	doMappingExc(ui.actionGraphicsBpp32, MdFb::BPP_32);

	// Graphics, Stretch Mode.
	initMapper(SLOT(map_actionGraphicsStretch_triggered(int)));
	initActGrp();
	doMappingExc(ui.actionGraphicsStretchNone,       STRETCH_NONE);
	doMappingExc(ui.actionGraphicsStretchHorizontal, STRETCH_H);
	doMappingExc(ui.actionGraphicsStretchVertical,   STRETCH_V);
	doMappingExc(ui.actionGraphicsStretchFull,       STRETCH_FULL);
	doBaseMenu(ui.mnuGraphicsStretch, "actionGraphicsStretch",
		SLOT(mnu_mnuGraphicsStretch_triggered()));

	// System, Region.
	initMapper(SLOT(map_actionSystemRegion_triggered(int)));
	initActGrp();
	doMappingExc(ui.actionSystemRegionAuto, SysVersion::REGION_AUTO);
	doMappingExc(ui.actionSystemRegionJPN,  SysVersion::REGION_JP_NTSC);
	doMappingExc(ui.actionSystemRegionAsia, SysVersion::REGION_ASIA_PAL);
	doMappingExc(ui.actionSystemRegionUSA,  SysVersion::REGION_US_NTSC);
	doMappingExc(ui.actionSystemRegionEUR,  SysVersion::REGION_EU_PAL);
	doBaseMenu(ui.mnuSystemRegion, "actionSystemRegion",
		SLOT(mnu_mnuSystemRegion_triggered()));

	// Options, SoundTest.
	initMapper(SLOT(map_actionSound_triggered(int)));
	initActGrp();
	doMappingExc(ui.actionSound11, 11025);
	doMappingExc(ui.actionSound16, 16000);
	doMappingExc(ui.actionSound22, 22050);
	doMappingExc(ui.actionSound32, 32000);
	doMappingExc(ui.actionSound44, 44100);
	doMappingExc(ui.actionSound48, 48000);

	// Non-Menu Actions.
	initMapper(SLOT(map_actionNoMenuSaveSlot_triggered(int)));
	initActGrp();
	doMappingExc(ui.actionNoMenuSaveSlot0, 0);
	doMappingExc(ui.actionNoMenuSaveSlot1, 1);
	doMappingExc(ui.actionNoMenuSaveSlot2, 2);
	doMappingExc(ui.actionNoMenuSaveSlot3, 3);
	doMappingExc(ui.actionNoMenuSaveSlot4, 4);
	doMappingExc(ui.actionNoMenuSaveSlot5, 5);
	doMappingExc(ui.actionNoMenuSaveSlot6, 6);
	doMappingExc(ui.actionNoMenuSaveSlot7, 7);
	doMappingExc(ui.actionNoMenuSaveSlot8, 8);
	doMappingExc(ui.actionNoMenuSaveSlot9, 9);

	/** Menu synchronization slots. **/
	// TODO: Qt::UniqueConnection?
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/stretchMode"),
					q, SLOT(stretchMode_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("System/regionCode"),
					q, SLOT(regionCode_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Options/enableSRam"),
					q, SLOT(enableSRam_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("GensWindow/showMenuBar"),
					q, SLOT(showMenuBar_changed_slot(QVariant)));

#if !defined(Q_OS_MAC)
	/** Menu item icons. **/
	// NOTE: Menu item icons don't fit in on Mac OS X, hence why
	// this is in a !defined(Q_OS_MAC) block.

	// NOTE: We're initializing menu item icons here instead of
	// in Qt Designer because we have a custom icon system that
	// retrieves system icons in some cases.
	// Also, xdg theme icons are only properly supported in qt-4.8.
	#define setActionIcon(_action, _icon_fdo) \
		_action->setIcon(GensQApplication::IconFromTheme(QLatin1String(_icon_fdo)));

	// TODO: Add more icons.
	// TODO: Add OS "standard" icons, like in mcrecover.
	// File
	setActionIcon(ui.actionFileOpenROM, "document-open");
	setActionIcon(ui.actionFileRecentROMs, "document-open-recent");
	setActionIcon(ui.actionFileCloseROM, "document-close");
	setActionIcon(ui.actionFileGeneralConfiguration, "configure");
	setActionIcon(ui.actionFileSegaCDControlPanel, "media-optical");
	setActionIcon(ui.actionFileQuit, "application-exit");
	// Options
	setActionIcon(ui.actionOptionsControllers, "input-gaming");
	// Help
	setActionIcon(ui.actionHelpAbout, "help-about");
#endif /* !defined(Q_OS_MAC) */

	// Set the Gens Menu shortcuts.
	// TODO: Split into two classes:
	// - Config class (common, shared with everything)
	// - Implementation class (unique, one per window)
	// TODO: Non-menu actions.
	GensMenuShortcuts *menuShortcuts = gqt4_cfg->gensMenuShortcuts();
	menuShortcuts->clear();
	foreach (QAction *action, q->menuBar()->actions()) {
		QMenu *menu = action->menu();
		// TODO: Is this check needed?
		if (menu != nullptr) {
			menuShortcuts->addMenu(menu);
		}
	}

	// Shortcuts for non-menu actions.
	if (!nonMenu) {
		nonMenu = new QMenu(q);
	} else {
		nonMenu->clear();
	}
	nonMenu->addAction(ui.actionNoMenuFastBlur);
	nonMenu->addAction(ui.actionNoMenuSaveSlot0);
	nonMenu->addAction(ui.actionNoMenuSaveSlot1);
	nonMenu->addAction(ui.actionNoMenuSaveSlot2);
	nonMenu->addAction(ui.actionNoMenuSaveSlot3);
	nonMenu->addAction(ui.actionNoMenuSaveSlot4);
	nonMenu->addAction(ui.actionNoMenuSaveSlot5);
	nonMenu->addAction(ui.actionNoMenuSaveSlot6);
	nonMenu->addAction(ui.actionNoMenuSaveSlot7);
	nonMenu->addAction(ui.actionNoMenuSaveSlot8);
	nonMenu->addAction(ui.actionNoMenuSaveSlot9);
	nonMenu->addAction(ui.actionNoMenuSaveSlotPrev);
	nonMenu->addAction(ui.actionNoMenuSaveSlotNext);
	nonMenu->addAction(ui.actionNoMenuLoadStateFrom);
	nonMenu->addAction(ui.actionNoMenuSaveStateAs);
	menuShortcuts->addMenu(nonMenu);

	// Synchronize the menu items.
	syncAll();

	// Update the menu bar visibility.
	updateMenuBarVisibility();
}

/**
 * Update the menu bar visibility.
 */
void GensWindowPrivate::updateMenuBarVisibility(void)
{
	// TODO: If the value changed and we're windowed,
	// resize the window to compensate.
	Q_Q(GensWindow);
	QMenuBar *menuBar = q->menuBar();
	int height_adjust = 0;

	if (!isShowMenuBar()) {
		// Hide the menu bar.
		if (!q->isMaximized() && !q->isMinimized()) {
			height_adjust = -menuBar->height();
		}
		if (!isGlobalMenuBar()) {
			// System does not have a global menu bar.
			menuBar->setVisible(false);
		}
	} else {
		// Check if the menu bar was there already.
		const bool wasMenuBarThere = menuBar->isVisible();

		// Show the menu bar.
		menuBar->setVisible(true);

		// Adjust the menu bar size.
		if (!wasMenuBarThere && !q->isMaximized() && !q->isMinimized()) {
			menuBar->adjustSize();	// ensure the QMenuBar gets the correct size
			height_adjust = menuBar->height();
		}
	}

	// Hide "Show Menu Bar" if we're using a global menu bar.
	ui.actionGraphicsShowMenuBar->setVisible(!isGlobalMenuBar());

	if (!isGlobalMenuBar() && height_adjust != 0) {
		// Adjust the window height to compensate for the menu bar change.
		q->resize(q->width(), q->height() + height_adjust);
	}
}

}
