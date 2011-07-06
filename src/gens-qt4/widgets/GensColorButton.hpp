/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensColorButton.hpp: QButton class with background color support.       *
 *                                                                         *
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

#ifndef __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__
#define __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__

#include <QtGui/QPushButton>
#include <QtGui/QColor>

namespace GensQt4
{

class GensColorButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor RESET unsetBgColor)
	
	public:
		GensColorButton(QWidget *parent = 0)
			: QPushButton(parent) { }
		GensColorButton(const QString& text, QWidget *parent = 0)
			: QPushButton(text, parent) { }
		GensColorButton(const QIcon& icon, const QString& text, QWidget *parent = 0)
			: QPushButton(icon, text, parent) { }
		
		QColor bgColor(void);
		void setBgColor(const QColor& newBgColor);
		void unsetBgColor(void);
	
	protected:
		static QColor TextColor_For_BgColor(const QColor& color);
		
		// Stylesheet for QPushButton background color.
		static const QString ms_sCssBtnColors;
		
		QColor m_bgColor;
};

inline QColor GensColorButton::bgColor(void)
	{ return m_bgColor; }

inline void GensColorButton::unsetBgColor(void)
	{ setBgColor(QColor()); }

/**
* TextColor_For_BgColor(): Get the text color for a given background color.
* If the luminance is < 128, this returns white.
* Otherwise, this returns black.
* @return Text color for the given background color.
*/
inline QColor GensColorButton::TextColor_For_BgColor(const QColor& color)
	{ return (color.value() < 128 ? QColor(Qt::white) : QColor(Qt::black)); }

}

#endif /* __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__ */
