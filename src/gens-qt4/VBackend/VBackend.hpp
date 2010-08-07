/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend.hpp: Video Backend class.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include <config.h>

#ifndef __GENS_QT4_VBACKEND_HPP__
#define __GENS_QT4_VBACKEND_HPP__

#include <QtGui/QWidget>

#include "libgens/MD/VdpRend.hpp"

namespace GensQt4
{

class VBackend
{
	public:
		VBackend();
		virtual ~VBackend();
		
		void setVbDirty(void) { m_vbDirty = true; }
		virtual void vbUpdate(void) = 0;
		
		// Return a QWidget* version of this object.
		// Since this is the base class, this will return NULL.
		virtual QWidget *toQWidget(void) { return NULL; }
		
		bool paused(void) const { return m_paused; }
		void setPaused(bool newPaused)
		{
			if (m_paused == newPaused)
				return;
			
			// Update the pause status.
			m_paused = newPaused;
			setVbDirty();
		}
	
	protected:
		// Dirty flag. If set, texture must be reuploaded.
		bool m_vbDirty;
		
		// Color depth information.
		LibGens::VdpRend::ColorDepth m_lastBpp;
		
		// Effects.
		virtual void updatePausedEffect(void);
		
		// Internal rendering buffer used for software effects.
		// NOTE: This takes up ~322 KB!
		// TODO: Dynamically allocate this buffer only if it's needed?
		LibGens::VdpRend::Screen_t m_intScreen;
	
	private:
		// Effects.
		bool m_paused;
};

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
