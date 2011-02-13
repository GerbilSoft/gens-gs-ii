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

// Menu definitions.
#include "GensMenuBar_menus.hpp"

// GensQApplication::IconFromTheme()
#include "../GensQApplication.hpp"


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
	parseMainMenu(&ms_gmmiMain[0]);
}

GensMenuBar::~GensMenuBar()
{
	// Delete the signal mapper.
	delete m_signalMapper;
	
	// Clear the menu maps.
	clearHashTables();
}


/**
 * clearHashTables(): Clear the menu hash tables and lists.
 * - m_hashActions: Hash table of menu actions.
 * - m_lstSeparators: List of menu separators.
 * - m_hashMenus: Hash table of top-level menus.
 */
void GensMenuBar::clearHashTables(void)
{
	// TODO: Consider using QScopedPointer or QSharedPointer instead?
	
	// Actions map.
	QAction *action;
	foreach(action, m_hashActions)
		delete action;
	m_hashActions.clear();
	
	// Separators list.
	while (!m_lstSeparators.isEmpty())
		delete m_lstSeparators.takeFirst();
	
	// Menus map.
	QMenu *menu;
	foreach(menu, m_hashMenus)
		delete menu;
	m_hashMenus.clear();
}


/**
 * parseMainMenu(): Parse an array of GensMainMenuItem items.
 * @param mainMenu Pointer to the first item in the GensMainMenuItem array.
 */
void GensMenuBar::parseMainMenu(const GensMenuBar::MainMenuItem *mainMenu)
{
	QMenu *mnuSubMenu;
	
	// Clear the menu hash tables and lists.
	clearHashTables();
	
	for (; mainMenu->id != 0; mainMenu++)
	{
		// Create a new submenu.
		mnuSubMenu = new QMenu(this);
		mnuSubMenu->setTitle(tr(mainMenu->text));
		
		// Parse the menu.
		parseMenu(mainMenu->submenu, mnuSubMenu);
		
		// Add the menu to the menu bar.
		this->addMenu(mnuSubMenu);
		
		// Add the QMenu to the menus map.
		m_hashMenus.insert(mainMenu->id, mnuSubMenu);
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
	QMenu *mnuSubMenu;			// QMenu for GMI_SUBMENU items.
	QActionGroup *actionGroup = NULL;	// QActionGroup for GMI_RADIO items.
	
	for (; menu->id != 0; menu++)
	{
		// TODO: Add other menu item types.
		// For now, only GMI_NORMAL and GMI_SEPARATOR are supported.
		
		if (menu->type == GMI_SEPARATOR)
		{
			// Menu separator.
			m_lstSeparators.append(parent->addSeparator());
			continue;
		}
		
		mnuItem = new QAction(parent);
		mnuItem->setText(tr(menu->text));
		
		switch (menu->type)
		{
			case GMI_CHECK:
				mnuItem->setCheckable(true);
				break;
			
			case GMI_SUBMENU:
				// Parse the submenu.
				mnuSubMenu = new QMenu(this);
				mnuSubMenu->setTitle(tr(menu->text));
				parseMenu(menu->submenu, mnuSubMenu);
				mnuItem->setMenu(mnuSubMenu);
				m_hashMenus.insert(menu->submenu_id, mnuSubMenu);
				break;
			
			case GMI_RADIO:
				// Check for the QActionGroup.
				// TODO: Do we need to save references to QActionGroup to delete them later?
				if (!actionGroup)
				{
					// Not currently in a QActionGroup.
					actionGroup = new QActionGroup(this);
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
		
		if (menu->type != GMI_RADIO)
			actionGroup = NULL;
		
#ifndef __APPLE__
		// Set the menu icon.
		// (This isn't done on Mac OS X, since icons in menus look out of place there.)
		if (menu->icon_fdo)
			mnuItem->setIcon(GensQApplication::IconFromTheme(QLatin1String(menu->icon_fdo)));
#endif /* __APPLE__ */
		
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
		
		// Add the QAction to the actions map.
		m_hashActions.insert(menu->id, mnuItem);
	}
}


/**
 * menuItemCheckState(): Get a menu item's check state.
 * @param id Menu item ID.
 * @return True if checked; false if not checked or not checkable.
 */
bool GensMenuBar::menuItemCheckState(int id)
{
	QAction *mnuItem = m_hashActions.value(id, NULL);
	if (!mnuItem)
		return false;
	
	// TODO: Is the isCheckable() check needed?
	if (!mnuItem->isCheckable())
		return false;
	return mnuItem->isChecked();
}


/**
 * setMenuItemCheckState(): Set a menu item's check state.
 * @param id Menu item ID.
 * @param newCheck New check state.
 * @return 0 on success; non-zero on error.
 */
int GensMenuBar::setMenuItemCheckState(int id, bool newCheck)
{
	QAction *mnuItem = m_hashActions.value(id, NULL);
	if (!mnuItem)
		return -1;
	
	// TODO: Is the isCheckable() check needed?
	if (!mnuItem->isCheckable())
		return -2;
	
	mnuItem->setChecked(newCheck);
	return 0;
}

}
