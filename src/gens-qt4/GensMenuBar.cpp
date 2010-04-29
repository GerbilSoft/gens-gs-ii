/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar.cpp: Gens Menu Bar class.                                   *
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

#include "GensMenuBar.hpp"

// Qt includes.
#include <QtGui/QApplication>

// Text translation macro.
#define TR(text) \
	QApplication::translate("GensMenuBar", (text), NULL, QApplication::UnicodeUTF8)

/**
 * QICON_FROMTHEME(): Icon loading function.
 * Qt 4.6 supports FreeDesktop.org icon themes.
 * Older versions do not, unfortunately.
 */
#if QT_VERSION >= 0x040600
#define QICON_FROMTHEME(name, fallback) \
	(QIcon::hasThemeIcon(name) ? QIcon::fromTheme(name) : QIcon(fallback))
#else
#define QICON_FROMTHEME(name, fallback) \
	QIcon(fallback)
#endif

namespace GensQt4
{

GensMenuBar::GensMenuBar(QWidget *parent)
	: QMenuBar::QMenuBar(parent)
{
	// Populate the menu bar.
	// TODO
	
	// TODO: Qt4's preferred method is adding actions to the parent window,
	// then adding actions to menus.
	// We're going to simply add actions to menus.
	QMenu *mnuFile = new QMenu(this);
	mnuFile->setTitle(TR("&File"));
	QAction *mnuFileQuit = new QAction(mnuFile);
	mnuFileQuit->setText(TR("&Quit"));
	mnuFileQuit->setShortcut(QKeySequence(QKeySequence::Quit));
	mnuFileQuit->setIcon(QICON_FROMTHEME("application-exit", ":/oxygen-16x16/application-exit.png"));
	
	mnuFile->addAction(mnuFileQuit);
	this->addMenu(mnuFile);
	
	QMenu *mnuResTest = new QMenu(this);
	mnuResTest->setTitle(TR("&ResTest"));
	QAction *mnuResTest1x = new QAction(mnuResTest);
	mnuResTest1x->setText(TR("320x240 (&1x)"));
	QAction *mnuResTest2x = new QAction(mnuResTest);
	mnuResTest2x->setText(TR("640x480 (&2x)"));
	QAction *mnuResTest3x = new QAction(mnuResTest);
	mnuResTest3x->setText(TR("960x720 (&3x)"));
	QAction *mnuResTest4x = new QAction(mnuResTest);
	mnuResTest4x->setText(TR("1280x960 (&4x)"));
	
	mnuResTest->addAction(mnuResTest1x);
	mnuResTest->addAction(mnuResTest2x);
	mnuResTest->addAction(mnuResTest3x);
	mnuResTest->addAction(mnuResTest4x);
	this->addMenu(mnuResTest);

	QMenu *mnuHelp = new QMenu(this);
	mnuHelp->setTitle(TR("&Help"));
	QAction *mnuHelpAbout = new QAction(mnuFile);
	mnuHelpAbout->setText(TR("&About Gens/GS II"));
	mnuHelpAbout->setIcon(QICON_FROMTHEME("help-about", ":/oxygen-16x16/help-about.png"));
	
	mnuHelp->addAction(mnuHelpAbout);
	this->addMenu(mnuHelp);
}

GensMenuBar::~GensMenuBar()
{
}

}
