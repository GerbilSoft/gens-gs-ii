/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensColorButton.cpp: QButton class with background color support.       *
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

#include "GensColorButton.hpp"

namespace GensQt4
{

// Stylesheet for QPushButton background color.
const char *GensColorButton::ms_sCssBtnColors =
	"QPushButton { background-color: %1; color: %2; }";

/**
 * setBgColor(): Set the background color.
 * @param newBgColor New background color. (If an invalid color is specified, unsets the background color.)
 */
void GensColorButton::setBgColor(const QColor& newBgColor)
{
	if (m_bgColor == newBgColor || (!m_bgColor.isValid() && !newBgColor.isValid()))
		return;
	
	m_bgColor = newBgColor;
	if (!m_bgColor.isValid())
	{
		this->setStyleSheet(QString());
		return;
	}
	
	// Create a stylesheet with the new background color.
	QColor colorText = TextColor_For_BgColor(m_bgColor);
	this->setStyleSheet(QString::fromLatin1(ms_sCssBtnColors)
				.arg(m_bgColor.name()).arg(colorText.name()));
}

}
