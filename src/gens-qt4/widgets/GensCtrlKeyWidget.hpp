/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlKeyWidget.hpp: Controller key input widget.                     *
 * Based on KKeySequenceWidget from KDE 4.6.0.                             *
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

#ifndef __GENS_QT4_WIDGETS_GENSCTRLKEYWIDGET_HPP__
#define __GENS_QT4_WIDGETS_GENSCTRLKEYWIDGET_HPP__

#include "libgens/GensInput/GensKey_t.h"

// Qt includes and classes.
#include <QtGui/QWidget>
class QLabel;

namespace GensQt4
{

class GensCtrlKeyWidgetPrivate;

class GensCtrlKeyWidget : public QWidget
{
	Q_OBJECT
	
	public:
		explicit GensCtrlKeyWidget(QWidget *parent = 0, QLabel *label = 0);
		~GensCtrlKeyWidget();
		
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);
		
		/**
		 * Set the display label.
		 */
		void setLabel(QLabel *label);
		
		/**
		 * Return the currently selected GensKey_t.
		 */
		GensKey_t key() const;
	
	signals:
		void keyChanged(GensKey_t key);
	
	public slots:
		/**
		 * Capture a GensKey_t. This call will only return once a GensKey_t
		 * has been captured or input was aborted.
		 * If a key sequence was input, keySequenceChanged() will be emitted.
		 */
		void captureKey(void);
		
		/**
		 * Set the current GensKey_t.
		 */
		void setKey(GensKey_t key);
		
		/**
		 * Clear the current GensKey_t.
		 */
		void clearKey(void);
	
	private slots:
		// NOTE: We can't use Q_PRIVATE_SLOT() properly without KDE's automoc4.
		//Q_PRIVATE_SLOT(d, void captureKeyTimeout(void))
		void captureKeyTimeout(void);
		
		// Blink the label.
		void blinkLabel(void);
		
		// lblDisplay destroyed slot.
		void labelDestroyed(void);
	
	private:
		friend class GensCtrlKeyWidgetPrivate;
		GensCtrlKeyWidgetPrivate *const d;
		
		Q_DISABLE_COPY(GensCtrlKeyWidget)
};

}

#endif /* __GENS_QT4_WIDGETS_GENSCTRLKEYWIDGET_HPP__ */
