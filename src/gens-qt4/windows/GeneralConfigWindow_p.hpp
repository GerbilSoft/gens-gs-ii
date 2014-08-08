/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_p.hpp: General Configuration Window. (PRIVATE CLASS)*
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

#ifndef __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_P_HPP__
#define __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_P_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QColor>
class QActionGroup;
class QLineEdit;

#include "gqt4_main.hpp"

#include "ui_GeneralConfigWindow.h"
namespace GensQt4
{

static inline void SetValByPath_bool(const char *path, bool value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_QColor(const char *path, const QColor& value)
	{ gqt4_cfg->set(QLatin1String(path), value.name()); }
static inline void SetValByPath_int(const char *path, int value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_uint(const char *path, unsigned int value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_QString(const char *path, QString value)
	{ gqt4_cfg->set(QLatin1String(path), value); }

// Gens widgets.
class GensLineEdit;

class GeneralConfigWindow;

class GeneralConfigWindowPrivate
{
	public:
		GeneralConfigWindowPrivate(GeneralConfigWindow *q);

	private:
		GeneralConfigWindow *const q_ptr;
		Q_DECLARE_PUBLIC(GeneralConfigWindow)
	private:
		Q_DISABLE_COPY(GeneralConfigWindowPrivate)

	public:
		// Single window instance.
		static GeneralConfigWindow *ms_GeneralConfigWindow;
		Ui::GeneralConfigWindow ui;

		// Warning string.
		// NOTE: This must be built at runtime;
		// otherwise, QObject might not be initialized.
		QString sWarning;

		// Should settings be applied immediately?
		bool applySettingsImmediately;
		/**
		 * Enable or disable the Apply button.
		 * @param enabled True to enable; false to disable.
		 */
		void setApplyButtonEnabled(bool enabled);

#ifdef Q_WS_MAC
		// Mac OS X UI customizations.
		void setupUi_mac(void);
#endif /* Q_WS_MAC */

		/**
		 * Check if the warranty is void.
		 * If it is, we'll show some super secret settings.
		 * @return True if the warranty is void.
		 */
		static bool isWarrantyVoid(void);

		// Toolbar action group.
		QActionGroup *actgrpToolbar;

		/** Onscreen Display **/
		QColor osdSelectColor(QString color_id, const QColor& init_color);

		// Onscreen Display: Colors.
		QColor osdFpsColor;
		QColor osdMsgColor;

		/** System. **/
		uint16_t regionCodeOrder(void) const;

		/** Sega Genesis **/

		// Select ROM file.
		void selectRomFile(const QString &rom_desc, QLineEdit *txtRomFile);

		// Sega Genesis: Update TMSS ROM file status.
		QString mdUpdateTmssRomFileStatus(GensLineEdit *txtRomFile);

		// Sega Genesis: TMSS ROM file information.
		QString sMDTmssRomStatus;

		/** Sega CD: Boot ROM **/

		// Sega CD: Boot ROM filesize.
		static const int MCD_ROM_FILESIZE = 131072;

		// Sega CD: Update Boot ROM file status.
		QString mcdUpdateRomFileStatus(GensLineEdit *txtRomFile, int region_code);

		// Sega CD: Display Boot ROM file status.
		void mcdDisplayRomFileStatus(const QString &rom_id, const QString &rom_desc);

		// Sega CD: Boot ROM file information.
		QString sMcdRomStatus_USA;
		QString sMcdRomStatus_EUR;
		QString sMcdRomStatus_JPN;
		QString sMcdRomStatus_Asia;

		/**
		 * Update the ROM file status.
		 */
		void updateRomFileStatus(void);

		/** External Programs **/
		QString sExtPrgStatus_UnRAR;
		void extprgDisplayFileStatus(const QString &file_id, const QString &file_desc);
};

}

#endif /* __GENS_QT4_WINDOWS_GENERALCONFIGWINDOW_P_HPP__ */
