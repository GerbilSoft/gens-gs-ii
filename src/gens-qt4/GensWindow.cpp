/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.cpp: Gens Window.                                            *
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

#include "GensWindow.hpp"
#include "gqt4_main.hpp"

// Qt4 windows.
#include "AboutWindow.hpp"
#include "CtrlConfigWindow.hpp"

// LibGens.
#include "libgens/lg_main.hpp"
#include "libgens/MD/EmuMD.hpp"
#include "libgens/macros/git.h"
#include "libgens/macros/log_msg.h"
#include "libgens/Util/Timing.hpp"

// Video Backend classes.
#include "VBackend/GensQGLWidget.hpp"

// C includes. (Needed for fps timing.)
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Win32 needs io.h for dup().
// TODO: Move File/Open to another file.
#include <io.h>

// QWidget doesn't differentiate between L/R modifier keys,
// and neither do WM_KEYDOWN/WM_KEYUP.
#include "libgens/IO/KeyManager.hpp"
#endif

// Qt4 includes.
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QFileDialog>

// Text translation macro.
#define TR(text) \
	QApplication::translate("GensWindow", (text), NULL, QApplication::UnicodeUTF8)


namespace GensQt4
{

/**
 * GensWindow(): Initialize the Gens window.
 */
GensWindow::GensWindow()
{
	m_scale = 1;		// Set the scale to 1x by default.
	m_hasInitResize = false;
	
	// Set up the User Interface.
	setupUi();
}


/**
 * ~GensWindow(): Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
	// TODO
}


/**
 * setupUi(): Set up the User Interface.
 */
void GensWindow::setupUi(void)
{
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("GensWindow"));
	
	// Set the window icon.
	QIcon winIcon;
	winIcon.addFile(":/gens/gensgs_48x48.png", QSize(48, 48));
	winIcon.addFile(":/gens/gensgs_32x32.png", QSize(32, 32));
	winIcon.addFile(":/gens/gensgs_16x16.png", QSize(16, 16));
	this->setWindowIcon(winIcon);
	
	// Create the central widget.
	centralwidget = new QWidget(this);
	centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
	this->setCentralWidget(centralwidget);
	
	// Retranslate the UI.
	retranslateUi();
	
	// Connect slots by name.
	QMetaObject::connectSlotsByName(this);
	
	// Create the menubar.
	m_menubar = new GensMenuBar(this);
	this->setMenuBar(m_menubar);
	
	// Create the Video Backend.
	// TODO: Allow selection of all available VBackend classes.
	m_vBackend = new GensQGLWidget(this->centralwidget);
	
	// Create the layout.
	layout = new QVBoxLayout(this->centralwidget);
	layout->setObjectName(QString::fromUtf8("layout"));
	layout->setMargin(0);
	layout->setSpacing(0);
	centralwidget->setLayout(layout);
	
	// Add the Video Backend to the layout.
	// TODO: Figure out a way to do this without RTTI.
	QWidget *vbackend_widget = m_vBackend->toQWidget();
	if (vbackend_widget != NULL)
	{
		layout->addWidget(vbackend_widget);
	}
	else
	{
		// Not a QWidget!
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Windowed mode: VBackend is not a QWidget!");
	}
	
	// Connect the GensMenuBar's triggered() signal.
	connect(m_menubar, SIGNAL(triggered(int)),
		this, SLOT(menuTriggered(int)));
	
	// Connect Emulation Manager signals to GensWindow.
	// TODO: Make m_emuManager a pointer instead of an object?
	connect(&m_emuManager, SIGNAL(updateFps(double)),
		this, SLOT(updateFps(double)));
	connect(&m_emuManager, SIGNAL(stateChanged(void)),
		this, SLOT(stateChanged(void)));
	connect(&m_emuManager, SIGNAL(updateVideo(void)),
		this, SLOT(updateVideo(void)));
	connect(&m_emuManager, SIGNAL(osdPrintMsg(int, const QString&)),
		this, SLOT(osdPrintMsg(int, const QString&)));
	
	// Retranslate the UI.
	retranslateUi();
}


/**
 * retranslateUi(): Retranslate the User Interface.
 */
void GensWindow::retranslateUi(void)
{
	// Set the Gens title.
	setGensTitle();
}


/**
 * closeEvent(): Window is being closed.
 * @param event Close event.
 */
void GensWindow::closeEvent(QCloseEvent *event)
{
	// Quit.
	m_emuManager.closeRom();
	QuitGens();
	
	// Accept the close event.
	event->accept();
}


/**
 * showEvent(): Window is being shown.
 * @param event Show event.
 */
void GensWindow::showEvent(QShowEvent *event)
{
	if (m_hasInitResize)
		return;
	
	// Run the initial resize.
	m_hasInitResize = true;
	gensResize();
}


/**
 * gensResize(): Resize the Gens window to show the image at its expected size.
 */
void GensWindow::gensResize(void)
{
	// Get the drawing size.
	// TODO: Scale for larger renderers.
	int img_width = 320 * m_scale;
	int img_height = 240 * m_scale;
	
	// Enforce a minimum size of 320x240.
	if (img_width < 320)
		img_width = 320;
	if (img_height < 240)
		img_height = 240;
	
	// Calculate the window height.
	int win_height = img_height;
	if (m_menubar)
		win_height += m_menubar->size().height();
	
	// Set the new window size.
	this->resize(img_width, win_height);
}


/** Slots. **/


/**
 * menuTriggered(): Menu item triggered.
 * @param id Menu item ID.
 */
void GensWindow::menuTriggered(int id)
{
	switch (MNUID_MENU(id))
	{
		case IDM_FILE_MENU:
			// File menu.
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_FILE_OPEN):
					m_emuManager.openRom(this);
					break;
				
				case MNUID_ITEM(IDM_FILE_CLOSE):
					m_emuManager.closeRom();
					break;
				
				case MNUID_ITEM(IDM_FILE_QUIT):
					// Quit.
					m_emuManager.closeRom();
					QuitGens();
					this->close();
					break;
				
				default:
					break;
			}
			break;
		
		case IDM_HELP_MENU:
			// Help menu.
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_HELP_ABOUT):
					// About Gens/GS II.
					AboutWindow::ShowSingle(this);
					break;
				
				default:
					break;
			}
			break;
		
		case IDM_RESBPPTEST_MENU:
			// Resolution / Color Depth Testing.
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_RESBPPTEST_1X):
					m_scale = 1;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_2X):
					m_scale = 2;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_3X):
					m_scale = 3;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_4X):
					m_scale = 4;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_15):
					LibGens::VdpRend::m_palette.setBpp(LibGens::VdpPalette::BPP_15);
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_16):
					LibGens::VdpRend::m_palette.setBpp(LibGens::VdpPalette::BPP_16);
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_32):
					LibGens::VdpRend::m_palette.setBpp(LibGens::VdpPalette::BPP_32);
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				default:
					break;
			}
			break;
		
		case IDM_CTRLTEST_MENU:
			// Controller Testing
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_CTRLTEST_NONE):
					m_emuManager.setController(0, LibGens::IoBase::IOT_NONE);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_3BT):
					m_emuManager.setController(0, LibGens::IoBase::IOT_3BTN);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_6BT):
					m_emuManager.setController(0, LibGens::IoBase::IOT_6BTN);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_2BT):
					m_emuManager.setController(0, LibGens::IoBase::IOT_2BTN);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_MEGAMOUSE):
					m_emuManager.setController(0, LibGens::IoBase::IOT_MEGA_MOUSE);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_TEAMPLAYER):
					m_emuManager.setController(0, LibGens::IoBase::IOT_TEAMPLAYER);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_4WP):
					// TODO
					m_emuManager.setController(0, LibGens::IoBase::IOT_4WP_SLAVE);
					m_emuManager.setController(1, LibGens::IoBase::IOT_4WP_MASTER);
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_CONFIG):
					// Controller Configuration.
					CtrlConfigWindow::ShowSingle(this);
					break;
				
				default:
					break;
			}
		
		default:
			break;
	}
}


/**
 * setGensTitle(): Set the Gens window title.
 */
void GensWindow::setGensTitle(void)
{
	// TODO: Indicate UI status.
	QString title = TR("Gens/GS II");
	title += " " + TR("dev") + " - ";
	
	// TODO
#if 0
	if (!m_rom)
	{
		// No ROM is running.
		title += TR("Idle");
	}
	else
	{
		// ROM is running.
		// TODO: Determine the system. For now, assume Genesis.
		title += "Genesis: ";
		title += QString(m_rom->romNameUS());
	}
#endif
	
	this->setWindowTitle(title);
}


/**
 * osd(): LibGens OSD handler.
 * @param osd_type: OSD type.
 * @param param: Integer parameter.
 */
void GensWindow::osd(OsdType osd_type, int param)
{
	switch (osd_type)
	{
		case OSD_SRAM_LOAD:
			m_vBackend->osd_printf(1500, "SRAM loaded. (%d bytes)", param);
			break;
		
		case OSD_SRAM_SAVE:
			m_vBackend->osd_printf(1500, "SRAM saved. (%d bytes)", param);
			break;
		
		case OSD_SRAM_AUTOSAVE:
			m_vBackend->osd_printf(1500, "SRAM autosaved. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_LOAD:
			m_vBackend->osd_printf(1500, "EEPROM loaded. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_SAVE:
			m_vBackend->osd_printf(1500, "EEPROM saved. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_AUTOSAVE:
			m_vBackend->osd_printf(1500, "EEPROM autosaved. (%d bytes)", param);
			break;
		
		default:
			// Unknown OSD type.
			break;
	}
}

}
