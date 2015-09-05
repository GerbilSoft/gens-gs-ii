/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQLineEdit.hpp: QLineEdit class with indicator icon.                 *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#include "GensLineEdit.hpp"

#include <QtGui/QLabel>
#include <QtGui/QStyle>

namespace GensQt4 {

GensLineEdit::GensLineEdit(QWidget *parent)
	: super(parent)
{
	m_label = new QLabel(this);
	m_label->setMinimumSize(18, 16);
	m_label->setMaximumSize(18, 16);
	
	// Initialize the label.
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet(QString::fromLatin1("QLineEdit { padding-right: %1px; }").arg(m_label->sizeHint().width() + frameWidth + 1));
	
	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), m_label->sizeHint().height() + frameWidth * 2 + 2),
		       qMax(msz.height(), m_label->sizeHint().height() + frameWidth * 2 + 2));
}

void GensLineEdit::resizeEvent(QResizeEvent *)
{
	QSize sz = m_label->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	m_label->move(rect().right() - frameWidth - sz.width(),
		      (rect().bottom() + 1 - sz.height()) / 2);
}

void GensLineEdit::setIcon(const QIcon& icon)
{
	m_icon = icon;
	m_label->setPixmap(icon.pixmap(16, 16));
}

}
