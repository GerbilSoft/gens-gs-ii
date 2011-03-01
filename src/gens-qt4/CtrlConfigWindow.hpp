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

/**
 * Controller Configuration ports.
 * 0: Port 1
 * 1: Port 2
 * 2-5: Ports TP1A-TP1D
 * 6-9: Ports TP2A-TP2D
 * 10-13: Ports 4WPA-4WPD
 */
#define CTRL_CFG_MAX_PORTS 14
#define CTRL_CFG_PORT_TP1A 2
#define CTRL_CFG_PORT_TP2A 6
#define CTRL_CFG_PORT_4WPA 10

// Toolbar separators.
#define CTRL_CFG_TBSEP_TP1 0
#define CTRL_CFG_TBSEP_TP2 1
#define CTRL_CFG_TBSEP_4WP 2
#define CTRL_CFG_TBSEP_MAX 3

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
		static const QString GetPortName(int port);
		
		// Internal controller settings.
		LibGens::IoBase::IoType m_devType[CTRL_CFG_MAX_PORTS];
		
		QActionGroup *m_actgrpSelPort;
		
		// Dropdown device lock.
		// Used when rebuilding cboDevice.
		int cboDevice_lock(void);
		int cboDevice_unlock(void);
		bool isCboDeviceLocked(void) const;
	
	protected slots:
		void accept(void);
		void reject(void);
		
		void reload(void);
		void apply(void);
		
		/** Widget slots. **/
		void on_actionPort1_toggled(bool checked);
		void on_actionPort2_toggled(bool checked);
		
		void on_actionPortTP1A_toggled(bool checked);
		void on_actionPortTP1B_toggled(bool checked);
		void on_actionPortTP1C_toggled(bool checked);
		void on_actionPortTP1D_toggled(bool checked);
		
		void on_actionPortTP2A_toggled(bool checked);
		void on_actionPortTP2B_toggled(bool checked);
		void on_actionPortTP2C_toggled(bool checked);
		void on_actionPortTP2D_toggled(bool checked);
		
		void on_actionPort4WPA_toggled(bool checked);
		void on_actionPort4WPB_toggled(bool checked);
		void on_actionPort4WPC_toggled(bool checked);
		void on_actionPort4WPD_toggled(bool checked);
		
		void on_cboDevice_currentIndexChanged(int index);
	
	private:
		static CtrlConfigWindow *m_CtrlConfigWindow;
		
		// Selected port.
		int m_selPort;
		
		// Toolbar separators.
		QAction *m_tbSep[CTRL_CFG_TBSEP_MAX];
		
		// Update port information.
		void updatePortButton(int port);
		void updatePortSettings(int port);
		
		// Select a port.
		void selectPort(int port);
		void rebuildCboDevice(bool isTP);
		
		// Dropdown device lock.
		// Used when rebuilding cboDevice.
		int m_cboDeviceLockCnt;
};

inline bool CtrlConfigWindow::isCboDeviceLocked(void) const
	{ return (m_cboDeviceLockCnt > 0); }

}

#endif /* __GENS_QT4_ABOUTWINDOW_HPP__ */
