/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow.hpp: General Configuration Window.                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __GENS_QT4_GENERALCONFIGWINDOW_HPP__
#define __GENS_QT4_GENERALCONFIGWINDOW_HPP__

#include "ui_GeneralConfigWindow.h"

// Qt includes.
#include <QtGui/QMainWindow>
#include <QtGui/QColor>

class QAction;
class QActionGroup;

// Apply changes immediately.
#ifdef Q_WS_MAC
#define GCW_APPLY_IMMED
#else /* !Q_WS_MAC */
#ifdef GCW_APPLY_IMMED
#undef GCW_APPLY_IMMED
#endif /* GCW_APPLY_IMMED */
#endif /* Q_WS_MAC */

// GensLineEdit widget.
#include "widgets/GensLineEdit.hpp"

// libgens: Sega CD Boot ROM database.
#include "libgens/Data/mcd_rom_db.h"

namespace GensQt4
{

class GeneralConfigWindow : public QMainWindow, public Ui::GeneralConfigWindow
{
	Q_OBJECT

	public:
		static void ShowSingle(QWidget *parent = nullptr);

	protected:
		GeneralConfigWindow(QWidget *parent = nullptr);
		virtual ~GeneralConfigWindow();

		QSize sizeHint(void) const
			{ return this->baseSize(); }

		void keyPressEvent(QKeyEvent *event);

		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

		// Warning string.
		// NOTE: This must be built at runtime;
		// otherwise, QObject might not be initialized.
		QString m_sWarning;

#ifndef GCW_APPLY_IMMED
		/**
		 * setApplyButtonEnabled(): Enable or disable the Apply button.
		 * @param enabled True to enable; false to disable.
		 */
		void setApplyButtonEnabled(bool enabled);
#endif

#ifdef Q_WS_MAC
		// Mac OS X UI customizations.
		void setupUi_mac(void);
#endif

	protected slots:
		void accept(void);
		void reject(void);
		
		void reload(void);
		void apply(void);

	private:
		// Toolbar action group.
		QActionGroup *m_agBtnGroup;

	private slots:
		// Toolbar action group.
		void toolbarTriggered(QAction *action);

	private:
		static GeneralConfigWindow *m_GeneralConfigWindow;

		/** Onscreen Display **/
		QColor osdSelectColor(QString color_id, const QColor& init_color);

		// Onscreen Display: Colors.
		QColor m_osdFpsColor;
		QColor m_osdMsgColor;

		/** System. **/
		uint16_t regionCodeOrder(void) const;

		/** Sega CD: Boot ROM **/

		// Sega CD: Boot ROM filesize.
		static const int MCD_ROM_FILESIZE = 131072;

		// Sega CD: Select Boot ROM file.
		void mcdSelectRomFile(QString rom_id, GensLineEdit *txtRomFile);

		// Sega CD: Update Boot ROM file status.
		QString mcdUpdateRomFileStatus(GensLineEdit *txtRomFile, int region_code);

		// Sega CD: Display Boot ROM file status.
		void mcdDisplayRomFileStatus(QString rom_id, QString rom_desc);

		// Sega CD: Boot ROM file information.
		QString sMcdRomStatus_USA;
		QString sMcdRomStatus_EUR;
		QString sMcdRomStatus_JPN;
		QString sMcdRomStatus_Asia;

		/** External Programs **/
		QString sExtPrgStatus_UnRAR;
		void extprgDisplayFileStatus(QString file_id, QString file_desc);
	
	private slots:
		/** Onscreen Display **/
		void on_btnOsdFpsColor_clicked(void);
		void on_btnOsdMsgColor_clicked(void);

		/** System. **/
		void on_btnRegionDetectUp_clicked(void);
		void on_btnRegionDetectDown_clicked(void);
		// TODO: Detect changes in lstRegionDetect.

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
		void on_chkVScrollBug_toggled(bool checked);
		void on_chkZeroLengthDMA_toggled(bool checked);

		/** System. **/
		void on_cboRegionCurrent_currentIndexChanged(int index);
};

}

#endif /* __GENS_QT4_GENERALCONFIGWINDOW_HPP__ */
