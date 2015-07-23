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

// Qt includes.
#include <QtCore/QSignalMapper>
#include <QtGui/QActionGroup>

// LibGens includes.
#include "libgens/Util/MdFb.hpp"
#include "libgens/MD/SysVersion.hpp"
using LibGens::MdFb;
using LibGens::SysVersion;

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

	// Create QSignalMappers and QActionGroups for submenus
	// with lots of similar items.
	QSignalMapper *mapper;
	QActionGroup *actgrp;

	// Graphcs, Resolution.
	mapper = new QSignalMapper(q);
	mapper->setMapping(ui.actionGraphicsResolution1x, 1);
	mapper->setMapping(ui.actionGraphicsResolution2x, 2);
	mapper->setMapping(ui.actionGraphicsResolution3x, 3);
	mapper->setMapping(ui.actionGraphicsResolution4x, 4);

	QObject::connect(ui.actionGraphicsResolution1x, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsResolution2x, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsResolution3x, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsResolution4x, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);

	QObject::connect(mapper, SIGNAL(mapped(int)),
		q, SLOT(map_actionGraphicsResolution_triggered(int)),
		Qt::UniqueConnection);

	// Graphics, Color Depth.
	mapper = new QSignalMapper(q);
	mapper->setMapping(ui.actionGraphicsBpp15, MdFb::BPP_15);
	mapper->setMapping(ui.actionGraphicsBpp16, MdFb::BPP_16);
	mapper->setMapping(ui.actionGraphicsBpp32, MdFb::BPP_32);

	QObject::connect(ui.actionGraphicsBpp15, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsBpp16, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsBpp32, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);

	QObject::connect(mapper, SIGNAL(mapped(int)),
		q, SLOT(map_actionGraphicsBpp_triggered(int)),
		Qt::UniqueConnection);

	actgrp = new QActionGroup(q);
	actgrp->setExclusive(true);
	actgrp->addAction(ui.actionGraphicsBpp15);
	actgrp->addAction(ui.actionGraphicsBpp16);
	actgrp->addAction(ui.actionGraphicsBpp32);

	// Graphics, Stretch Mode.
	mapper = new QSignalMapper(q);
	mapper->setMapping(ui.actionGraphicsStretchNone, STRETCH_NONE);
	mapper->setMapping(ui.actionGraphicsStretchHorizontal, STRETCH_H);
	mapper->setMapping(ui.actionGraphicsStretchVertical, STRETCH_V);
	mapper->setMapping(ui.actionGraphicsStretchFull, STRETCH_FULL);

	QObject::connect(ui.actionGraphicsStretchNone, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsStretchHorizontal, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsStretchVertical, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionGraphicsStretchFull, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);

	QObject::connect(mapper, SIGNAL(mapped(int)),
		q, SLOT(map_actionGraphicsStretch_triggered(int)),
		Qt::UniqueConnection);

	actgrp = new QActionGroup(q);
	actgrp->setExclusive(true);
	actgrp->addAction(ui.actionGraphicsStretchNone);
	actgrp->addAction(ui.actionGraphicsStretchHorizontal);
	actgrp->addAction(ui.actionGraphicsStretchVertical);
	actgrp->addAction(ui.actionGraphicsStretchFull);

	// System, Region.
	mapper = new QSignalMapper(q);
	mapper->setMapping(ui.actionSystemRegionAuto, SysVersion::REGION_AUTO);
	mapper->setMapping(ui.actionSystemRegionJPN, SysVersion::REGION_JP_NTSC);
	mapper->setMapping(ui.actionSystemRegionAsia, SysVersion::REGION_ASIA_PAL);
	mapper->setMapping(ui.actionSystemRegionUSA, SysVersion::REGION_US_NTSC);
	mapper->setMapping(ui.actionSystemRegionEUR, SysVersion::REGION_EU_PAL);

	QObject::connect(ui.actionSystemRegionAuto, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionSystemRegionJPN, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionSystemRegionAsia, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionSystemRegionUSA, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);
	QObject::connect(ui.actionSystemRegionEUR, SIGNAL(triggered()),
		mapper, SLOT(map()), Qt::UniqueConnection);

	QObject::connect(mapper, SIGNAL(mapped(int)),
		q, SLOT(map_actionSystemRegion_triggered(int)),
		Qt::UniqueConnection);

	actgrp = new QActionGroup(q);
	actgrp->setExclusive(true);
	actgrp->addAction(ui.actionSystemRegionAuto);
	actgrp->addAction(ui.actionSystemRegionJPN);
	actgrp->addAction(ui.actionSystemRegionAsia);
	actgrp->addAction(ui.actionSystemRegionUSA);
	actgrp->addAction(ui.actionSystemRegionEUR);
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
