/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRomsMenu.hpp: Recent ROMs Menu.                                   *
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

#include "RecentRomsMenu.hpp"

// gqt4_config
#include "gqt4_main.hpp"

// Menu item IDs.
#include "GensMenuBar_menus.hpp"

// Key Handler.
#include "../Input/KeyHandlerQt.hpp"

// Qt includes.
#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include <QtGui/QAction>

namespace GensQt4
{

class RecentRomsMenuPrivate
{
	public:
		RecentRomsMenuPrivate(RecentRomsMenu *q, const RecentRoms *initRecentRoms);
		
		void init(void);
		
		void setRecentRoms(const RecentRoms *newRecentRoms);
		void update(void);
		
	private:
		RecentRomsMenu *const q;
		Q_DISABLE_COPY(RecentRomsMenuPrivate)
		
		QSignalMapper *m_signalMapper;
	
	public:
		// Recent ROMs class.
		const RecentRoms *recentRoms;
};

/************************************
 * RecentRomsMenuPrivate functions. *
 ************************************/

RecentRomsMenuPrivate::RecentRomsMenuPrivate(RecentRomsMenu *q, const RecentRoms *initRecentRoms)
	: q(q)
	, m_signalMapper(new QSignalMapper(q))
	, recentRoms(initRecentRoms)
{ }


/**
 * Initialize RecentRomsMenuPrivate.
 */
void RecentRomsMenuPrivate::init(void)
{
	// Connect the QSignalMapper's mapped() signal.
	QObject::connect(m_signalMapper, SIGNAL(mapped(int)),
			 q, SIGNAL(triggered(int)));
	
	// Update the QActions list.
	update();
	
	if (!recentRoms)
		return;
	
	// Recent ROMs class is initialized.
	// Connect the signals.
	QObject::connect(recentRoms, SIGNAL(destroyed()),
			 q, SLOT(recentRomsDestroyed()));
	QObject::connect(recentRoms, SIGNAL(updated()),
			 q, SLOT(recentRomsUpdated()));
}


/**
 * Set the Recent ROMs class this menu should represent.
 * @param newRecentRoms New Recent ROMs class, or NULL to unset.
 */
inline void RecentRomsMenuPrivate::setRecentRoms(const RecentRoms *newRecentRoms)
{
	if (this->recentRoms == newRecentRoms)
		return;
	
	if (recentRoms)
	{
		// Disconnect the signals from the existing Recent ROMs class.
		QObject::disconnect(recentRoms, SIGNAL(destroyed()),
				    q, SLOT(recentRomsDestroyed()));
		QObject::disconnect(recentRoms, SIGNAL(updated()),
				    q, SLOT(recentRomsupdated()));
	}
	
	// Set the new Recent ROMs class.
	recentRoms = newRecentRoms;
	
	if (recentRoms)
	{
		// Connect the signals from the new Recent ROMs class.
		QObject::connect(recentRoms, SIGNAL(destroyed()),
				 q, SLOT(recentRomsDestroyed()));
		QObject::connect(recentRoms, SIGNAL(updated()),
				 q, SLOT(recentRomsupdated()));
	}
	
	// Update the menu.
	update();
}


/**
 * Update the Recent ROMs menu.
 */
void RecentRomsMenuPrivate::update(void)
{
	// Delete all existing QActions.
	qDeleteAll(q->actions());
	q->actions().clear();
	
	// If we don't have a Recent ROMs class, don't do anything.
	if (!recentRoms)
	{
		emit q->updated();
		return;
	}
	
	// Create new QActions from the Recent ROMs list.
	// TODO: Move the ROM format prefixes somewhere else.
	static const char *RomFormatPrefix[] =
	{
		"---", "MD", "MCD", "32X",
		"MCD32X", "SMS", "GG", "SG",
		"Pico", NULL
	};
	
	int i = 1;
	foreach(const RecentRom_t& rom, recentRoms->romList())
	{
		QString title = QChar(L'&') + QString::number(i) + QChar(L' ');
		
		// System ID.
		title += QChar(L'[');
		if (rom.sysId >= LibGens::Rom::MDP_SYSTEM_UNKNOWN &&
		    rom.sysId < LibGens::Rom::MDP_SYSTEM_MAX)
		{
			title += QLatin1String(RomFormatPrefix[rom.sysId]);
		}
		else
		{
			title += QLatin1String(RomFormatPrefix[LibGens::Rom::MDP_SYSTEM_UNKNOWN]);
		}
		title += QChar(L']');
		
		// Remove directories from the filename.
		QString filename = QDir::fromNativeSeparators(rom.filename);
		int slash_pos = filename.lastIndexOf(QChar(L'/'));
		if (slash_pos >= 0)
			filename.remove(0, (slash_pos + 1));
		
		// Escape ampersands in the ROM filename.
		filename.replace(QChar(L'&'), QLatin1String("&&"));
		
		// Append the processed filename.
		title += QLatin1String("\t ") + filename;
		
		if (!rom.z_filename.isEmpty())
		{
			// ROM has a compressed filename.
			// Escape the ampersands and append the compressed filename.
			filename = rom.z_filename;
			filename.replace(QChar(L'&'), QLatin1String("&&"));
			title += QLatin1String(" [") + filename + QChar(L']');
		}
		
		// Create the QAction.
		QAction *action = new QAction(title, q);
		
		// Set the shortcut key.
		// NOTE: The shortcut key won't show up due to the "\t" after the system ID.
		const int mnuItemId = ((IDM_FILE_RECENT_1 - 1) + i);
		GensKey_t gensKey = gqt4_config->actionToKey(mnuItemId);
		action->setShortcut(KeyHandlerQt::KeyValMToQtKey(gensKey));
		
		// Connect the signal to the signal mapper.
		QObject::connect(action, SIGNAL(triggered()),
				 m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(action, mnuItemId);
		
		// Add the action to the RecentRomsMenu.
		q->addAction(action);
		
		// Next ROM.
		i++;
	}
	
	// Recent ROMs menu has been updated.
	emit q->updated();
}


/*****************************
 * RecentRomsMenu functions. *
 *****************************/

RecentRomsMenu::RecentRomsMenu(QWidget* parent, const RecentRoms* recentRoms)
	: QMenu(parent)
	, d(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	d->init();
}

RecentRomsMenu::RecentRomsMenu(const QString& title, QWidget *parent, const RecentRoms *recentRoms)
	: QMenu(title, parent)
	, d(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	d->init();
}

RecentRomsMenu::~RecentRomsMenu()
{
	delete d;
	
	// Make sure all of the QActions are deleted.
	qDeleteAll(this->actions());
	this->actions().clear();
}


/**
 * Get the Recent ROMs class this menu is representing.
 * @return Recent ROMs class, or NULL if no class is assigned.
 */
const RecentRoms *RecentRomsMenu::recentRoms(void)
	{ return d->recentRoms; }


/**
 * Set the Recent ROMs class this menu should represent.
 * WRAPPER FUNCTION for RecentRomsMenuPrivate::isDirty().
 * @param newRecentRoms New Recent ROMs class, or NULL to unset.
 */
void RecentRomsMenu::setRecentRoms(const RecentRoms *newRecentRoms)
	{ d->setRecentRoms(newRecentRoms); }


/**
 * d->recentRoms was destroyed.
 */
void RecentRomsMenu::recentRomsDestroyed(void)
	{ d->recentRoms = NULL; }


/**
 * The Recent ROMs class has been updated.
 * Call d->update() to update the menu.
 */
void RecentRomsMenu::recentRomsUpdated(void)
	{ d->update(); }

}
