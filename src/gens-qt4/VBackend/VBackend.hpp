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

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtGui/QWidget>
#include <QtGui/QColor>

// LibGens includes.
#include "libgens/Util/MdFb.hpp"
#include "libgens/Vdp/VdpPalette.hpp"

// paused_t, StretchMode_t
#include "gqt4_datatypes.h"

// Emulation Context.
#include "libgens/EmuContext.hpp"

// Configuration items.
#include "Config/ConfigItem.hpp"
#include "Config/ConfigItemColor.hpp"

namespace GensQt4
{

// Forward declaration for MsgTimer.
// Can't #include "MsgTimer.hpp" due to circular dependencies.
class MsgTimer;
class KeyHandlerQt;

class VBackend : public QWidget
{
	Q_OBJECT
	
	public:
		VBackend(QWidget *parent = 0, KeyHandlerQt *keyHandler = 0);
		virtual ~VBackend();
		
		void setVbDirty(void)
			{ m_vbDirty = true; }
		void setMdScreenDirty(void)
			{ m_mdScreenDirty = true; }
		virtual void vbUpdate(void) = 0;
		
		void setKeyHandler(KeyHandlerQt *newKeyHandler);
		
		/** Properties. **/
		
		/** Video settings. **/
		inline bool isPaused(void) const
			{ return !!(m_paused.data); }
		inline bool isAutoPaused(void) const
			{ return !!(m_paused.paused_auto); }
		inline bool isManualPaused(void) const
			{ return !!(m_paused.paused_manual); }
		void setPaused(paused_t newPaused);
		
		/** Onscreen display. **/
		bool osdFpsEnabled(void) const;
		QColor osdFpsColor(void) const;
		bool osdMsgEnabled(void) const;
		QColor osdMsgColor(void) const;
		
		/** Properties. **/
		bool fastBlur(void) const;
		void setFastBlur(bool newFastBlur);
		
		bool aspectRatioConstraint(void) const;
		bool hasAspectRatioConstraintChanged(void) const;
		void resetAspectRatioConstraintChanged(void);
		bool bilinearFilter(void) const;
		bool pauseTint(void) const;
		StretchMode_t stretchMode(void) const;
		void setStretchMode(StretchMode_t newStretchMode);
		
		/** Emulation Context. **/
		LibGens::EmuContext *emuContext(void) const;
		void setEmuContext(LibGens::EmuContext *newEmuContext);
		bool isRunning(void) const;
		
		/** Format strings. **/
		
		// NOTE: Format string argument is 3 instead of 2.
		// This is due to the implicit "this" parameter.
		void osd_vprintf(const int duration, const utf8_str *msg, va_list ap)
			__attribute__ ((format (printf, 3, 0)));
		void osd_printf(const int duration, const utf8_str *msg, ...)
			__attribute__ ((format (printf, 3, 4)));
		void osd_printqs(const int duration, const QString& msg);
		
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
		// Key handler.
		KeyHandlerQt *m_keyHandler;
		
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
		LibGens::MdFb *m_intScreen;
		
		// Get the current average FPS.
		inline double fpsAvg(void)
			{ return m_fpsAvg; }
		
		// OSD message struct.
		struct OsdMessage
		{
			OsdMessage(const QString &msg, double endTime);
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
		
		/** Emulation Context. **/
		LibGens::EmuContext *m_emuContext;
		QMutex m_mtxEmuContext;
		
		// Reset Stretch Mode. (Used if the stretch mode is invalid.)
		void stretchMode_reset(void);
	
	protected slots:
		/** Properties. **/
		void osdFpsEnabled_changed_slot(const QVariant& enable);	// bool
		void osdFpsColor_changed_slot(const QColor& color);
		void osdMsgEnabled_changed_slot(const QVariant& enable);	// bool
		void osdMsgColor_changed_slot(const QColor& color);
		
		virtual void fastBlur_changed_slot(const QVariant& newFastBlur);				// bool
		virtual void aspectRatioConstraint_changed_slot(const QVariant& newAspectRatioConstraint);	// bool
		virtual void bilinearFilter_changed_slot(const QVariant& newBilinearFilter);			// bool
		virtual void pauseTint_changed_slot(const QVariant& newPauseTint);				// bool
		virtual void stretchMode_changed_slot(const QVariant& newStretchMode);				// int
	
	private:
		// Effects.
		paused_t m_paused;
		ConfigItem *m_cfg_fastBlur;	// bool
		ConfigItem *m_cfg_pauseTint;	// bool
		
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
		ConfigItem *m_cfg_osdFpsEnabled;
		ConfigItem *m_cfg_osdMsgEnabled;
		
		// OSD colors.
		ConfigItemColor *m_cfg_osdFpsColor;
		ConfigItemColor *m_cfg_osdMsgColor;
		
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
		ConfigItem *m_cfg_aspectRatioConstraint;	// bool
		bool m_aspectRatioConstraint_changed;
		ConfigItem *m_cfg_bilinearFilter;		// bool
		ConfigItem *m_cfg_stretchMode;			// StretchMode_t
	
	private slots:
		// Key handler destroyed slot.
		void keyHandlerDestroyed(void);
};

/** Onscreen display. **/

inline bool VBackend::osdFpsEnabled(void) const
	{ return m_cfg_osdFpsEnabled->value().toBool(); }
inline QColor VBackend::osdFpsColor(void) const
	{ return m_cfg_osdFpsColor->value(); }
inline bool VBackend::osdMsgEnabled(void) const
	{ return m_cfg_osdMsgEnabled->value().toBool(); }
inline QColor VBackend::osdMsgColor(void) const
	{ return m_cfg_osdMsgColor->value(); }

/**
 * osd_vprintf(): Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 * @param ... Format arguments.
 */
inline void VBackend::osd_printf(const int duration, const utf8_str *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	osd_vprintf(duration, msg, ap);
	va_end(ap);
}

/** OSD message struct constructor. **/
inline VBackend::OsdMessage::OsdMessage(const QString& msg, double endTime)
	: msg(msg)
	, endTime(endTime)
{ }

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
	{ return m_cfg_fastBlur->value().toBool(); }
inline void VBackend::setFastBlur(bool newFastBlur)
	{ m_cfg_fastBlur->setValue(newFastBlur); }
inline bool VBackend::aspectRatioConstraint(void) const
	{ return m_cfg_aspectRatioConstraint->value().toBool(); }
inline bool VBackend::hasAspectRatioConstraintChanged(void) const
	{ return m_aspectRatioConstraint_changed; }
inline void VBackend::resetAspectRatioConstraintChanged(void)
	{ m_aspectRatioConstraint_changed = false; }
inline bool VBackend::bilinearFilter(void) const
	{ return m_cfg_bilinearFilter->value().toBool(); }
inline bool VBackend::pauseTint(void) const
	{ return m_cfg_pauseTint->value().toBool(); }
inline StretchMode_t VBackend::stretchMode(void) const
	{ return (StretchMode_t)m_cfg_stretchMode->value().toInt(); }
inline void VBackend::setStretchMode(StretchMode_t newStretchMode)
	{ m_cfg_stretchMode->setValue((int)newStretchMode); }

inline LibGens::EmuContext *VBackend::emuContext(void) const
	{ return m_emuContext; }
inline bool VBackend::isRunning(void) const
{
	// TODO: Lock m_mtxEmuContext?
	return (!!m_emuContext);
}

inline void VBackend::stretchMode_reset(void)
	{ m_cfg_stretchMode->setValue(m_cfg_stretchMode->def()); }

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
