/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * Screenshot.hpp: Screenshot handler.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __GENS_QT4_SCREENSHOT_HPP__
#define __GENS_QT4_SCREENSHOT_HPP__

// LibGens includes.
#include "libgens/Rom.hpp"
#include "libgens/EmuContext.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QImage>
class QIODevice;
class QImageWriter;

namespace GensQt4
{

class Screenshot : public QObject
{
	Q_OBJECT
	
	public:
		Screenshot(LibGens::Rom *rom, LibGens::EmuContext *context, QObject *parent = 0);
		~Screenshot();
		
		QImage get(void);
		int save(QString filename);
		int save(QIODevice *device);
	
	protected:
		LibGens::Rom *m_rom;
		LibGens::EmuContext *m_context;
		QImage m_img;
		
		void update(void);
		int save_int(QImageWriter& writer);
};

/**
 * Get the internal image.
 * @return QImage with screenshot, or empty QImage if either ROM or EmuContext isn't set.
 */
inline QImage Screenshot::get(void)
{
	return m_img;
}

}

#endif /* __GENS_QT4_SCREENSHOT_HPP__ */
