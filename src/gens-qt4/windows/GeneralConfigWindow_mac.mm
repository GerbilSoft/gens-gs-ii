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
 * Set up the Mac OS X-specific UI elements.
 */
void GeneralConfigWindow::setupUi_mac(void)
{
	// Remove the window icon.
	// SuitCase says the window icon is only used if there's
	// a document open in the window, i.e. a "proxy icon".
	this->setWindowIcon(QIcon());
		
	// Increase the margins for the QDialogButtonBox.
	// TODO: Remove QDialogButtonBox on Mac OS X,
	// and have settings apply immediately.
	vboxButtonBox->setContentsMargins(16, 16, 16, 16);
	
	// Force a fixed-size layout.
	// Mac OS X's Carbon and Cocoa libraries are annoying and
	// will enable the Zoom button if the layout isn't fixed.
	// (On Cocoa, using Qt::CustomizeWindowHint breaks the
	//  unified titlebar/toolbar setup.)
	// TODO: Re-verify this with the recent GeneralConfigWindow changes.
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

}
