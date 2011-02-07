/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_mac.cpp: General Configuration Window. (Mac OS X)   *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

// Make sure this file is only compiled on Mac OS X.
#include <QtCore/qglobal.h>
#ifndef Q_WS_MAC
#error GeneralConfigWindow_mac.cpp is for Mac OS X only!
#endif

#include "GeneralConfigWindow.hpp"

// Qt includes.
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>

// Mac OS X includes.
#ifdef QT_MAC_USE_COCOA
// TODO: Cocoa window.
#else
// Carbon window.
#include <Carbon/Carbon.h>
#ifndef kWindowToolbarButtonAttribute
#define kWindowToolbarButtonAttribute (1 << 6)
#endif
#endif


namespace GensQt4
{

/**
 * setupUi_mac(): Set up the Mac OS X-specific UI elements.
 */
void GeneralConfigWindow::setupUi_mac(void)
{
#if QT_VERSION > 0x040300
	// Set the unified titlebar / toolbar property.
	// This property was added in Qt 4.3.
	setUnifiedTitleAndToolBarOnMac(true);
#endif
	
	// Remove the window icon.
	// SuitCase says the window icon is only used if there's
	// a document open in the window, i.e. a "proxy icon".
	this->setWindowIcon(QIcon());
	
	// Create the toolbar.
	toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(32, 32));
	toolBar->setMovable(false);
	toolBar->setFloatable(false);
	toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolBar->layout()->setSpacing(0);
	toolBar->layout()->setMargin(0);
	addToolBar(Qt::TopToolBarArea, toolBar);
	
	// Toolbar buttons.
	struct ToolBarBtn
	{
		const char *text;
		const char *iconQrc;
	};
	
	ToolBarBtn btns[] =
	{
		// TODO: Use a Sega CD icon instead of a generic CD-ROM icon.
		// TODO: Use Mac generic program icon instead of Oxygen.
		{"&General",		":/oxygen-32x32/configure.png"},
		{"G&raphics",		":/oxygen-32x32/applications-graphics.png"},
		{"Sega &CD",		":/oxygen-32x32/media-optical.png"},
		{"E&xternal Programs",	":/oxygen-32x32/utilities-terminal.png"},
		
		// End of list.
		{NULL, NULL}
	};
	
	// Create the action group for the toolbar buttons.
	QActionGroup *agBtnGroup = new QActionGroup(this);
	agBtnGroup->setExclusive(true);
	connect(agBtnGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(toolbarTriggered(QAction*)));
	
	// Add the toolbar buttons.
	// TODO: See if these need to be deleted manually.
	for (int i = 0; i < (int)(sizeof(btns)/sizeof(btns[0])-1); i++)
	{
		QAction *action = new QAction(QIcon(QString::fromLatin1(btns[i].iconQrc)),
						tr(btns[i].text), this);
		action->setCheckable(true);
		action->setData(i);	// used for tab selection
		if (i == 0)
			action->setChecked(true);
		
		// TODO: Connect the triggered() signal from the QSignalMapper.
		
		// Add the action to the action group and the toolbar.
		agBtnGroup->addAction(action);
		toolBar->addAction(action);
	}
	
	// Create a stacked widget and move the tab contents over.
	stackedWidget = new QStackedWidget(this);
	stackedWidget->addWidget(tabGeneral);
	tabGeneral->setContentsMargins(0, 0, 0, 0);
	stackedWidget->addWidget(tabGraphics);
	tabGraphics->setContentsMargins(0, 0, 0, 0);
	stackedWidget->addWidget(tabSegaCD);
	tabSegaCD->setContentsMargins(0, 0, 0, 0);
	stackedWidget->addWidget(tabExtPrg);
	tabExtPrg->setContentsMargins(0, 0, 0, 0);
	vboxDialog->insertWidget(0, stackedWidget);
	
	// Delete the old tab widget.
	delete tabWidget;
	tabWidget = NULL;
	
	// Increase the margins for the QDialogButtonBox.
	// TODO: Remove QDialogButtonBox on Mac OS X,
	// and have settings apply immediately.
	vboxButtonBox->setContentsMargins(16, 16, 16, 16);
	
	// Hide the toolbar show/hide button in the titlebar.
	// TODO: Do this on a window update event too?
#ifdef QT_MAC_USE_COCOA
	// Qt is using Cocoa.
	// TODO
#else
	// Qt is using Carbon.
	ChangeWindowAttributes(qt_mac_window_for(this),
				0, kWindowToolbarButtonAttribute);
#endif
}


/**
 * toolbarTriggered(): A toolbar button was clicked.
 * @param action Toolbar button.
 */
void GeneralConfigWindow::toolbarTriggered(QAction *action)
{
	QVariant data = action->data();
	if (!data.isValid() || !data.canConvert(QVariant::Int))
		return;
	
	int tab = data.toInt();
	if (tab < 0 || tab >= stackedWidget->count())
		return;
	
	stackedWidget->setCurrentIndex(tab);
}

}
