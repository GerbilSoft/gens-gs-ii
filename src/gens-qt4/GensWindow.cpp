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

// LibGens.
#include "libgens/lg_main.hpp"
#include "libgens/MD/EmuMD.hpp"
#include "libgens/macros/git.h"
#include "libgens/macros/log_msg.h"
#include "libgens/Util/Timing.hpp"

// Controller devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"

// Test loading ROMs.
#include "libgens/Rom.hpp"

// Video Backend classes.
#include "VBackend/GensQGLWidget.hpp"

// C includes. (Needed for fps timing.)
#include <stdio.h>

#ifdef _WIN32
// Win32 needs io.h for dup().
// TODO: Move File/Open to another file.
#include <io.h>
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
	
	// Controller change.
	// NOTE: DEBUG CODE: Remove this later.
	m_ctrlChange = -1;
}


/**
 * ~GensWindow(): Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
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
	
	// Retranslate the UI.
	retranslateUi();
}


/**
 * retranslateUi(): Retranslate the User Interface.
 */
void GensWindow::retranslateUi(void)
{
	// TODO: Indicate UI status.
	QString title = TR("Gens/GS II");
	title += " - ";
	title += TR("Development Build");
#ifdef GENS_GIT_VERSION
	title += " (" GENS_GIT_VERSION ")";
#endif
	
	this->setWindowTitle(title);
}


/**
 * closeEvent(): Window is being closed.
 * @param event Close event.
 */
void GensWindow::closeEvent(QCloseEvent *event)
{
	// Quit.
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
				{
					// Open ROM.
					// TODO: Move to another function and/or another file.
					// TODO: Proper compressed file support.
					#define ZLIB_EXT " *.zip *.zsg *.gz"
					#define LZMA_EXT " *.7z"
					#define RAR_EXT " *.rar"
					QString filename = QFileDialog::getOpenFileName(this, TR("Open ROM"), "", 
							TR("Sega Genesis / 32X ROMs; Sega CD disc images") +
							"(*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT ");;" +
							TR("All Files") + "(*.*)");
					if (filename.isEmpty())
						break;
					
					// Open the file.
					QFile m_file(filename);
					if (!m_file.open(QIODevice::ReadOnly))
					{
						// Error opening the file.
						printf("Error opening file %s: %d\n", filename.toUtf8().constData(), m_file.error());
						break;
					}
					
					// Dup the file and open it in LibGens.
					// LibGens::Rom will close f once it's done using it.
					FILE *f = fdopen(dup(m_file.handle()), "rb");
					LibGens::Rom *m_rom = new LibGens::Rom(f);
					printf("ROM information: format == %d, system == %d\n", m_rom->romFormat(), m_rom->sysId());
					
					// TODO: Process the ROM image.
					delete m_rom;
					break;
				}
				
				case MNUID_ITEM(IDM_FILE_BLIT):
					// Blit!
					LibGens::EmuMD::Init_TEST();
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				case MNUID_ITEM(IDM_FILE_EMUTHREAD):
				{
					bool checked = m_menubar->menuItemCheckState(IDM_FILE_EMUTHREAD);
					if (gqt4_emuThread == NULL && !checked)
						break;
					else if (gqt4_emuThread != NULL && checked)
						break;
					
					// Toggle the emulation thread state.
					if (gqt4_emuThread != NULL)
					{
						// Emulation thread is already running. Stop it.
						gqt4_emuThread->stop();
						gqt4_emuThread->wait();
						delete gqt4_emuThread;
						gqt4_emuThread = NULL;
						break;
					}
					
					// Emulation thread isn't running. Start it.
					gqt4_emuThread = new EmuThread();
					QObject::connect(gqt4_emuThread, SIGNAL(frameDone(void)),
							 this, SLOT(emuFrameDone(void)));
					gqt4_emuThread->start();
					break;
				}
				
				case MNUID_ITEM(IDM_FILE_QUIT):
					// Quit.
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
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_15;
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_16):
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_16;
					m_vBackend->setVbDirty();
					m_vBackend->vbUpdate();
					break;
				
				case MNUID_ITEM(IDM_RESBPPTEST_32):
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_32;
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
					m_ctrlChange = 0;
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_3BT):
					m_ctrlChange = 1;
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_6BT):
					m_ctrlChange = 2;
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_2BT):
					m_ctrlChange = 3;
					break;
				
				case MNUID_ITEM(IDM_CTRLTEST_MEGAMOUSE):
					m_ctrlChange = 4;
					break;
				
				default:
					break;
			}
		
		default:
			break;
	}
}


void GensWindow::emuFrameDone(void)
{
	static double lastTime = 0;
	static int frames = 0;
	frames++;
	
	if (lastTime < 0.001)
		lastTime = LibGens::Timing::GetTimeD();
	else
	{
		double thisTime = LibGens::Timing::GetTimeD();
		if ((thisTime - lastTime) >= 0.250)
		{
			// Push the current fps.
			// (Updated four times per second.)
			m_vBackend->pushFps((double)frames / (thisTime - lastTime));
			lastTime = thisTime;
			frames = 0;
		}
	}
	
	// Check if the controller was changed.
	// NOTE: DEBUG CODE: Remove this later!
	if (m_ctrlChange != -1)
	{
		LibGens::IoBase *controller = NULL;
		switch (m_ctrlChange)
		{
			case 0:
				// No controller.
				controller = new LibGens::IoBase(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to NONE.");
				break;
			
			case 1:
				// 3-button controller.
				controller = new LibGens::Io3Button(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to 3-BUTTON.");
				break;
			
			case 2:
				// 6-button controller.
				controller = new LibGens::Io6Button(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to 6-BUTTON.");
				break;
			
			case 3:
				// 2-button controller.
				controller = new LibGens::Io2Button(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to 2-BUTTON.");
				break;
			
			case 4:
				// Sega Mega Mouse.
				controller = new LibGens::IoMegaMouse(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to SEGA MEGA MOUSE.");
				break;
			
			default:
				break;
		}
		
		if (controller)
		{
			delete LibGens::EmuMD::m_port1;
			LibGens::EmuMD::m_port1 = controller;
		}
		
		m_ctrlChange = -1;
	}
	
	// Update the GensQGLWidget.
	m_vBackend->setVbDirty();
	m_vBackend->vbUpdate();
	
	// Tell the emulation thread that we're ready for another frame.
	if (gqt4_emuThread)
		gqt4_emuThread->resume();
}

}
