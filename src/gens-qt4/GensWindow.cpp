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

#include "GensWindow.hpp"
#include "gqt4_main.hpp"

// Qt4 windows.
#include "AboutWindow.hpp"

// LibGens.
#include "libgens/lg_main.hpp"
#include "libgens/MD/EmuMD.hpp"

// C++ includes.
#include <algorithm>

// Qt4 includes.
#include <QtCore/QString>
#include <QtGui/QIcon>

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
	this->setWindowTitle(TR("Gens/GS II"));
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
	if (MNUID_MENU(id) == IDM_FILE_MENU)
	{
		// File menu.
		switch (MNUID_ITEM(id))
		{
			case MNUID_ITEM(IDM_FILE_BLIT):
				// Blit!
				LibGens::EmuMD::Init_TEST();
				m_glWidget->setDirty();
				m_glWidget->updateGL();
				break;
			
			case MNUID_ITEM(IDM_FILE_QUIT):
				// Quit.
				QuitGens();
				this->close();
				break;
			
			default:
				break;
		}
	}
	else if (MNUID_MENU(id) == IDM_HELP_MENU)
	{
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
	}
	else if (MNUID_MENU(id) == IDM_RESTEST_MENU)
	{
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
	}
}

}
