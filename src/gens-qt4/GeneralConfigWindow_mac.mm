/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_mac.mm: General Configuration Window. (Mac OS X)    *
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
#include "GensQApplication.hpp"

// Qt includes.
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>

// Mac OS X includes.
#ifndef QT_MAC_USE_COCOA
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
		const char *iconFdo;
	};
	
	static const ToolBarBtn btns[] =
	{
		// TODO: Use a Sega CD icon instead of a generic CD-ROM icon.
		// TODO: Use Mac generic program icon instead of Oxygen.
		{"&General",		"configure"},
		{"G&raphics",		"applications-graphics"},
		{"&System",		"applications-system"},
		{"Sega &CD",		"media-optical"},
		{"E&xternal Programs",	"utilities-terminal"},
		
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
		QAction *action = new QAction(
				GensQApplication::IconFromTheme(QLatin1String(btns[i].iconFdo)),
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
	
	// Create a stacked widget.
	// Size obtained from Qt Designer.
	// (We need to specify the size here because of the fixed layout hack.)
	// TODO: Remove extra height from the tabs.
	const QSize szStacked(478, 409);
	stackedWidget = new QStackedWidget(this);
	stackedWidget->setMinimumSize(szStacked);
	stackedWidget->setMaximumSize(szStacked);
	stackedWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	// Move the tabs to the stacked widget.
	stackedWidget->addWidget(tabGeneral);
	tabGeneral->setContentsMargins(0, 0, 0, 0);
	stackedWidget->addWidget(tabGraphics);
	tabGraphics->setContentsMargins(0, 0, 0, 0);
	stackedWidget->addWidget(tabSystem);
	tabSystem->setContentsMargins(0, 0, 0, 0);
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
	
	// Force a fixed-size layout.
	// Mac OS X's Carbon and Cocoa libraries are annoying and
	// will enable the Zoom button if the layout isn't fixed.
	// (On Cocoa, using Qt::CustomizeWindowHint breaks the
	//  unified titlebar/toolbar setup.)
	this->layout()->setSizeConstraint(QLayout::SetFixedSize);
	
	// Hide the toolbar show/hide button in the titlebar.
	// TODO: Do this on a window update event too?
#ifdef QT_MAC_USE_COCOA
	// Qt is using Cocoa.
	OSWindowRef window = qt_mac_window_for(this);
	[window setShowsToolbarButton:false];
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
