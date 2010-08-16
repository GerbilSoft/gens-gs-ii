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

// Controller devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"
#include "libgens/IO/IoTeamplayer.hpp"

// Test loading ROMs.
#include "libgens/Rom.hpp"

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
	
	// No ROM is loaded at startup.
	// TODO: Move this to another file.
	// NOTE: This must be set before calling setupUi()!
	m_rom = NULL;
	
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
					openRom();
					break;
				
				case MNUID_ITEM(IDM_FILE_CLOSE):
					closeRom();
					break;
				
				case MNUID_ITEM(IDM_FILE_QUIT):
					// Quit.
					closeRom();
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
				
				case MNUID_ITEM(IDM_CTRLTEST_TEAMPLAYER):
					m_ctrlChange = 5;
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
			
			case 5:
				// Sega Teamplayer.
				controller = new LibGens::IoTeamplayer(LibGens::EmuMD::m_port1);
				// TODO: Copy settings from existing Port 1 controller.
				m_vBackend->osd_printf(1500, "Port 1 set to SEGA TEAMPLAYER.");
				break;
			
			default:
				break;
		}
		
		if (controller)
		{
			delete LibGens::EmuMD::m_port1;
			LibGens::EmuMD::m_port1 = controller;
			
			// Print controller information.
			printf("Port 1: %s - %d buttons: ", controller->devName(), controller->numButtons());
			if (controller->numButtons() > 0)
			{
				int btn = 0;
				while (btn >= 0)
				{
					if (btn != 0)
						printf(", ");
					printf("%s", controller->buttonName(btn));
					btn = controller->nextLogicalButton(btn);
				}
				printf("\n");
			}
			else
			{
				// No buttons.
				printf("(none)\n");
			}
		}
		
		m_ctrlChange = -1;
	}
	
	// Update the GensQGLWidget.
	m_vBackend->setVbDirty();
	m_vBackend->vbUpdate();
	
#ifdef _WIN32
	// Update Shift/Control/Alt states.
	// TODO: Only do this if the input backend doesn't support L/R modifiers natively.
	// QWidget doesn't; GLFW does.
	// TODO: When should these key states be updated?
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_LSHIFT, VK_LSHIFT);
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_RSHIFT, VK_RSHIFT);
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_LCTRL, VK_LCONTROL);
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_RCTRL, VK_RCONTROL);
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_LALT, VK_LMENU);
	LibGens::KeyManager::WinKeySet(LibGens::KEYV_RALT, VK_RMENU);
#endif
	
	// Tell the emulation thread that we're ready for another frame.
	if (gqt4_emuThread)
		gqt4_emuThread->resume();
}


/**
 * openRom(): Open a ROM file.
 * TODO: Move this to another file!
 */
void GensWindow::openRom(void)
{
	// TODO: Move to another function and/or another file.
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"
	
	QString filename = QFileDialog::getOpenFileName(this,
			TR("Open ROM"),		// Dialog title
			"",			// Default filename.
			TR("Sega Genesis ROM images") + " (*.bin *.gen);;" +
#if 0
			TR("Sega Genesis / 32X ROMs; Sega CD disc images") +
			"(*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT ");;" +
#endif
			TR("All Files") + "(*.*)");
	if (filename.isEmpty())
		return;
	
	if (gqt4_emuThread || m_rom)
	{
		// Close the ROM first.
		closeRom();
	}
	
	// Open the file using the LibGens::Rom class.
	// TODO: This won't work for KIO...
	m_rom = new LibGens::Rom(filename.toUtf8().constData());
	if (!m_rom->isOpen())
	{
		// Couldn't open the ROM file.
		printf("Error opening ROM file. (TODO: Get error information.)\n");
		delete m_rom;
		m_rom = NULL;
		return;
	}
	
	printf("ROM information: format == %d, system == %d\n", m_rom->romFormat(), m_rom->sysId());
	
	if (m_rom->sysId() != LibGens::Rom::MDP_SYSTEM_MD)
	{
		// Only MD ROM images are supported.
		printf("ERROR: Only Sega Genesis / Mega Drive ROM images are supported right now.\n");
		delete m_rom;
		m_rom = NULL;
		return;
	}
	
	if (m_rom->romFormat() != LibGens::Rom::RFMT_BINARY)
	{
		// Only binary ROM images are supported.
		printf("ERROR: Only binary ROM images are supported right now.\n");
		delete m_rom;
		m_rom = NULL;
		return;
	}
	
	// Load the ROM image in EmuMD.
	LibGens::EmuMD::SetRom(m_rom);
	m_rom->close();
	
	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	
	// Start the emulation thread.
	gqt4_emuThread = new EmuThread();
	QObject::connect(gqt4_emuThread, SIGNAL(frameDone(void)),
			 this, SLOT(emuFrameDone(void)));
	gqt4_emuThread->start();
	
	// Update the Gens title.
	setGensTitle();
}


/**
 * openRom(): Close the loaded ROM file.
 * TODO: Move this to another file!
 */
void GensWindow::closeRom(void)
{
	if (gqt4_emuThread)
	{
		// Stop the emulation thread.
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	if (m_rom)
	{
		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		LibGens::EmuMD::SaveData(m_rom);
		
		// Delete the Rom instance.
		delete m_rom;
		m_rom = NULL;
	}
	
	// TODO: Clear the screen, start the idle animation, etc.
	
	// Update the Gens title.
	setGensTitle();
}


/**
 * setGensTitle(): Set the Gens window title.
 */
void GensWindow::setGensTitle(void)
{
	// TODO: Indicate UI status.
	QString title = TR("Gens/GS II");
	title += " " + TR("dev") + " - ";
	
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
