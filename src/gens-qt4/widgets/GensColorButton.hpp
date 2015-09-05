/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensColorButton.hpp: QButton class with background color support.       *
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

#ifndef __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__
#define __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__

#include <QtGui/QPushButton>
#include <QtGui/QColor>

namespace GensQt4 {

class GensColorButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor RESET unsetBgColor)
	
	public:
		GensColorButton(QWidget *parent = 0)
			: super(parent) { }
		GensColorButton(const QString& text, QWidget *parent = 0)
			: super(text, parent) { }
		GensColorButton(const QIcon& icon, const QString& text, QWidget *parent = 0)
			: super(icon, text, parent) { }

	private:
		typedef QPushButton super;
	private:
		Q_DISABLE_COPY(GensColorButton)

	public:
		/**
		 * Get the current background color.
		 * @return Background color.
		 */
		QColor bgColor(void);

		/**
		 * Set the background color.
		 * @param newBgColor New background color. (If an invalid color is specified, unsets the background color.)
		 */
		void setBgColor(const QColor& newBgColor);

		/**
		 * Unset the background color.
		 */
		void unsetBgColor(void);
	
	protected:
		// TODO: Move to a private class?

		/**
		 * Get the text color for a given background color.
		 * If the luminance is < 128, this returns white.
		 * Otherwise, this returns black.
		 * @return Text color for the given background color.
		 */
		static QColor TextColor_For_BgColor(const QColor& color);

		// Stylesheet for QPushButton background color.
		static const char *ms_sCssBtnColors;

		QColor m_bgColor;
};

inline QColor GensColorButton::bgColor(void)
	{ return m_bgColor; }

inline void GensColorButton::unsetBgColor(void)
	{ setBgColor(QColor()); }

inline QColor GensColorButton::TextColor_For_BgColor(const QColor& color)
	{ return (color.value() < 128 ? QColor(Qt::white) : QColor(Qt::black)); }

}

#endif /* __GENS_QT4_WIDGETS_GENSCOLORBUTTON_HPP__ */
