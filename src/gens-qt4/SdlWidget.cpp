/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * SdlWidget.cpp: SDL Widget class.                                        *
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

#include "SdlWidget.hpp"

#include "libgens/lg_main.hpp"
#include "libgens/SdlVideo.hpp"

namespace GensQt4
{

SdlWidget::SdlWidget(QWidget *parent)
	: SDLWIDGET_BASECLASS(parent)
{
	if (parent != NULL)
	{
		// Parent widget specified.
		
		// Make sure the window is embedded properly.
		this->setAttribute(Qt::WA_NativeWindow);
		this->setAttribute(Qt::WA_PaintOnScreen);
		this->setAttribute(Qt::WA_OpaquePaintEvent);
		
		// Don't draw the background.
		this->setAttribute(Qt::WA_NoBackground);
		this->setAttribute(Qt::WA_NoSystemBackground);
		
		// Use a fixed size policy.
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
	else
	{
		// No parent widget specified. Run SDL in its own window.
		this->showEvent(NULL);
	}
}


/**
 * paintEvent(): Widget needs to be repainted.
 * @param event QPaintEvent.
 */
void SdlWidget::paintEvent(QPaintEvent *event)
{
	// Send an UPDATE event to LibGens.
	// TODO: Include the coordinates to update?
	LibGens::qToLG->push(LibGens::MtQueue::MTQ_LG_UPDATE, NULL);
}


/**
 * showEvent(): Widget is being shown.
 * @param event QShowEvent.
 */
void SdlWidget::showEvent(QShowEvent *event)
{
	static bool lgInit = false;
	
	if (lgInit)
	{
		// LibGens is already initialized.
		return;
	}
	
	// Initialize LibGens.
	void *wId = (void*)(this->parent() ? this->winId() : 0);
	printf("omg - wid: 0x%08X\n", wId);
	int ret = LibGens::Init(wId);
	if (ret != 0)
	{
		// TODO: Error handling.
		return;
	}
	// NOTE: Call gensResize() after receiving acknowledgement of the window initialization.
	//gens_window.gensResize();
	// NOTE: Emit resizeEvent() after receiving acknowledgement of the window initialization.
	emit sdlHasResized(QSize(LibGens::SdlVideo::Width(), LibGens::SdlVideo::Height()));
	
	uint32_t color;
	color = (255 | (128 << 8) | (64 << 16));
	LibGens::qToLG->push(LibGens::MtQueue::MTQ_LG_SETBGCOLOR, (void*)color);
}

}
