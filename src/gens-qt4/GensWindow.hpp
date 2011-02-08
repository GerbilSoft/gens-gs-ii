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
#include "libgens/MD/VdpPalette.hpp"

// gens-qt4 classes.
#include "gqt4_main.hpp"
#include "VBackend/VBackend.hpp"
#include "EmuManager.hpp"
#include "actions/GensActions.hpp"

namespace GensQt4
{

// Class declarations.
class GensMenuBar;

class GensWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		GensWindow();
		~GensWindow();
		
		// LibGens OSD handler.
		void osd(OsdType osd_type, int param);
		
		// Rescale the window.
		void rescale(int scale)
		{
			if (scale <= 0 || scale > 8)
				return;
			m_scale = scale;
			gensResize();
		}
		
		// Set color depth.
		// TODO: Should this really be here, or should it be a slot?
		void setBpp(LibGens::VdpPalette::ColorDepth bpp);
	
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
		void closeRom(void);
		void saveState(void);
		void loadState(void);
		void screenShot(void);
		void setController(int port, LibGens::IoBase::IoType type);
		void setAudioRate(int newRate);
		void setStereo(bool newStereo);
	
	protected:
		void setupUi(void);
		void retranslateUi(void);
		
		// QMainWindow virtual functions.
		void closeEvent(QCloseEvent *event);
		void showEvent(QShowEvent *event);
		
		// QMainWindow virtual functions: drag and drop.
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);
		
		// Widgets.
		VBackend *m_vBackend;	// GensQGLWidget.
		GensMenuBar *m_menubar;	// Gens menu bar.
		
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
			m_vBackend->setVbDirty();
			m_vBackend->vbUpdate();
		}
		
		/**
		 * osdPrintMsg(): Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString& msg)
			{ m_vBackend->osd_printf(duration, "%s", msg.toUtf8().constData()); }
		
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
		 * @param newAutoPause New Auto Pause setting.
		 */
		void autoPause_changed_slot(bool newAutoPause);
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
