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
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>

// GensKeySequenceWidget.
// TODO: Add property for "single key" and add gamepad support.
#include "GensKeySequenceWidget.hpp"
#include "GensCtrlKeyWidget.hpp"

// I/O devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"

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
		
		static QString ButtonName_l(LibGens::IoBase::ButtonName_t buttonName);
		
		void clearAllButtons(void);
	
	private:
		GensCtrlCfgWidget *const q;
		Q_DISABLE_COPY(GensCtrlCfgWidgetPrivate)
		
		LibGens::IoBase::IoType m_ioType;
		
		QGridLayout *m_layout;
		QLabel *m_lblButtonName[MAX_CFG_BTNS];
		QLabel *m_lblKeyDisplay[MAX_CFG_BTNS];
		GensCtrlKeyWidget *m_btnCfg[MAX_CFG_BTNS];
		QSpacerItem *m_vspcCfg;
		
		// "Change All", "Clear All".
		QPushButton *btnChangeAll;
		QPushButton *btnClearAll;
		QHBoxLayout *hboxOptions;
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
	for (size_t i = 0; i < MAX_CFG_BTNS; i++)
	{
		delete m_lblButtonName[i];
		delete m_lblKeyDisplay[i];
		delete m_btnCfg[i];
	}
}


/**
 * GensCtrlCfgWidgetPrivate::init(): Initialize the grid layout.
 */
void GensCtrlCfgWidgetPrivate::init(void)
{
	// Monospaced font.
	QFont fntMonospace(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	
	// Add MAX_CFG_BTNS items to the grid layout.
	for (size_t i = 0; i < MAX_CFG_BTNS; i++)
	{
		m_lblButtonName[i] = new QLabel();
		m_lblButtonName[i]->setVisible(false);
		m_lblKeyDisplay[i] = new QLabel();
		m_lblKeyDisplay[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		m_lblKeyDisplay[i]->setVisible(false);
		m_lblKeyDisplay[i]->setFont(fntMonospace);
		m_btnCfg[i] = new GensCtrlKeyWidget(NULL, m_lblKeyDisplay[i]);
		m_btnCfg[i]->setVisible(false);
		
		m_layout->addWidget(m_lblButtonName[i], i, 0, Qt::AlignLeft);
		m_layout->addWidget(m_lblKeyDisplay[i], i, 1, Qt::AlignLeft);
		m_layout->addWidget(m_btnCfg[i], i, 2, Qt::AlignRight);
	}
	
	// Add a vertical spacer at the bottom of the layout.
	m_vspcCfg = new QSpacerItem(128, 128, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layout->addItem(m_vspcCfg, MAX_CFG_BTNS, 1, 1, 1, Qt::AlignCenter);
	
	// Create the HBox.
	hboxOptions = new QHBoxLayout();
	hboxOptions->setContentsMargins(0, 8, 0, 0); // TODO: Use style default for Top margin.
	m_layout->addLayout(hboxOptions, MAX_CFG_BTNS+1, 0, 1, 3, Qt::AlignCenter);
	
	// Add the "Change All" and "Clear All" buttons.
	// TODO: Icons.
	btnChangeAll = new QPushButton(GensCtrlCfgWidget::tr("&Change All Buttons"), q);
	hboxOptions->addWidget(btnChangeAll);
	
	btnClearAll = new QPushButton(GensCtrlCfgWidget::tr("C&lear All Buttons"), q);
	QObject::connect(btnClearAll, SIGNAL(clicked(bool)),
			 q, SLOT(clearAllButtons()));
	hboxOptions->addWidget(btnClearAll);
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
		case LibGens::IoBase::IOT_NONE: ctrl = new LibGens::IoBase(); break;
		case LibGens::IoBase::IOT_3BTN: ctrl = new LibGens::Io3Button(); break;
		case LibGens::IoBase::IOT_6BTN: ctrl = new LibGens::Io6Button(); break;
		case LibGens::IoBase::IOT_2BTN: ctrl = new LibGens::Io2Button(); break;
		case LibGens::IoBase::IOT_MEGA_MOUSE: ctrl = new LibGens::IoMegaMouse(); break;
		
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
	for (int i = 0, button = 0;
	     i < numButtons && button >= 0; i++)
	{
		LibGens::IoBase::ButtonName_t buttonName = ctrl->buttonName(button);
		sBtnLabel = ButtonName_l(buttonName) + QChar(L':');
		
		m_lblButtonName[i]->setText(sBtnLabel);
		m_lblButtonName[i]->setVisible(true);
		m_lblKeyDisplay[i]->setVisible(true);
		m_btnCfg[i]->setVisible(true);
		
		// Get the next logical button.
		button = ctrl->nextLogicalButton(button);
	}
	
	// Hide other buttons.
	for (int i = numButtons; i < MAX_CFG_BTNS; i++)
	{
		m_lblButtonName[i]->setVisible(false);
		m_lblKeyDisplay[i]->setVisible(false);
		m_btnCfg[i]->setVisible(false);
	}
	
	// Delete the IoBase object
	delete ctrl;
}


/**
 * ButtonName_l(): Get a localized LibGens button name.
 * @param buttonName LibGens button name.
 * @return Localized button name, or empty string on error.
 */
QString GensCtrlCfgWidgetPrivate::ButtonName_l(LibGens::IoBase::ButtonName_t buttonName)
{
	switch (buttonName)
	{
		// Standard controller buttons.
		case LibGens::IoBase::BTNNAME_UP:
			//: Standard controller: D-Pad UP.
			return GensCtrlCfgWidget::tr("Up", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_DOWN:
			//: Standard controller: D-Pad DOWN.
			return GensCtrlCfgWidget::tr("Down", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_LEFT:
			//: Standard controller: D-Pad LEFT.
			return GensCtrlCfgWidget::tr("Left", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_RIGHT:
			//: Standard controller: D-Pad RIGHT.
			return GensCtrlCfgWidget::tr("Right", "controller-standard");
			
		case LibGens::IoBase::BTNNAME_B:
			//: Standard controller: B button.
			return GensCtrlCfgWidget::tr("B", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_C:
			//: Standard controller: C button.
			return GensCtrlCfgWidget::tr("C", "controller-standard");
			
		case LibGens::IoBase::BTNNAME_A:
			//: Standard controller: A button.
			return GensCtrlCfgWidget::tr("A", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_START:
			//: Standard controller: START button.
			return GensCtrlCfgWidget::tr("Start", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_Z:
			//: Standard controller: Z button.
			return GensCtrlCfgWidget::tr("Z", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_Y:
			//: Standard controller: Y button.
			return GensCtrlCfgWidget::tr("Y", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_X:
			//: Standard controller: X button.
			return GensCtrlCfgWidget::tr("X", "controller-standard");
		
		case LibGens::IoBase::BTNNAME_MODE:
			//: Standard controller: MODE button.
			return GensCtrlCfgWidget::tr("Mode", "controller-standard");
		
		// SMS/GG buttons.
		case LibGens::IoBase::BTNNAME_1:
			//: SMS/Game Gear: 1 button.
			return GensCtrlCfgWidget::tr("1", "controller-sms-gg");
		
		case LibGens::IoBase::BTNNAME_2:
			//: SMS/Game Gear: 2 button.
			return GensCtrlCfgWidget::tr("2", "controller-sms-gg");
		
		// Sega Mega Mouse buttons.
		case LibGens::IoBase::BTNNAME_MOUSE_LEFT:
			//: Sega Mega Mouse: LEFT mouse button.
			return GensCtrlCfgWidget::tr("Left", "controller-mouse");
		
		case LibGens::IoBase::BTNNAME_MOUSE_RIGHT:
			//: Sega Mega Mouse: RIGHT mouse button.
			return GensCtrlCfgWidget::tr("Right", "controller-mouse");
		
		case LibGens::IoBase::BTNNAME_MOUSE_MIDDLE:
			//: Sega Mega Mouse: MIDDLE mouse button.
			return GensCtrlCfgWidget::tr("Middle", "controller-mouse");
		
		case LibGens::IoBase::BTNNAME_MOUSE_START:
			//: Sega Mega Mouse: START button.
			return GensCtrlCfgWidget::tr("Start", "controller-mouse");
		
		default:
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * GensCtrlCfgWidgetPrivate::clearAllButtons(): Clear all mapped buttons.
 * WRAPPER SLOT for GensCtrlCfgWidetPrivate.
 */
void GensCtrlCfgWidgetPrivate::clearAllButtons(void)
{
	for (int i = 0; i < MAX_CFG_BTNS; i++)
		m_btnCfg[i]->clearKey();
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


/**
 * GensCtrlCfgWidget::clearAllButtons(): Clear all mapped buttons.
 * WRAPPER SLOT for GensCtrlCfgWidetPrivate.
 */
void GensCtrlCfgWidget::clearAllButtons(void)
{
	d->clearAllButtons();
}

}
