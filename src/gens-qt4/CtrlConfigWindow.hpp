/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfigWindow.hpp: Controller Configuration Window.                  *
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

#ifndef __GENS_QT4_CTRLCONFIGWINDOW_HPP__
#define __GENS_QT4_CTRLCONFIGWINDOW_HPP__

#include <QtGui/QMainWindow>
#include "ui_CtrlConfigWindow.h"

// Qt classes.
class QActionGroup;

// LibGens includes.
#include "libgens/IO/IoBase.hpp"

namespace GensQt4
{

class CtrlConfigWindow : public QMainWindow, public Ui::CtrlConfigWindow
{
	Q_OBJECT
	
	public:
		static void ShowSingle(QWidget *parent = NULL);
	
	protected:
		CtrlConfigWindow(QWidget *parent = NULL);
		virtual ~CtrlConfigWindow();
		
		// Constant data.
		static const char *ms_CtrlIconFilenames[LibGens::IoBase::IOT_MAX];
		static const QString GetShortDeviceName(LibGens::IoBase::IoType devType);
		static const QString GetLongDeviceName(LibGens::IoBase::IoType devType);
		
		// Internal controller settings.
		LibGens::IoBase::IoType m_devType[2];
		
		QActionGroup *m_actgrpSelPort;
		
		/**
		 * selectedPort(): Determine which port is selected.
		 * TODO: There has to be some way to optimize this...
		 * @return Selected port index, or -1 if no port is selected.
		 */
		inline int selectedPort(void) const;
		
		void updatePortButton(int port);
		void updatePortSettings(int port);
	
	protected slots:
		void accept(void);
		void reject(void);
		
		void reload(void);
		void apply(void);
		
		/** Widget slots. **/
		void on_actionPort1_toggled(bool checked);
		void on_actionPort2_toggled(bool checked);
	
	private:
		static CtrlConfigWindow *m_CtrlConfigWindow;
};

/**
 * selectedPort(): Determine which port is selected.
 * TODO: There has to be some way to optimize this...
 * @return Selected port index, or -1 if no port is selected.
 */
inline int CtrlConfigWindow::selectedPort(void) const
{
	if (actionPort1->isChecked())
		return 0;
	else if (actionPort2->isChecked())
		return 1;
	
	// No port selected.
	return -1;
}

}

#endif /* __GENS_QT4_ABOUTWINDOW_HPP__ */
