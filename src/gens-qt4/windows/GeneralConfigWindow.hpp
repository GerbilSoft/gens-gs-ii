/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow.hpp: General Configuration Window.                  *
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

#ifndef __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_HPP__
#define __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_HPP__

// Qt includes.
#include <QtGui/QMainWindow>

namespace GensQt4
{

class GeneralConfigWindowPrivate;

class GeneralConfigWindow : public QMainWindow
{
	Q_OBJECT

	public:
		static void ShowSingle(QWidget *parent = nullptr);

	protected:
		GeneralConfigWindow(QWidget *parent = nullptr);
		virtual ~GeneralConfigWindow();

	private:
		GeneralConfigWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GeneralConfigWindow);
	private:
		Q_DISABLE_COPY(GeneralConfigWindow);

	protected:
		virtual QSize sizeHint(void) const override
			{ return this->baseSize(); }

		virtual void keyPressEvent(QKeyEvent *event) override;

		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) override;

	protected slots:
		void accept(void);
		void reject(void);
		
		void reload(void);
		void apply(void);

	private slots:
		// Toolbar action group.
		void toolbarTriggered(QAction *action);
	
	private slots:
		/** Onscreen Display **/
		void on_btnOsdFpsColor_clicked(void);
		void on_btnOsdMsgColor_clicked(void);

		/** System. **/
		void on_btnRegionDetectUp_clicked(void);
		void on_btnRegionDetectDown_clicked(void);
		// TODO: Detect changes in lstRegionDetect.

		/** Sega Genesis **/
		void on_chkMDTMSS_toggled(bool checked);
		void on_txtMDTMSSRom_focusIn(void);
		void on_txtMDTMSSRom_textChanged(void);
		void on_btnMDTMSSRom_clicked(void);

		/** Sega CD: Boot ROM **/

		void on_btnMcdRomUSA_clicked(void);
		void on_btnMcdRomEUR_clicked(void);
		void on_btnMcdRomJPN_clicked(void);
		void on_btnMcdRomAsia_clicked(void);

		void on_txtMcdRomUSA_focusIn(void);
		void on_txtMcdRomEUR_focusIn(void);
		void on_txtMcdRomJPN_focusIn(void);
		void on_txtMcdRomAsia_focusIn(void);

		void on_txtMcdRomUSA_textChanged(void);
		void on_txtMcdRomEUR_textChanged(void);
		void on_txtMcdRomJPN_textChanged(void);
		void on_txtMcdRomAsia_textChanged(void);

		/** External Programs **/
		void on_btnExtPrgUnRAR_clicked(void);
		void on_txtExtPrgUnRAR_focusIn(void);
		void on_txtExtPrgUnRAR_textChanged(void);

		/**
		 * Setting change notifications.
		 * On Mac OS X, settings are applied immediately.
		 * On other systems, they aren't.
		 * TODO: GNOME apparently applies settings immediately as well...
		 */

		/** Onscreen display. **/
		void on_chkOsdFpsEnable_toggled(bool checked);
		void on_chkOsdMsgEnable_toggled(bool checked);
		void on_chkAutoFixChecksum_toggled(bool checked);
		void on_chkAutoPause_toggled(bool checked);
		void on_chkPauseTint_toggled(bool checked);
		void on_cboIntroStyle_currentIndexChanged(int index);
		void on_cboIntroColor_currentIndexChanged(int index);

		/** Graphics settings. **/
		void on_chkAspectRatioConstraint_toggled(bool checked);
		void on_chkFastBlur_toggled(bool checked);
		void on_chkBilinearFilter_toggled(bool checked);
		void on_cboInterlacedMode_currentIndexChanged(int index);

		/** Advanced VDP settings. **/
		void on_chkSpriteLimits_toggled(bool checked);
		void on_chkBorderColor_toggled(bool checked);
		void on_chkNtscV30Rolling_toggled(bool checked);
		void on_chkZeroLengthDMA_toggled(bool checked);
		void on_chkVScrollBug_toggled(bool checked);
		void on_chkUpdatePaletteInVBlankOnly_toggled(bool checked);

		/** System. **/
		void on_cboRegionCurrent_currentIndexChanged(int index);
};

}

#endif /* __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_HPP__ */
