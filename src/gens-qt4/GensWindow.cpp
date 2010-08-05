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

// Test loading ROMs.
#include "libgens/Rom.hpp"

// C includes. (Needed for fps timing.)
#include <stdio.h>

// Timing functions.
// TODO: Move to a separate file. (Maybe move to libgens?)
#ifdef HAVE_LIBRT
#include <time.h>
#else
#include <sys/time.h>
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
	m_emuThread = NULL;
	
	// Set up the User Interface.
	setupUi();
}


/**
 * ~GensWindow(): Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
	if (m_emuThread)
	{
		m_emuThread->stop();
		m_emuThread->wait();
		delete m_emuThread;
		m_emuThread = NULL;
	}
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
	
	// Create the QGLWidget.
	// TODO: Make this use a pluggable backend instead.
	m_glWidget = new GensQGLWidget(this->centralwidget);
	
	// Create the layout.
	layout = new QVBoxLayout(this->centralwidget);
	layout->setObjectName(QString::fromUtf8("layout"));
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_glWidget);
	centralwidget->setLayout(layout);
	
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
					QString filename = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", 
							tr("Sega CD / 32X / Genesis ROMs (*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT));
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
					m_glWidget->setDirty();
					m_glWidget->updateGL();
					break;
				
				case MNUID_ITEM(IDM_FILE_EMUTHREAD):
					if (m_emuThread != NULL)
					{
						// Emulation thread is already running. Stop it.
						m_emuThread->stop();
						m_emuThread->wait();
						delete m_emuThread;
						m_emuThread = NULL;
						break;
					}
					
					// Emulation thread isn't running. Start it.
					m_emuThread = new EmuThread();
					QObject::connect(m_emuThread, SIGNAL(frameDone(void)),
							 this, SLOT(emuFrameDone(void)));
					m_emuThread->start();
					break;
				
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
		
		case IDM_RESTEST_MENU:
			// Resolution Testing.
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_RESTEST_1X):
					m_scale = 1;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESTEST_2X):
					m_scale = 2;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESTEST_3X):
					m_scale = 3;
					gensResize();
					break;
				
				case MNUID_ITEM(IDM_RESTEST_4X):
					m_scale = 4;
					gensResize();
					break;
				
				default:
					break;
			}
			break;
		
		case IDM_BPPTEST_MENU:
			// Color Depth Testing.
			switch (MNUID_ITEM(id))
			{
				case MNUID_ITEM(IDM_BPPTEST_15):
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_15;
					m_glWidget->setDirty();
					m_glWidget->updateGL();
					break;
				
				case MNUID_ITEM(IDM_BPPTEST_16):
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_16;
					m_glWidget->setDirty();
					m_glWidget->updateGL();
					break;
				
				case MNUID_ITEM(IDM_BPPTEST_32):
					LibGens::VdpRend::Bpp = LibGens::VdpRend::BPP_32;
					m_glWidget->setDirty();
					m_glWidget->updateGL();
					break;
				
				default:
					break;
			}
		
		default:
			break;
	}
}


static double getTime(void)
{
	// TODO: Use integer arithmetic instead of floating-point.
	// TODO: Add Win32-specific versions that use QueryPerformanceCounter() and/or GetTickCount().
	// TODO: Move to a separate file. (Maybe move to libgens?)
#ifdef HAVE_LIBRT
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec + (ts.tv_nsec / 1000000000.0));
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + (tv.tv_usec / 1000000.0));
#endif
}


void GensWindow::emuFrameDone(void)
{
	static double lastTime = 0;
	static int frames = 0;
	frames++;
	
	if (lastTime < 0.001)
		lastTime = getTime();
	else
	{
		double thisTime = getTime();
		if ((thisTime - lastTime) >= 1.00)
		{
			// Print fps.
			printf("fps: %f\n", ((double)frames / (thisTime - lastTime)));
			lastTime = thisTime;
			frames = 0;
		}
	}
	
	// Update the GensQGLWidget.
	m_glWidget->setDirty();
	m_glWidget->updateGL();
	
	// Tell the emulation thread that we're ready for another frame.
	if (m_emuThread)
		m_emuThread->resume();
}

}
