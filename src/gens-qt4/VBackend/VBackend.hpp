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

// C includes.
#include <stdarg.h>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QWidget>

#include "libgens/MD/VdpRend.hpp"

namespace GensQt4
{

// Forward declaration for MsgTimer.
// Can't #include "MsgTimer.hpp" due to circular dependencies.
class MsgTimer;

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
		
		bool isPaused(void) const { return m_paused; }
		void setPaused(bool newPaused);
		
		bool fastBlur(void) const { return m_fastBlur; }
		void setFastBlur(bool newFastBlur)
		{
			if (m_fastBlur == newFastBlur)
				return;
			
			// Update the Fast Blur setting.
			m_fastBlur = newFastBlur;
			if (isRunning())
				setVbDirty();
		}
		
		bool isRunning(void) const { return m_running; }
		void setRunning(bool newIsRunning);
		
		// NOTE: Format string argument is 3 instead of 2.
		// This is due to the implicit "this" parameter.
		void osd_vprintf(const int duration, const char *msg, va_list ap)
			__attribute__ ((format (printf, 3, 0)));
		void osd_printf(const int duration, const char *msg, ...)
			__attribute__ ((format (printf, 3, 4)))
		{
			va_list ap;
			va_start(ap, msg);
			osd_vprintf(duration, msg, ap);
			va_end(ap);
		}
		
		// FPS manager.
		void resetFps(void);
		void pushFps(double fps);
		
		bool showFps(void) const { return m_showFps; }
		void setShowFps(bool newShowFps)
		{
			if (m_showFps == newShowFps)
				return;
			
			// Update the Show FPS setting.
			m_showFps = newShowFps;
			setVbDirty();		// TODO: Texture doesn't really need to be reuploaded...
			
			// Mark the OSD list as dirty if the emulator is running.
			if (isRunning())
				setOsdListDirty();
		}
	
	protected:
		// Dirty flag. If set, texture must be reuploaded.
		bool m_vbDirty;
		
		// Color depth information.
		LibGens::VdpPalette::ColorDepth m_lastBpp;
		
		// Effects.
		void updatePausedEffect(bool fromMdScreen = true);
		void updateFastBlur(bool fromMdScreen = true);
		
		// Internal rendering buffer used for software effects.
		// NOTE: This takes up (336*240*4) == 322,560 bytes!
		LibGens::VdpRend::Screen_t *m_intScreen;
		
		// OSD message struct.
		struct OsdMessage
		{
			OsdMessage(const char *msg, double endTime)
			{
				this->msg = QString(msg);
				this->endTime = endTime;
			}
			
			QString msg;
			double endTime;
		};
		QList<OsdMessage> m_osdList;
		
		// m_osdListDirty: Set if the OSD message list has been changed.
		bool m_osdListDirty;
		inline void setOsdListDirty(void) { m_osdListDirty = true; }
		
		/**
		 * osd_process(): Process the OSD queue.
		 * This should ONLY be called by MsgTimer!
		 * @return Number of messages remaining in the OSD queue.
		 */
		int osd_process(void);
		friend class MsgTimer;
		
		// FPS manager.
		double m_fps[8];
		double m_fpsAvg;	// Average fps.
		int m_fpsPtr;		// Pointer to next fps slot to use.
		bool m_showFps;
	
	private:
		// Effects.
		bool m_paused;
		bool m_fastBlur;
		
		// Is the emulator running?
		bool m_running;
		
		// Message timer.
		MsgTimer *m_msgTimer;
};

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
