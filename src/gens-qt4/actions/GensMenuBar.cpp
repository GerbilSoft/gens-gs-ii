/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar.cpp: Gens Menu Bar class.                                   *
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

// C includes.
#include <assert.h>

// Menu definitions.
#include "GensMenuBar_menus.hpp"

// GensQApplication::IconFromTheme()
#include "../GensQApplication.hpp"

// gqt4_config
#include "../gqt4_main.hpp"

// Needed for KeyValMToQtKey().
#include "../Input/KeyHandlerQt.hpp"

// GensMenuBarPrivate
#include "GensMenuBar_p.hpp"

// Recent ROMs menu.
#include "RecentRomsMenu.hpp"

namespace GensQt4
{

/*********************************
 * GensMenuBarPrivate functions. *
 *********************************/

GensMenuBarPrivate::GensMenuBarPrivate(GensMenuBar *q)
	: q(q)
	, m_lockCnt(0)
	, emuManager(NULL)
	, recentRomsMenu(NULL)
	, m_signalMapper(new QSignalMapper(q))
{ }


GensMenuBarPrivate::~GensMenuBarPrivate()
{
	// Clear the menu maps.
	clearHashTables();
	
	// Delete the popup menu.
	delete popupMenu;
	
	// Delete the "Recent ROMs" menu.
	delete recentRomsMenu;
}


/**
 * GensMenuBarPrivate::init(): Initialize GensMenuBarPrivate.
 * @param initEmuManager Initial EmuManager class.
 */
void GensMenuBarPrivate::init(EmuManager *initEmuManager)
{
	// Set the Emulation Manager.
	setEmuManager(initEmuManager);
	
	// Connect the QSignalMapper's mapped() signal.
	QObject::connect(m_signalMapper, SIGNAL(mapped(int)),
			 q, SLOT(menuItemSelected(int)));
	
	// Create the "Recent ROMs" menu.
	recentRomsMenu = new RecentRomsMenu(NULL, gqt4_cfg->recentRomsObject());
	QObject::connect(recentRomsMenu, SIGNAL(updated()),
			 q, SLOT(recentRoms_updated()));
	QObject::connect(recentRomsMenu, SIGNAL(triggered(int)),
			 q, SLOT(menuItemSelected(int)));
	
	// Create the popup menu.
	popupMenu = new QMenu();
	retranslate();
}


/**
 * GensMenuBarPrivate::setEmuManager(): Set the emulation manager.
 * @param newEmuManager New emulation manager.
 */
void GensMenuBarPrivate::setEmuManager(EmuManager *newEmuManager)
{
	if (emuManager == newEmuManager)
		return;
	
	if (emuManager)
	{
		// Disconnect emulation manager signals.
		QObject::disconnect(emuManager, SIGNAL(stateChanged(void)),
				    q, SLOT(stateChanged(void)));
	}
	
	emuManager = newEmuManager;
	if (emuManager)
	{
		// Connect emulation manager signals.
		QObject::connect(emuManager, SIGNAL(stateChanged(void)),
				 q, SLOT(stateChanged(void)));
	}
	
	// Emulation state has changed.
	q->stateChanged();
}


/**
 * GensMenuBarPrivate::clearHashTables(): Clear the menu hash tables and lists.
 * - hashActions: Hash table of menu actions.
 * - m_lstSeparators: List of menu separators.
 */
void GensMenuBarPrivate::clearHashTables(void)
{
	// TODO: Consider using QScopedPointer or QSharedPointer instead?
	
	// Actions map.
	foreach (QAction *action, hashActions)
		delete action;
	hashActions.clear();
	
	// Separators list.
	while (!m_lstSeparators.isEmpty())
		delete m_lstSeparators.takeFirst();
	m_lstSeparators.clear();
}


/**
 * GensMenuBarPrivate::retranslate(): Retranslate the menus.
 */
inline void GensMenuBarPrivate::retranslate(void)
{
	// (Re-)Populate the popup menu.
	parseMainMenu(&GensMenuBar::ms_gmmiMain[0]);
	
	// Synchronization.
	syncAll();	// Synchronize the menus.
	syncConnect();	// Connect synchronization slots.
}


/**
 * GensMenuBarPrivate::parseMainMenu(): Parse an array of GensMainMenuItem items.
 * The menus are added to m_popupMenu.
 * @param mainMenu Pointer to the first item in the GensMainMenuItem array.
 */
void GensMenuBarPrivate::parseMainMenu(const GensMenuBar::MainMenuItem *mainMenu)
{
	QMenu *mnuSubMenu;
	
	// Clear everything.
	clearHashTables();	// Clear the menu hash tables and lists.
	popupMenu->clear();	// Clear the popup menu.
	
	for (; mainMenu->id != 0; mainMenu++)
	{
		// Create a new submenu.
		mnuSubMenu = new QMenu();
		mnuSubMenu->setTitle(GensMenuBar::tr(mainMenu->text));
		
		// Parse the menu.
		parseMenu(mainMenu->submenu, mnuSubMenu);
		
		// Add the menu to the popup menu.
		popupMenu->addMenu(mnuSubMenu);
	}
}


/**
 * GensMenuBarPrivate::parseMenuBar(): Parse an array of GensMenuItem items.
 * @param menu Pointer to the first item in the GensMenuItem array.
 * @param parent QMenu to add the menu items to.
 */
void GensMenuBarPrivate::parseMenu(const GensMenuBar::MenuItem *menu, QMenu *parent)
{
	QAction *mnuItem;
	QMenu *mnuSubMenu;			// QMenu for GMI_SUBMENU items.
	QActionGroup *actionGroup = NULL;	// QActionGroup for GMI_RADIO items.
	
	for (; menu->id != 0; menu++)
	{
		// TODO: Add other menu item types.
		// For now, only GMI_NORMAL and GMI_SEPARATOR are supported.
		
		if (menu->type == GensMenuBar::GMI_SEPARATOR)
		{
			// Menu separator.
			m_lstSeparators.append(parent->addSeparator());
			continue;
		}
		
		mnuItem = new QAction(q);
		mnuItem->setText(GensMenuBar::tr(menu->text));
		
		switch (menu->type)
		{
			case GensMenuBar::GMI_CHECK:
				mnuItem->setCheckable(true);
				break;
			
			case GensMenuBar::GMI_SUBMENU:
				// Parse the submenu.
				mnuSubMenu = new QMenu(parent);
				mnuSubMenu->setTitle(GensMenuBar::tr(menu->text));
				parseMenu(menu->submenu, mnuSubMenu);
				mnuItem->setMenu(mnuSubMenu);
				break;
			
			case GensMenuBar::GMI_RADIO:
				// Check for the QActionGroup.
				// TODO: Do we need to save references to QActionGroup to delete them later?
				if (!actionGroup)
				{
					// Not currently in a QActionGroup.
					actionGroup = new QActionGroup(q);
					actionGroup->setExclusive(true);
				}
				
				// Mark the menu item as a radio button.
				// (checkable == true; part of exclusive QActionGroup)
				mnuItem->setCheckable(true);
				actionGroup->addAction(mnuItem);
				break;
			
			default:
				break;
		}
		
		if (menu->type != GensMenuBar::GMI_RADIO)
			actionGroup = NULL;
		
#ifndef __APPLE__
		// Set the menu icon.
		// (This isn't done on Mac OS X, since icons in menus look out of place there.)
		if (menu->icon_fdo)
			mnuItem->setIcon(GensQApplication::IconFromTheme(QLatin1String(menu->icon_fdo)));
#endif /* __APPLE__ */
		
		// Set the shortcut key.
		GensKey_t gensKey = gqt4_cfg->actionToKey(menu->id);
		mnuItem->setShortcut(KeyHandlerQt::KeyValMToQtKey(gensKey));
		
		// Connect the signal to the signal mapper.
		QObject::connect(mnuItem, SIGNAL(triggered()),
				 this->m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(mnuItem, menu->id);
		
		// Add the menu item to the menu.
		parent->addAction(mnuItem);
		
		// Add the QAction to the actions map.
		hashActions.insert(menu->id, mnuItem);
	}
}


/**
 * lock(), unlock(): Temporarily lock menu actions.
 * Calls are cumulative; 2 locks requires 2 unlocks.
 * Calling unlock() when not locked will return an error.
 * @return 0 on success; non-zero on error.
 */
int GensMenuBarPrivate::lock(void)
{
	m_lockCnt++;
	return 0;
}

int GensMenuBarPrivate::unlock(void)
{
	assert(m_lockCnt >= 0);
	if (m_lockCnt <= 0)
		return -1;
	
	m_lockCnt--;
	return 0;
}

/**
 * GensMenuBarPrivate::isLocked(): Check if the menu actions are locked.
 * @return True if the menu actions are locked; false otherwise.
 */
bool GensMenuBarPrivate::isLocked(void)
	{ return (m_lockCnt > 0); }


/**************************
 * GensMenuBar functions. *
 **************************/

GensMenuBar::GensMenuBar(QObject *parent, EmuManager *emuManager)
	: QObject(parent)
	, d(new GensMenuBarPrivate(this))
{
	// Initialize GensMenuBarPrivate.
	d->init(emuManager);
}


GensMenuBar::~GensMenuBar()
{
	delete d;
}


/**
 * GensMenuBar::popupMenu(): Get the popup menu.
 * @return Popup menu.
 */
QMenu *GensMenuBar::popupMenu(void)
	{ return d->popupMenu; }


/**
 * GensMenuBar::createMenuBar(): Create a menu bar.
 * @return QMenuBar containing the Gens menus.
 */
QMenuBar *GensMenuBar::createMenuBar(void)
{
	QMenuBar *menuBar = new QMenuBar();
	
	foreach(QAction* action, d->popupMenu->actions())
		menuBar->addAction(action);
	
	return menuBar;
}


/**
 * GensMenuBarPrivate::retranslate(): Retranslate the menus.
 * WRAPPER FUNCTION for GensMenuBarPrivate::retranslate().
 */
void GensMenuBar::retranslate(void)
	{ d->retranslate(); }


/**
 * GensMenuBar::menuItemCheckState(): Get a menu item's check state.
 * @param id Menu item ID.
 * @return True if checked; false if not checked or not checkable.
 */
bool GensMenuBar::menuItemCheckState(int id)
{
	QAction *mnuItem = d->hashActions.value(id, NULL);
	if (!mnuItem)
		return false;
	
	// TODO: Is the isCheckable() check needed?
	if (!mnuItem->isCheckable())
		return false;
	return mnuItem->isChecked();
}


/**
 * GensMenuBar::setMenuItemCheckState(): Set a menu item's check state.
 * @param id Menu item ID.
 * @param newCheck New check state.
 * @return 0 on success; non-zero on error.
 */
int GensMenuBar::setMenuItemCheckState(int id, bool newCheck)
{
	QAction *mnuItem = d->hashActions.value(id, NULL);
	if (!mnuItem)
		return -1;
	
	// TODO: Is the isCheckable() check needed?
	if (!mnuItem->isCheckable())
		return -2;
	
	mnuItem->setChecked(newCheck);
	return 0;
}


/**
 * lock(), unlock(): Temporarily lock menu actions.
 * WRAPPER FUNCTIONS for GensMenuBarPrivate.
 * Calls are cumulative; 2 locks requires 2 unlocks.
 * Calling unlock() when not locked will return an error.
 * @return 0 on success; non-zero on error.
 */
int GensMenuBar::lock(void)
	{ return d->lock(); }
int GensMenuBar::unlock(void)
	{ return d->unlock(); }

/**
 * GensMenuBar::isLocked(): Check if the menu actions are locked.
 * WRAPPER FUNCTION for GensMenuBarPrivate::isLocked().
 * @return True if the menu actions are locked; false otherwise.
 */
bool GensMenuBar::isLocked(void)
	{ return d->isLocked(); }


/**
 * GensMenuBar::menuItemSelected(): A menu item has been selected.
 * @param id Menu item ID.
 */
void GensMenuBar::menuItemSelected(int id)
{
	if (d->isLocked())
		return;
	
	// Get the menu item check state.
	bool state = menuItemCheckState(id);
	
	// Emit the menu item triggered() signal.
	emit triggered(id, state);
}

}
