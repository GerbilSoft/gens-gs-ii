/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend.hpp: Video Backend class.                                      *
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

#ifndef __GENS_QT4_VBACKEND_HPP__
#define __GENS_QT4_VBACKEND_HPP__

// C includes.
#include <stdarg.h>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QWidget>
#include <QtGui/QColor>

#include "libgens/MD/VdpRend.hpp"

// paused_t
#include "gqt4_datatypes.h"

// StretchMode is in GensConfig.
// TODO: Move somewhere else, or use int?
#include "GensConfig.hpp"


namespace GensQt4
{

// Forward declaration for MsgTimer.
// Can't #include "MsgTimer.hpp" due to circular dependencies.
class MsgTimer;

class VBackend : public QWidget
{
	Q_OBJECT
	
	public:
		VBackend(QWidget *parent = 0);
		virtual ~VBackend();
		
		void setVbDirty(void)
			{ m_vbDirty = true; }
		void setMdScreenDirty(void)
			{ m_mdScreenDirty = true; }
		virtual void vbUpdate(void) = 0;
		
		/** Properties. **/
		
		/** Video settings. **/
		inline bool isPaused(void) const
			{ return !!(m_paused.data); }
		inline bool isAutoPaused(void) const
			{ return !!(m_paused.paused_auto); }
		inline bool isManualPaused(void) const
			{ return !!(m_paused.paused_manual); }
		void setPaused(paused_t newPaused);
		
		inline bool isRunning(void) const { return m_running; }
		void setRunning(bool newIsRunning);
		
		/** Onscreen display. **/
		bool osdFpsEnabled(void) const;
		QColor osdFpsColor(void) const;
		bool osdMsgEnabled(void) const;
		QColor osdMsgColor(void) const;
		
		/** Format strings. **/
		
		// NOTE: Format string argument is 3 instead of 2.
		// This is due to the implicit "this" parameter.
		void osd_vprintf(const int duration, const char *msg, va_list ap)
			__attribute__ ((format (printf, 3, 0)));
		void osd_printf(const int duration, const char *msg, ...)
			__attribute__ ((format (printf, 3, 4)));
		
		/**
		 * osd_lock() / osd_unlock(): Temporarily lock the OSD.
		 * Primarily used when loading settings all at once.
		 * Calls are cumulative; 2 locks requires 2 unlocks.
		 * Calling osd_unlock() when not locked will return an error.
		 * @return 0 on success; non-zero on error.
		 */
		int osd_lock(void);
		int osd_unlock(void);
		
		/**
		 * osd_show_preview(): Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		virtual void osd_show_preview(int duration, const QImage& img);
		
		// FPS manager.
		void resetFps(void);
		void pushFps(double fps);
		
		// Recording OSD.
		int recSetStatus(const QString& component, bool isRecording);
		int recSetDuration(const QString& component, int duration);
		int recStart(const QString& component);
		int recStop(const QString& component);
	
	protected:
		// Dirty flags.
		bool m_vbDirty;		// VBackend dirty: screen must be redrawn.
		bool m_mdScreenDirty;	// MD Screen dirty: texture must be reuploaded.
		
		// Color depth information.
		LibGens::VdpPalette::ColorDepth m_lastBpp;
		
		// Effects.
		void updatePausedEffect(bool fromMdScreen = true);
		void updateFastBlur(bool fromMdScreen = true);
		
		// Internal rendering buffer used for software effects.
		// NOTE: This takes up (336*240*4) == 322,560 bytes!
		LibGens::VdpRend::Screen_t *m_intScreen;
		
		// Get the current average FPS.
		inline double fpsAvg(void)
			{ return m_fpsAvg; }
		
		// OSD message struct.
		struct OsdMessage
		{
			OsdMessage(const char *msg, double endTime);
			QString msg;
			double endTime;
		};
		QList<OsdMessage> m_osdList;
		
		bool isOsdListDirty(void);
		void setOsdListDirty(void);
		void clearOsdListDirty(void);
		
		// Preview image.
		bool m_preview_show;
		QImage m_preview_img;
		double m_preview_endTime;
		
		struct RecOsd
		{
			QString component;
			int duration;		// ms
			double lastUpdate;	// ms
			bool isRecording;	// True if recording; false if stopped.
		};
		QList<RecOsd> m_osdRecList;
		
		/** Properties. **/
		// TODO: Should we keep these properties here, or just get them from gqt4_config?
		bool fastBlur(void) const;
		bool aspectRatioConstraint(void) const;
		bool hasAspectRatioConstraintChanged(void) const;
		void resetAspectRatioConstraintChanged(void);
		bool bilinearFilter(void) const;
		bool pauseTint(void) const;
		GensConfig::StretchMode stretchMode(void) const;
	
	protected slots:
		void setOsdFpsEnabled(bool enable);
		void setOsdFpsColor(const QColor& color);
		void setOsdMsgEnabled(bool enable);
		void setOsdMsgColor(const QColor& color);
		
		virtual void setFastBlur(bool newFastBlur);
		virtual void setAspectRatioConstraint(bool newAspectRatioConstraint);
		virtual void setBilinearFilter(bool newBilinearFilter);
		virtual void setPauseTint(bool newPauseTint);
		virtual void setStretchMode(GensConfig::StretchMode newStretchMode);
	
	private:
		// Effects.
		paused_t m_paused;
		bool m_fastBlur;
		bool m_pauseTint;
		
		// Is the emulator running?
		bool m_running;
		
		// Message timer.
		MsgTimer *m_msgTimer;
		
		/** Onscreen Display. **/
		
		// FPS manager.
		double m_fps[8];
		double m_fpsAvg;	// Average fps.
		int m_fpsPtr;		// Pointer to next fps slot to use.
		
		// m_osdListDirty: Set if the OSD message list has been changed.
		bool m_osdListDirty;
		
		// OSD enable bits.
		bool m_osdFpsEnabled;
		bool m_osdMsgEnabled;
		
		// OSD colors.
		QColor m_osdFpsColor;
		QColor m_osdMsgColor;
		
		// OSD lock counter.
		int m_osdLockCnt;
		
		/**
		 * osd_process(): Process the OSD queue.
		 * This should ONLY be called by MsgTimer!
		 * @return Number of messages remaining in the OSD queue.
		 */
		int osd_process(void);
		friend class MsgTimer;
		
		/** Video settings. **/
		bool m_aspectRatioConstraint;
		bool m_aspectRatioConstraint_changed;
		bool m_bilinearFilter;
		GensConfig::StretchMode m_stretchMode;
};

/** Onscreen display. **/

inline bool VBackend::osdFpsEnabled(void) const
	{ return m_osdFpsEnabled; }
inline QColor VBackend::osdFpsColor(void) const
	{ return m_osdFpsColor; }
inline bool VBackend::osdMsgEnabled(void) const
	{ return m_osdMsgEnabled; }
inline QColor VBackend::osdMsgColor(void) const
	{ return m_osdMsgColor; }

/**
 * osd_vprintf(): Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 * @param ... Format arguments.
 */
inline void VBackend::osd_printf(const int duration, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	osd_vprintf(duration, msg, ap);
	va_end(ap);
}

/** OSD message struct constructor. **/
inline VBackend::OsdMessage::OsdMessage(const char *msg, double endTime)
{
	this->msg = QString::fromUtf8(msg);
	this->endTime = endTime;
}

/** OSD list dirty flag functions. **/
inline bool VBackend::isOsdListDirty(void)
	{ return m_osdListDirty; }
inline void VBackend::setOsdListDirty(void)
	{ m_osdListDirty = true; }
inline void VBackend::clearOsdListDirty(void)
	{ m_osdListDirty = false; }

/** Recording OSD. **/
inline int VBackend::recStart(const QString& component)
	{ return recSetStatus(component, true); }
inline int VBackend::recStop(const QString& component)
	{ return recSetStatus(component, false); }

/** Property read functions. **/
// TODO: Should we keep these properties here, or just get them from gqt4_config?
inline bool VBackend::fastBlur(void) const
	{ return m_fastBlur; }
inline bool VBackend::aspectRatioConstraint(void) const
	{ return m_aspectRatioConstraint; }
inline bool VBackend::hasAspectRatioConstraintChanged(void) const
	{ return m_aspectRatioConstraint_changed; }
inline void VBackend::resetAspectRatioConstraintChanged(void)
	{ m_aspectRatioConstraint_changed = false; }
inline bool VBackend::bilinearFilter(void) const
	{ return m_bilinearFilter; }
inline bool VBackend::pauseTint(void) const
	{ return m_pauseTint; }
inline GensConfig::StretchMode VBackend::stretchMode(void) const
	{ return m_stretchMode; }

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
