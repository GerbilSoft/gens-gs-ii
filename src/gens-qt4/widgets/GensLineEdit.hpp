/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQLineEdit.hpp: QLineEdit class with indicator icon.                 *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
 *                                                                         *
 * Based on Lineedit with a clear button.                                  *
 * http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button/       *
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

#ifndef __GENS_QT4_GENSLINEEDITICON_HPP__
#define __GENS_QT4_GENSLINEEDITICON_HPP__

#include <QtGui/QLineEdit>
#include <QtGui/QIcon>

class QLabel;
class QFocusEvent;

namespace GensQt4
{

class GensLineEdit : public QLineEdit
{
	Q_OBJECT
	
	Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
	
	public:
		GensLineEdit(QWidget *parent = 0);
		
		QIcon icon(void) const { return m_icon; }
		void setIcon(const QIcon& icon);
	
	signals:
		void focusIn(void);
		void focusOut(void);
	
	protected:
		void resizeEvent(QResizeEvent *);
		
		void focusInEvent(QFocusEvent *event)
		{
			this->QLineEdit::focusInEvent(event);
			emit focusIn();
		}
		void focusOutEvent(QFocusEvent *event)
		{
			this->QLineEdit::focusOutEvent(event);
			emit focusOut();
		}
	
	private:
		QLabel *m_label;
		QIcon m_icon;
};

}

#endif /* __GENS_QT4_GENSLINEEDITICON_HPP__ */
