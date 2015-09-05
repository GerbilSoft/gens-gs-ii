/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.hpp: Gens Window.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

// gens-qt4 classes.
#include "gqt4_main.hpp"
#include "VBackend/VBackend.hpp"
#include "EmuManager.hpp"
#include "Input/KeyHandlerQt.hpp"

// Idle thread.
#include "IdleThread.hpp"

namespace GensQt4 {

class GensWindowPrivate;
class GensWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		GensWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
		virtual ~GensWindow();

	private:
		typedef QMainWindow super;
		GensWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GensWindow)
	private:
		Q_DISABLE_COPY(GensWindow)

	public:
		// LibGens OSD handler.
		void osd(OsdType osd_type, int param);

		/**
		 * Rescale the window.
		 * @param scale New scale value.
		 */
		void rescale(int scale);

		// Set color depth.
		// TODO: Should this really be here, or should it be a slot?
		void setBpp(LibGens::MdFb::ColorDepth bpp);

		// Idle thread.
		// TODO: Rename to isIdleThreadAllowed().
		bool idleThreadAllowed(void);
		void setIdleThreadAllowed(bool idleThreadAllowed);

	protected:
		// QMainWindow virtual functions.
		virtual void closeEvent(QCloseEvent *event) override;
		virtual void showEvent(QShowEvent *event) override;

		// QMainWindow virtual functions: drag and drop.
		virtual void dragEnterEvent(QDragEnterEvent *event) override;
		virtual void dropEvent(QDropEvent *event) override;

		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) override;

	protected slots:
		/**
		 * Update the FPS counter.
		 */
		void updateFps(double fps);

		/**
		 * Emulation state changed.
		 * - Update the video backend "running" state.
		 * - Update the Gens title.
		 */
		void stateChanged(void);

		/**
		 * Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString &msg);

		/**
		 * Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osdShowPreview(int duration, const QImage& img);

		/**
		 * Application focus has changed.
		 * @param old Old widget.
		 * @param now New widget.
		 */
		void qAppFocusChanged(QWidget *old, QWidget *now);

		/**
		 * Auto Pause setting has changed.
		 * @param newAutoPause (bool) New Auto Pause setting.
		 */
		void autoPause_changed_slot(const QVariant &newAutoPause);

	private slots:
		/**
		 * The Idle thread is finished rendering a frame.
		 */
		void idleThread_frameDone(void);

		/**
		 * Intro Style setting has changed.
		 * @param newIntroStyle (int) New Intro Style setting.
		 */
		void introStyle_changed_slot(const QVariant &newIntroStyle);

		/**
		 * Show the context menu.
		 * @param pos Position to show the context menu. (widget coordinates)
		 */
		void showContextMenu(const QPoint& pos);

	private slots:
		/** Menu slots. **/

		// NOTE: Calling m_emuManager functions directly via
		// friend classes, or using inline functions here,
		// results in wacky memory corruption on Mac OS X.
		// (Mac OS X 10.5.7, PowerPC, gcc-4.0.1)
		// Specifically, the m_emuManager pointer is misread as
		// e.g. 0x101b4 when it should be 0x1b43210. It looks like
		// it's off by two bytes when reading the address from
		// the GensWindow instance.

		// File
		void on_actionFileOpenROM_triggered(void);
		void on_actionFileCloseROM_triggered(void);
		void on_actionFileSaveState_triggered(void);
		void on_actionFileLoadState_triggered(void);
		void on_actionFileGeneralConfiguration_triggered(void);
		void on_actionFileSegaCDControlPanel_triggered(void);
		void on_actionFileQuit_triggered(void);

		// File, Recent ROMs
		void mnu_actionFileRecentROMs_triggered(int idx);

		// Graphics
		void on_actionGraphicsShowMenuBar_triggered(bool checked);
		//void on_mnuGraphicsResolution_triggered(void);
		void map_actionGraphicsResolution_triggered(int scale);
		//void on_mnuGraphicsBpp_triggered(void);
		void map_actionGraphicsBpp_triggered(int bpp);
		void mnu_mnuGraphicsStretch_triggered(void);
		void map_actionGraphicsStretch_triggered(int stretchMode);
		void on_actionGraphicsScreenshot_triggered(void);

		// System
		void mnu_mnuSystemRegion_triggered(void);
		void map_actionSystemRegion_triggered(int region);
		void on_actionSystemHardReset_triggered(void);
		void on_actionSystemSoftReset_triggered(void);
		void on_actionSystemPause_triggered(bool checked);
		// TODO: Signal mapper for CPU reset options?
		void on_actionSystemResetM68K_triggered(void);
		void on_actionSystemResetS68K_triggered(void);
		void on_actionSystemResetMSH2_triggered(void);
		void on_actionSystemResetSSH2_triggered(void);
		void on_actionSystemResetZ80_triggered(void);

		// Options
		void on_actionOptionsSRAM_triggered(bool checked);
		void on_actionOptionsControllers_triggered(void);
		// SoundTest; remove this later.
		void map_actionSound_triggered(int freq);
		void on_actionSoundMono_triggered(void);
		void on_actionSoundStereo_triggered(void);

		// Help
		void on_actionHelpAbout_triggered(void);

		// Non-Menu Actions
		void on_actionNoMenuFastBlur_triggered(bool checked);
		void map_actionNoMenuSaveSlot_triggered(int saveSlot);
		void on_actionNoMenuSaveSlotPrev_triggered(void);
		void on_actionNoMenuSaveSlotNext_triggered(void);
		void on_actionNoMenuSaveStateAs_triggered(void);
		void on_actionNoMenuLoadStateFrom_triggered(void);

	private slots:
		/** Menu synchronization slots. **/
		void recentRoms_updated(void);
		void stretchMode_changed_slot(const QVariant &stretchMode);	// StretchMode_t
		void regionCode_changed_slot(const QVariant &regionCode);	// LibGens::SysVersion::RegionCode_t
		void enableSRam_changed_slot(const QVariant &enableSRam);	// bool
		void showMenuBar_changed_slot(const QVariant &showMenuBar);	// bool
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
