/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRomsMenu.hpp: Recent ROMs Menu.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

// Qt includes.
#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include <QtGui/QAction>

namespace GensQt4 {

class RecentRomsMenuPrivate
{
	public:
		RecentRomsMenuPrivate(RecentRomsMenu *q, const RecentRoms *initRecentRoms);

	private:
		RecentRomsMenu *const q_ptr;
		Q_DECLARE_PUBLIC(RecentRomsMenu)
	private:
		Q_DISABLE_COPY(RecentRomsMenuPrivate)

	public:
		void init(void);

		void setRecentRoms(const RecentRoms *newRecentRoms);
		void update(void);

	private:
		QSignalMapper *m_signalMapper;

	public:
		// Recent ROMs class.
		const RecentRoms *recentRoms;
};

/** RecentRomsMenuPrivate **/

RecentRomsMenuPrivate::RecentRomsMenuPrivate(RecentRomsMenu *q, const RecentRoms *initRecentRoms)
	: q_ptr(q)
	, m_signalMapper(new QSignalMapper(q))
	, recentRoms(initRecentRoms)
{ }

/**
 * Initialize RecentRomsMenuPrivate.
 */
void RecentRomsMenuPrivate::init(void)
{
	Q_Q(RecentRomsMenu);

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
			 q, SLOT(recentRomsDestroyed_slot()));
	QObject::connect(recentRoms, SIGNAL(updated()),
			 q, SLOT(recentRomsUpdated_slot()));
}


/**
 * Set the Recent ROMs class this menu should represent.
 * @param newRecentRoms New Recent ROMs class, or nullptr to unset.
 */
inline void RecentRomsMenuPrivate::setRecentRoms(const RecentRoms *newRecentRoms)
{
	if (this->recentRoms == newRecentRoms)
		return;

	Q_Q(RecentRomsMenu);

	if (recentRoms) {
		// Disconnect the signals from the existing Recent ROMs class.
		QObject::disconnect(recentRoms, SIGNAL(destroyed()),
				    q, SLOT(recentRomsDestroyed()));
		QObject::disconnect(recentRoms, SIGNAL(updated()),
				    q, SLOT(recentRomsupdated()));
	}

	// Set the new Recent ROMs class.
	recentRoms = newRecentRoms;

	if (recentRoms) {
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
	Q_Q(RecentRomsMenu);

	// Delete all existing QActions.
	qDeleteAll(q->actions());
	q->actions().clear();

	// If we don't have a Recent ROMs class, don't do anything.
	if (!recentRoms) {
		emit q->updated();
		return;
	}

	// Create new QActions from the Recent ROMs list.
	// TODO: Move the ROM format prefixes somewhere else.
	static const char RomFormatPrefix[][8] =
	{
		"---", "MD", "MCD", "32X",
		"MCD32X", "SMS", "GG", "SG",
		"Pico"
	};

	int i = 1;
	foreach (const RecentRom_t& rom, recentRoms->romList()) {
		QString title;
		title.reserve(rom.filename.size() + rom.z_filename.size() + 16);

		// Recent ROM number.
		title += QChar(L'&') + QString::number(i) + QChar(L' ');

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
		title += QChar(L' ') + filename;

		if (!rom.z_filename.isEmpty()) {
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
		// TODO: Port configurable shortcuts to the new menu system/
		/*
		const int mnuItemId = ((IDM_FILE_RECENT_1 - 1) + i);
		GensKey_t gensKey = gqt4_cfg->actionToKey(mnuItemId);
		action->setShortcut(KeyHandlerQt::KeyValMToQtKey(gensKey));
		*/
		action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0 + i));

		// Connect the signal to the signal mapper.
		QObject::connect(action, SIGNAL(triggered()),
				 m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(action, i);

		// Add the action to the RecentRomsMenu.
		q->addAction(action);

		// Next ROM.
		i++;
	}

	// Recent ROMs menu has been updated.
	emit q->updated();
}

/** RecentRomsMenu **/

RecentRomsMenu::RecentRomsMenu(QWidget* parent, const RecentRoms* recentRoms)
	: QMenu(parent)
	, d_ptr(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	Q_D(RecentRomsMenu);
	d->init();
}

RecentRomsMenu::RecentRomsMenu(const QString &title, QWidget *parent, const RecentRoms *recentRoms)
	: QMenu(title, parent)
	, d_ptr(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	Q_D(RecentRomsMenu);
	d->init();
}

RecentRomsMenu::~RecentRomsMenu()
{
	delete d_ptr;

	// Make sure all of the QActions are deleted.
	qDeleteAll(this->actions());
	this->actions().clear();
}

/**
 * Get the Recent ROMs class this menu is representing.
 * @return Recent ROMs class, or nullptr if no class is assigned.
 */
const RecentRoms *RecentRomsMenu::recentRoms(void)
{
	Q_D(RecentRomsMenu);
	return d->recentRoms;
}

/**
 * Set the Recent ROMs class this menu should represent.
 * WRAPPER FUNCTION for RecentRomsMenuPrivate::isDirty().
 * @param newRecentRoms New Recent ROMs class, or nullptr to unset.
 */
void RecentRomsMenu::setRecentRoms(const RecentRoms *newRecentRoms)
{
	Q_D(RecentRomsMenu);
	d->setRecentRoms(newRecentRoms);
}

/**
 * d->recentRoms was destroyed.
 */
void RecentRomsMenu::recentRomsDestroyed_slot(void)
{
	Q_D(RecentRomsMenu);
	d->recentRoms = nullptr;
}

/**
 * The Recent ROMs class has been updated.
 * Call d->update() to update the menu.
 */
void RecentRomsMenu::recentRomsUpdated_slot(void)
{
	Q_D(RecentRomsMenu);
	d->update();
}

}
