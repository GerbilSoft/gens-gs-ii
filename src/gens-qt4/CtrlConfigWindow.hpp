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

// Controller Configuration class.
#include "Config/CtrlConfig.hpp"

// Qt includes and classes.
#include <QtGui/QIcon>
#include <QtCore/QSignalMapper>
#include <QtCore/QVector>
class QActionGroup;

// LibGens includes.
#include "libgens/IoManager.hpp"

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
		
		void keyPressEvent(QKeyEvent *event);
		
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);
		
		// Internal controller settings.
		// TODO: Maybe an internal CtrlConfig?
		LibGens::IoManager::IoType_t m_devType[LibGens::IoManager::VIRTPORT_MAX];
		
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
		void toolbarPortSelected(int virtPort);
		void on_cboDevice_currentIndexChanged(int index);
	
	private:
		static CtrlConfigWindow *m_CtrlConfigWindow;
		
		// Controller data.
		static const char *const ms_CtrlIconFilenames[LibGens::IoManager::IOT_MAX];
		static QString GetShortDeviceName(LibGens::IoManager::IoType_t ioType);
		static QString GetLongDeviceName(LibGens::IoManager::IoType_t ioType);
		static QString GetPortName(LibGens::IoManager::VirtPort_t virtPort);
		static QIcon GetCtrlIcon(LibGens::IoManager::IoType_t ioType);
		
		// Selected port.
		LibGens::IoManager::VirtPort_t m_selPort;
		QSignalMapper *m_mapperSelPort;
		
		// Toolbar separators.
		QVector<QAction*> m_vecTbSep;
		
		// Update port information.
		void updatePortButton(LibGens::IoManager::VirtPort_t virtPort);
		void updatePortSettings(LibGens::IoManager::VirtPort_t virtPort);
		
		// Select a port.
		void selectPort(LibGens::IoManager::VirtPort_t virtPort);
		void cboDevice_setTP(bool isTP);
		
		// Dropdown device lock.
		// Used when rebuilding cboDevice.
		int m_cboDeviceLockCnt;
};

inline bool CtrlConfigWindow::isCboDeviceLocked(void) const
	{ return (m_cboDeviceLockCnt > 0); }

}

#endif /* __GENS_QT4_ABOUTWINDOW_HPP__ */
