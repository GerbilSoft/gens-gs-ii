/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlCfgWidget.hpp: Controller configuration widget.                 *
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

#include "GensCtrlCfgWidget.hpp"

// Qt includes.
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>

// GensKeySequenceWidget.
// TODO: Add property for "single key" and add gamepad support.
#include "GensKeySequenceWidget.hpp"

// I/O devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"

#define MAX_CFG_BTNS 12

namespace GensQt4
{

class GensCtrlCfgWidgetPrivate
{
	public:
		GensCtrlCfgWidgetPrivate(GensCtrlCfgWidget *q);
		~GensCtrlCfgWidgetPrivate();
		void init(void);
		
		inline LibGens::IoBase::IoType ioType(void);
		void setIoType(LibGens::IoBase::IoType newIoType);
	
	private:
		GensCtrlCfgWidget *const q;
		LibGens::IoBase::IoType m_ioType;
		
		QGridLayout *m_layout;
		QLabel *m_lblCfg[MAX_CFG_BTNS];
		GensKeySequenceWidget *m_btnCfg[MAX_CFG_BTNS];
};


/***************************************
 * GensCtrlCfgWidgetPrivate functions. *
 ***************************************/

GensCtrlCfgWidgetPrivate::GensCtrlCfgWidgetPrivate(GensCtrlCfgWidget *q)
	: q(q)
	, m_ioType(LibGens::IoBase::IOT_NONE)
	, m_layout(new QGridLayout(q))
{
	// Eliminate margins.
	m_layout->setContentsMargins(0, 0, 0, 0);
	
	// Reduce vertical spacing in the grid layout.
	m_layout->setVerticalSpacing(0);
}


GensCtrlCfgWidgetPrivate::~GensCtrlCfgWidgetPrivate()
{
	// Delete all the labels and buttons.
	// TODO: Is this necessary?
	for (size_t i = 0; i < (sizeof(m_lblCfg)/sizeof(m_lblCfg[0])); i++)
	{
		delete m_lblCfg[i];
		delete m_btnCfg[i];
	}
}


/**
 * GensCtrlCfgWidgetPrivate::init(): Initialize the grid layout.
 */
void GensCtrlCfgWidgetPrivate::init(void)
{
	// Add MAX_CFG_BTNS items to the grid layout.
	for (size_t i = 0; i < (sizeof(m_lblCfg)/sizeof(m_lblCfg[0])); i++)
	{
		m_lblCfg[i] = new QLabel();
		m_lblCfg[i]->setVisible(false);
		m_btnCfg[i] = new GensKeySequenceWidget();
		m_btnCfg[i]->setVisible(false);
		m_layout->addWidget(m_lblCfg[i], i, 0, Qt::AlignLeft);
		m_layout->addWidget(m_btnCfg[i], i, 1, Qt::AlignRight);
	}
}


/**
 * GensCtrlCfgWidget::ioType(): Get the current I/O type.
 * @return Current I/O type.
 */
inline LibGens::IoBase::IoType GensCtrlCfgWidgetPrivate::ioType(void)
	{ return m_ioType; }

/**
 * GensCtrlCfgWidgetPrivate::setIoType(): Set the I/O type.
 * @param newIoType New I/O type.
 */
void GensCtrlCfgWidgetPrivate::setIoType(LibGens::IoBase::IoType newIoType)
{
	if (m_ioType == newIoType)
		return;
	
	// Save the new I/O type.
	m_ioType = newIoType;
	
	// Update the grid layout based on the specified controller type.
	LibGens::IoBase *ctrl;
	switch (newIoType)
	{
		default:
		case LibGens::IoBase::IOT_NONE:	ctrl = new LibGens::IoBase(); break;
		case LibGens::IoBase::IOT_3BTN:	ctrl = new LibGens::Io3Button(); break;
		case LibGens::IoBase::IOT_6BTN:	ctrl = new LibGens::Io6Button(); break;
		case LibGens::IoBase::IOT_2BTN:	ctrl = new LibGens::Io2Button(); break;
		
		// TODO: Other devices.
#if 0
		IOT_TEAMPLAYER	= 5,
		IOT_4WP_MASTER	= 6,
		IOT_4WP_SLAVE	= 7,
#endif
	}
	
	// Get the number of buttons.
	int numButtons = ctrl->numButtons();
	if (numButtons > MAX_CFG_BTNS)
		numButtons = MAX_CFG_BTNS;
	
	// Show the buttons, in logical button order.
	QString sBtnLabel;
	const char *cBtnLabel;
	for (int i = 0, button = 0;
	     i < numButtons && button >= 0; i++)
	{
		cBtnLabel = ctrl->buttonName(button);
		if (!cBtnLabel)
			sBtnLabel.clear();
		else
			sBtnLabel = QLatin1String(cBtnLabel) + QChar(L':');
		
		m_lblCfg[i]->setText(sBtnLabel);
		m_lblCfg[i]->setVisible(true);
		m_btnCfg[i]->setVisible(true);
		
		// Get the next logical button.
		button = ctrl->nextLogicalButton(button);
	}
	
	// Hide other buttons.
	for (int i = numButtons; i < MAX_CFG_BTNS; i++)
	{
		m_lblCfg[i]->setVisible(false);
		m_btnCfg[i]->setVisible(false);
	}
	
	// Delete the IoBase object
	delete ctrl;
}


/********************************
 * GensCtrlCfgWidget functions. *
 ********************************/

GensCtrlCfgWidget::GensCtrlCfgWidget(QWidget* parent)
	: QWidget(parent)
	, d(new GensCtrlCfgWidgetPrivate(this))
{
	// Initialize the private members.
	d->init();
}

GensCtrlCfgWidget::~GensCtrlCfgWidget()
{
	delete d;
}


/**
 * GensCtrlCfgWidget::ioType(): Get the current I/O type.
 * @return Current I/O type.
 */
LibGens::IoBase::IoType GensCtrlCfgWidget::ioType(void)
	{ return d->ioType(); }

/**
 * GensCtrlCfgWidget::setIoType(): Set the current I/O type.
 * @param newIoType New I/O type.
 */
void GensCtrlCfgWidget::setIoType(LibGens::IoBase::IoType newIoType)
	{ d->setIoType(newIoType); }

}
