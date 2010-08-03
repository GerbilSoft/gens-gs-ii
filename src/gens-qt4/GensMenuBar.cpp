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
	// Create the signal mapper.
	m_signalMapper = new QSignalMapper(this);
	connect(this->m_signalMapper, SIGNAL(mapped(int)),
		this, SIGNAL(triggered(int)));
	
	// Populate the menu bar.
	// TODO
	
	static const MenuItem gmiFile[] =
	{
		{IDM_FILE_BLIT, GMI_NORMAL, "Blit!", NULL, MACCEL_NONE, Qt::CTRL + Qt::Key_B, NULL, NULL},
		{IDM_FILE_EMUTHREAD, GMI_NORMAL, "Start/Stop EmuThread", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_FILE_QUIT, GMI_NORMAL, "&Quit", NULL, MACCEL_QUIT, Qt::CTRL + Qt::Key_Q, "application-exit", ":/oxygen-16x16/application-exit.png"},
		
		{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
	};
	
	static const MenuItem gmiResTest[] =
	{
		{IDM_RESTEST_1X, GMI_NORMAL, "320x240 (&1x)", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_RESTEST_2X, GMI_NORMAL, "640x480 (&2x)", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_RESTEST_3X, GMI_NORMAL, "960x720 (&3x)", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_RESTEST_4X, GMI_NORMAL, "1280x960 (&4x)", NULL, MACCEL_NONE, 0, NULL, NULL},
		
		{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
	};
	
	static const MenuItem gmiBppTest[] =
	{
		{IDM_BPPTEST_15, GMI_NORMAL, "15-bit (555)", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_BPPTEST_16, GMI_NORMAL, "16-bit (565)", NULL, MACCEL_NONE, 0, NULL, NULL},
		{IDM_BPPTEST_32, GMI_NORMAL, "32-bit (888)", NULL, MACCEL_NONE, 0, NULL, NULL},
		
		{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
	};
	
	static const MenuItem gmiHelp[] =
	{
		{IDM_HELP_ABOUT, GMI_NORMAL, "&About Gens/GS II", NULL, MACCEL_NONE, 0, "help-about", ":/oxygen-16x16/help-about.png"},
		
		{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
	};
	
	static const MainMenuItem gmmiMain[] =
	{
		{IDM_FILE_MENU, "&File", &gmiFile[0]},
		{IDM_RESTEST_MENU, "&ResTest", &gmiResTest[0]},
		{IDM_BPPTEST_MENU, "&BppTest", &gmiBppTest[0]},
		{IDM_HELP_MENU, "&Help", &gmiHelp[0]},
		
		{0, NULL, NULL}
	};
	
	parseMainMenu(&gmmiMain[0]);
}

GensMenuBar::~GensMenuBar()
{
}


/**
 * parseMainMenu(): Parse an array of GensMainMenuItem items.
 * @param mainMenu Pointer to the first item in the GensMainMenuItem array.
 */
void GensMenuBar::parseMainMenu(const GensMenuBar::MainMenuItem *mainMenu)
{
	QMenu *mnuSubMenu;
	
	for (; mainMenu->id != 0; mainMenu++)
	{
		// Create a new submenu.
		mnuSubMenu = new QMenu(this);
		mnuSubMenu->setTitle(TR(mainMenu->text));
		
		// Parse the menu.
		parseMenu(mainMenu->submenu, mnuSubMenu);
		
		// Add the menu to the menu bar.
		this->addMenu(mnuSubMenu);
	}
}


/**
 * parseMenuBar(): Parse an array of GensMenuItem items.
 * @param menu Pointer to the first item in the GensMenuItem array.
 * @param parent QMenu to add the menu items to.
 */
void GensMenuBar::parseMenu(const GensMenuBar::MenuItem *menu, QMenu *parent)
{
	QAction *mnuItem;
	
	for (; menu->id != 0; menu++)
	{
		// TODO: Add other menu item types.
		// For now, only GMI_NORMAL and GMI_SEPARATOR are supported.
		
		if (menu->type == GMI_SEPARATOR)
		{
			// Menu separator.
			parent->addSeparator();
			continue;
		}
		
		if (menu->type != GMI_NORMAL)
			continue;
		
		mnuItem = new QAction(parent);
		mnuItem->setText(TR(menu->text));
		
		// Set the menu icon.
		mnuItem->setIcon(QICON_FROMTHEME(menu->icon_fdo, menu->icon_qrc));
		
		// Set the shortcut key.
		if (menu->key_std != MACCEL_NONE)
		{
			// StandardKey sequence specified.
			QKeySequence key((QKeySequence::StandardKey)menu->key_std);
			if (!key.isEmpty())
			{
				// Key sequence is valid.
				mnuItem->setShortcut(QKeySequence((QKeySequence::StandardKey)menu->key_std));
			}
			else if (menu->key_custom != 0)
			{
				// Key sequence is not valid.
				// Use the custom key sequence.
				mnuItem->setShortcut(menu->key_custom);
			}
		}
		else if (menu->key_custom != 0)
		{
			// Custom key sequence specified.
			mnuItem->setShortcut(menu->key_custom);
		}
		
		// Connect the signal to the signal mapper.
		connect(mnuItem, SIGNAL(triggered()),
			this->m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(mnuItem, menu->id);
		
		// Add the menu item to the menu.
		parent->addAction(mnuItem);
	}
}

}
