/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.hpp: Gens Window.                                            *
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

#ifndef __GENS_QT4_GENSWINDOW_HPP__
#define __GENS_QT4_GENSWINDOW_HPP__

// Qt includes.
#include <QtGui/QMainWindow>
#include <QtGui/QImage>

// Qt forward declarations.
class QVBoxLayout;
class QCloseEvent;

// LibGens includes.
#include "libgens/lg_osd.h"
#include "libgens/Vdp/VdpPalette.hpp"

// gens-qt4 classes.
#include "gqt4_main.hpp"
#include "VBackend/VBackend.hpp"
#include "EmuManager.hpp"
#include "actions/GensActions.hpp"
#include "actions/GensMenuBar.hpp"
#include "Input/KeyHandlerQt.hpp"

// Idle thread.
#include "IdleThread.hpp"

namespace GensQt4
{

class GensWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		GensWindow();
		~GensWindow();
		
		// LibGens OSD handler.
		void osd(OsdType osd_type, int param);
		
		/**
		 * rescale(): Rescale the window.
		 * @param scale New scale value.
		 */
		void rescale(int scale);
		
		// Set color depth.
		// TODO: Should this really be here, or should it be a slot?
		void setBpp(LibGens::VdpPalette::ColorDepth newBpp);
		
		// Idle thread.
		inline bool idleThreadAllowed(void)
			{ return m_idleThreadAllowed; }
		void setIdleThreadAllowed(bool newIdleThreadAllowed);
		
		// Wrapper for GensActions.
		bool menuItemCheckState(int action);
	
	public slots:
		/** Wrapper functions for GensActions. **/
		/** TODO: Have GensActions emit signals, and link them to EmuManager slots. **/
		
		// NOTE: Calling m_emuManager functions directly via
		// friend classes, or using inline functions here,
		// results in wacky memory corruption on Mac OS X.
		// (Mac OS X 10.5.7, PowerPC, gcc-4.0.1)
		// Specifically, the m_emuManager pointer is misread as
		// e.g. 0x101b4 when it should be 0x1b43210. It looks like
		// it's off by two bytes when reading the address from
		// the GensWindow instance.
		void openRom(void);
		void openRom(const QString& filename, QString z_filename = QString());
		void closeRom(void);
		void saveState(void);
		void loadState(void);
		void screenShot(void);
		void setAudioRate(int newRate);
		void setStereo(bool newStereo);
	
	protected:
		void setupUi(void);
		
		// QMainWindow virtual functions.
		void closeEvent(QCloseEvent *event);
		void showEvent(QShowEvent *event);
		
		// QMainWindow virtual functions: drag and drop.
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);
		
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);
		
		// Key handler.
		KeyHandlerQt *m_keyHandler;
		
		// Widgets.
		VBackend *m_vBackend;		// GensQGLWidget.
		GensMenuBar *m_gensMenuBar;	// Gens menu bar.
		
		QWidget *centralwidget;
		QVBoxLayout *layout;
		
		// GensWindow functions.
		void gensResize(void);	// Resize the window.
		
		int m_scale;		// Temporary scaling variable.
		bool m_hasInitResize;	// Has the initial resize occurred?
		
		// Emulation Manager.
		EmuManager *m_emuManager;
		
		// Actions manager.
		GensActions *m_gensActions;
		
		// Set the Gens window title.
		void setGensTitle(void);
		
		/** Configuration items. **/
		ConfigItem *m_cfg_autoPause;
		ConfigItem *m_cfg_introStyle;
		ConfigItem *m_cfg_showMenuBar;
	
	protected slots:
		/**
		 * updateFps(): Update the FPS counter.
		 */
		void updateFps(double fps)
			{ m_vBackend->pushFps(fps); }
		
		/**
		 * stateChanged(): Emulation state changed.
		 * - Update the video backend "running" state.
		 * - Update the Gens title.
		 */
		void stateChanged(void);
		
		/**
		 * updateVideo(): Update video.
		 */
		void updateVideo(void)
		{
			m_vBackend->setMdScreenDirty();
			m_vBackend->setVbDirty();
			m_vBackend->vbUpdate();
		}
		
		/**
		 * osdPrintMsg(): Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString& msg)
			{ m_vBackend->osd_printqs(duration, msg); }
		
		/**
		 * osdShowPreview(): Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osdShowPreview(int duration, const QImage& img)
			{ m_vBackend->osd_show_preview(duration, img); }
		
		/**
		 * qAppFocusChanged(): Application focus has changed.
		 * @param old Old widget.
		 * @param now New widget.
		 */
		void qAppFocusChanged(QWidget *old, QWidget *now);
		
		/**
		 * autoPause_changed_slot(): Auto Pause setting has changed.
		 * @param newAutoPause (bool) New Auto Pause setting.
		 */
		void autoPause_changed_slot(const QVariant& newAutoPause);
	
		/**
		 * showMenuBar_changed_slot(): Show Menu Bar setting has changed.
		 * @param newShowMenuBar (bool) New Show Menu Bar setting.
		 */
		void showMenuBar_changed_slot(const QVariant& newShowMenuBar);
		
	private:
		/** Idle thread. **/
		IdleThread *m_idleThread;
		bool m_idleThreadAllowed;
		void checkIdleThread(void);
	
	private slots:
		void idleThread_frameDone(void);
		
		/**
		 * introStyle_changed_slot(): Intro Style setting has changed.
		 * @param newIntroStyle (int) New Intro Style setting.
		 */
		void introStyle_changed_slot(const QVariant& newIntroStyle);
		
		/**
		 * showContextMenu(): Show the context menu.
		 * @param pos Position to show the context menu. (widget coordinates)
		 */
		void showContextMenu(const QPoint& pos);
};

// Wrapper for GensActions.
inline bool GensWindow::menuItemCheckState(int action)
	{ return m_gensMenuBar->menuItemCheckState(action); }

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
