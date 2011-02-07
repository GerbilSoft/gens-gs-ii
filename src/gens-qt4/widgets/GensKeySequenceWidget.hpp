/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensKeySequenceWidget.hpp: Key sequence input widget.                   *
 * Minimal reimplementation of KKeySequenceWidget from KDE 4.6.0.          *
 *                                                                         *
 * Copyright (c) 1998 Mark Donohoe <donohoe@kde.org>                       *
 * Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>                      *
 * Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>               *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_HPP__
#define __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_HPP__

#include <QtGui/QWidget>
#include <QtGui/QKeySequence>

namespace GensQt4
{

class GensKeySequenceWidgetPrivate;

class GensKeySequenceWidget : public QWidget
{
	Q_OBJECT
	
	public:
		explicit GensKeySequenceWidget(QWidget *parent = 0);
		~GensKeySequenceWidget();
		
		/**
		 * Return the currently selected key sequence.
		 */
		QKeySequence keySequence() const;
	
	signals:
		void keySequenceChanged(const QKeySequence& seq);
	
	public slots:
		/**
		 * Capture a shortcut from the keyboard. This call will only return once a key sequence
		 * has been captured or input was aborted.
		 * If a key sequence was input, keySequenceChanged() will be emitted.
		 */
		void captureKeySequence(void);
		
		/**
		 * Set the key sequence.
		 */
		void setKeySequence(const QKeySequence& seq);
		
		/**
		 * Clear the key sequence.
		 */
		void clearKeySequence(void);
	
	private slots:
		// NOTE: We can't use Q_PRIVATE_SLOT() properly without KDE's automoc4.
		//Q_PRIVATE_SLOT(d, void doneRecording(void))
		void doneRecording(void);
	
	private:
		friend class GensKeySequenceWidgetPrivate;
		GensKeySequenceWidgetPrivate *const d;
		
		Q_DISABLE_COPY(GensKeySequenceWidget);
};

}

#endif /* __GENS_QT4_WIDGETS_GENSKEYSEQUENCEWIDGET_HPP__ */
