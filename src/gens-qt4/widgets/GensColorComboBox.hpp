/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensColorComboBox.hpp: QComboBox class with color support.              *
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

#ifndef __GENS_QT4_WIDGETS_GENSCOLORCOMBOBOX_HPP__
#define __GENS_QT4_WIDGETS_GENSCOLORCOMBOBOX_HPP__

#include <QtGui/QComboBox>
#include <QtGui/QColor>

namespace GensQt4 {

class GensColorComboBox : public QComboBox
{
	Q_OBJECT
	
	public:
		GensColorComboBox(QWidget *parent = 0)
			: super(parent) { }

	private:
		typedef QComboBox super;
	private:
		Q_DISABLE_COPY(GensColorComboBox)

	public:
		void addItem(const QColor& color, const QString& text);
		void addItem(Qt::GlobalColor color);

		QColor itemColor(int i);
};

}

#endif /* __GENS_QT4_WIDGETS_GENSCOLORCOMBOBOX_HPP__ */
