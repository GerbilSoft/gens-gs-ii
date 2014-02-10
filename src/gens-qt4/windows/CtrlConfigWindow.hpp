/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfigWindow.hpp: Controller Configuration Window.                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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
#include "libgens/IO/IoManager.hpp"

// Toolbar separators.
#define CTRL_CFG_TBSEP_TP1 0
#define CTRL_CFG_TBSEP_TP2 1
#define CTRL_CFG_TBSEP_4WP 2
#define CTRL_CFG_TBSEP_MAX 3

namespace GensQt4
{

class CtrlConfigWindowPrivate;

class CtrlConfigWindow : public QMainWindow, public Ui::CtrlConfigWindow
{
	Q_OBJECT

	public:
		static void ShowSingle(QWidget *parent = nullptr);

	private:
		CtrlConfigWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(CtrlConfigWindow)
	private:
		Q_DISABLE_COPY(CtrlConfigWindow)

	protected:
		CtrlConfigWindow(QWidget *parent = nullptr);
		virtual ~CtrlConfigWindow();

	protected:
		virtual void keyPressEvent(QKeyEvent *event) override;

		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) override;

	protected slots:
		void accept(void);
		void reject(void);

		void reload(void);
		void apply(void);

		/** Widget slots. **/
		void toolbarPortSelected(int virtPort);
		void on_cboDevice_currentIndexChanged(int index);

		/**
		 * A key's configuration has been changed.
		 * @param idx Button index.
		 * @param gensKey New GensKey_t value.
		 */
		void on_ctrlCfgWidget_keyChanged(int idx, GensKey_t gensKey);

	private:
		// Update port information.
		void updatePortButton(LibGens::IoManager::VirtPort_t virtPort);
		void updatePortSettings(LibGens::IoManager::VirtPort_t virtPort);

		// Select a port.
		void selectPort(LibGens::IoManager::VirtPort_t virtPort);
};

}

#endif /* __GENS_QT4_ABOUTWINDOW_HPP__ */
