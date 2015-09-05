/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensKeySequenceWidget._phpp: Key sequence input widget.                 *
 * Minimal reimplementation of KKeySequenceWidget from KDE 4.6.0.          *
 * (PRIVATE HEADER)                                                        *
 *                                                                         *
 * Copyright (c) 1998 Mark Donohoe <donohoe@kde.org>                       *
 * Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>                      *
 * Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>               *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_P_HPP__
#define __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_P_HPP__

#include <QtGui/QPushButton>

namespace GensQt4 {

class GensKeySequenceWidgetPrivate;

class GensKeySequenceButton : public QPushButton
{
	Q_OBJECT

	public:
		explicit GensKeySequenceButton(GensKeySequenceWidgetPrivate *d, QWidget *parent = 0)
			: QPushButton(parent), d(d) { }
		virtual ~GensKeySequenceButton() { };

	private:
		typedef QPushButton super;
		GensKeySequenceWidgetPrivate *const d;
		Q_DISABLE_COPY(GensKeySequenceButton)

	public:
		// Reimplemented. [NOTE: NOT VIRTUAL!]
		void setText(const QString& text);

	protected:
		// Reimplemented for internal reasons.
		virtual bool event(QEvent *event) override;
		virtual void keyPressEvent(QKeyEvent *event) override;
		virtual void keyReleaseEvent(QKeyEvent *event) override;
		virtual void focusOutEvent(QFocusEvent *event) override;
};

}

#endif /* __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_P_HPP__ */
