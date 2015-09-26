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

#include "libgens/Rom.hpp"
using LibGens::Rom;

// EmuManager::SysAbbrev()
#include "../EmuManager.hpp"

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
 * Update the Recent ROMs menu.
 */
void RecentRomsMenuPrivate::update(void)
{
	Q_Q(RecentRomsMenu);

	// Delete all existing QActions.
	// TODO: Reuse existing QActions if possible.
	qDeleteAll(q->actions());
	q->actions().clear();

	// If we don't have a Recent ROMs class, don't do anything.
	if (!recentRoms) {
		emit q->updated();
		return;
	}

	// Create new QActions from the Recent ROMs list.

	//: %1 = menu index; %2 = system abbreviation; %3 = filename.
	const QString title_template = RecentRoms::tr("&%1 [%2] %3");
	//: %1 = menu index; %2 = system abbreviation; %3 = filename; %4 = filename inside archive.
	const QString title_z_template = RecentRoms::tr("&%1 [%2] %3 [%4]");

	int i = 1;
	// QStrings are placed here to avoid unnecessary initialization.
	QString title;
	QString filename;
	foreach (const RecentRom_t& rom, recentRoms->romList()) {
		// System ID.
		QString sysAbbrev = EmuManager::SysAbbrev(rom.sysId);
		if (sysAbbrev.isEmpty()) {
			sysAbbrev = QLatin1String("---");
		}

		// Remove directories from the filename.
		// TODO: Use LibGensText::FilenameBase() once it's merged.
		QString filename = QDir::fromNativeSeparators(rom.filename);
		int slash_pos = filename.lastIndexOf(QChar(L'/'));
		if (slash_pos >= 0) {
			filename.remove(0, (slash_pos + 1));
		}

		// Escape ampersands in the ROM filename.
		filename.replace(QChar(L'&'), QLatin1String("&&"));

		if (!rom.z_filename.isEmpty()) {
			// ROM has a compressed filename.
			// Escape the ampersands and append the compressed filename.
			QString z_filename = rom.z_filename;
			z_filename.replace(QChar(L'&'), QLatin1String("&&"));
			title = title_z_template
				.arg(i)			// ROM index.
				.arg(sysAbbrev)		// System abbreviation.
				.arg(filename)		// ROM filename, minus directories.
				.arg(z_filename);	// Compressed filename.
		} else {
			// No compressed filename.
			title = title_template
				.arg(i)			// ROM index.
				.arg(sysAbbrev)		// System abbreviation.
				.arg(filename);		// ROM filename, minus directories.
		}

		// Create the QAction.
		QAction *action = new QAction(title, q);
		// Set the objectName in order to handle shortcuts.
		action->setObjectName(QLatin1String("actionRecentROMs_") +
				      QString::number(i));

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
	: super(parent)
	, d_ptr(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	Q_D(RecentRomsMenu);
	d->init();
}

RecentRomsMenu::RecentRomsMenu(const QString &title, QWidget *parent, const RecentRoms *recentRoms)
	: super(title, parent)
	, d_ptr(new RecentRomsMenuPrivate(this, recentRoms))
{
	// Initialize RecentRomsMenuPrivate.
	Q_D(RecentRomsMenu);
	d->init();
}

RecentRomsMenu::~RecentRomsMenu()
{
	delete d_ptr;
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
 * @param recentRoms New Recent ROMs class, or nullptr to unset.
 */
void RecentRomsMenu::setRecentRoms(const RecentRoms *recentRoms)
{
	Q_D(RecentRomsMenu);
	if (d->recentRoms == recentRoms)
		return;

	if (d->recentRoms) {
		// Disconnect the signals from the existing Recent ROMs class.
		QObject::disconnect(d->recentRoms, SIGNAL(destroyed()),
				    this, SLOT(recentRomsDestroyed_slot()));
		QObject::disconnect(d->recentRoms, SIGNAL(updated()),
				    this, SLOT(recentRomsUpdated_slot()));
	}

	// Set the new Recent ROMs class.
	d->recentRoms = recentRoms;

	if (recentRoms) {
		// Connect the signals from the new Recent ROMs class.
		QObject::connect(recentRoms, SIGNAL(destroyed()),
				 this, SLOT(recentRomsDestroyed_slot()));
		QObject::connect(recentRoms, SIGNAL(updated()),
				 this, SLOT(recentRomsUpdated_slot()));
	}

	// Update the menu.
	update();
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
