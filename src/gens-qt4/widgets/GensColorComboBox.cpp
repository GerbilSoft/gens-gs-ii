/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensColorComboBox.cpp: QComboBox class with color support.              *
 * Adds convenience functions to add color selection entries.              *
 *                                                                         *
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

#include "GensColorComboBox.hpp"

// Qt includes.
#include <QtGui/QPainter>

namespace GensQt4 {

/**
 * Add an item to the GensColorComboBox.
 * @param color Color.
 * @param text Text.
 */
void GensColorComboBox::addItem(const QColor& color, const QString& text)
{
	// Create the icon for this item.
	// Currently uses the QComboBox's preset icon size.
	QSize size = this->iconSize();
	QPixmap pxm(size);
	pxm.fill(color);

	// Draw a black rectangle outline.
	// TODO: Use a darkened version of the color like Mac OS X's NSColorWell?
	QPainter painter(&pxm);
	painter.setPen(QColor(Qt::black));
	painter.drawRect(0, 0, size.width()-1, size.height()-1);

	// Convert the QPixmap to a QIcon and add the item.
	QIcon icon(pxm);
	super::addItem(icon, text, color);
}

/**
 * Add an item to the GensColorComboBox.
 * @param color Standard color.
 */
void GensColorComboBox::addItem(Qt::GlobalColor color)
{
	QString text;

	// http://doc.qt.nokia.com/4.7/qt.html#GlobalColor-enum
	// TODO: Add options for VGA colors instead of Qt standard colors?
	switch (color) {
		case Qt::white:		text = tr("White"); break;
		case Qt::black:		text = tr("Black"); break;
		case Qt::red:		text = tr("Red"); break;
		case Qt::darkRed:	text = tr("Dark Red"); break;
		case Qt::green:		text = tr("Green"); break;
		case Qt::darkGreen:	text = tr("Dark Green"); break;
		case Qt::blue:		text = tr("Blue"); break;
		case Qt::darkBlue:	text = tr("Dark Blue"); break;
		case Qt::cyan:		text = tr("Cyan"); break;
		case Qt::darkCyan:	text = tr("Dark Cyan"); break;
		case Qt::magenta:	text = tr("Magenta"); break;
		case Qt::darkMagenta:	text = tr("Dark Magenta"); break;
		case Qt::yellow:	text = tr("Yellow"); break;
		case Qt::darkYellow:	text = tr("Dark Yellow"); break;
		case Qt::gray:		text = tr("Gray"); break;
		case Qt::darkGray:	text = tr("Dark Gray"); break;
		case Qt::lightGray:	text = tr("Light Gray"); break;
		case Qt::transparent:	text = tr("Transparent"); break;
		default:
			break;
	}

	QColor qcolor(color);
	addItem(color, text);
}


/**
 * Get the color of the specified item.
 * @param i Item index.
 * @return QColor, or invalid QColor on error.
 */
QColor GensColorComboBox::itemColor(int i)
{
	if (i < 0 || i >= this->count())
		return QColor();

	QVariant data = this->itemData(i);
	if (!data.isValid())
		return QColor();

	return data.value<QColor>();
}

}
