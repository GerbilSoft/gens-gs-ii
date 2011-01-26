/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensConfig.hpp: Gens configuration.                                     *
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

#ifndef __GENS_QT4_GENSCONFIG_HPP__
#define __GENS_QT4_GENSCONFIG_HPP__

#include <QtCore/QObject>
#include <QtCore/QString>

class GensConfig : public QObject
{
	Q_OBJECT
	
	public:
		GensConfig();
		~GensConfig();
		
		void save(void);
		void reload(void);
		
		// Sega CD Boot ROMs.
		QString mcdRomUSA(void) { return m_mcdRomUSA; }
		void setMcdRomUSA(const QString& filename);
		QString mcdRomEUR(void) { return m_mcdRomEUR; }
		void setMcdRomEUR(const QString& filename);
		QString mcdRomJPN(void) { return m_mcdRomJPN; }
		void setMcdRomJPN(const QString& filename);
	
	protected:
		QString m_mcdRomUSA;
		QString m_mcdRomEUR;
		QString m_mcdRomJPN;
	
	signals:
		void mcdRomUSA_changed(const QString& filename);
		void mcdRomEUR_changed(const QString& filename);
		void mcdRomJPN_changed(const QString& filename);
};

#endif /* __GENS_QT4_GENSCONFIG_HPP__ */
