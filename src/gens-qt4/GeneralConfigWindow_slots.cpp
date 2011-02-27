/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_slots.cpp: General Configuration Window.            *
 * Slots for generic configuration items, e.g. simple checkboxes.          *
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

#include "GeneralConfigWindow.hpp"

// gqt4_main has gqt4_config.
#include "gqt4_main.hpp"

namespace GensQt4
{

// TODO: GNOME applies settings immediately.
// Change from compile-time option to runtime option.
// NOTE: Check this->isVisible() to prevent issues
// with stray signals from e.g. cboIntroColor while
// the window is being initialized.

#ifndef GCW_APPLY_IMMED
#define GENERIC_OPTION(fn, var) \
do { \
	((void)var); \
	setApplyButtonEnabled(true); \
} while (0)
#else
#define GENERIC_OPTION(fn, var) \
do { \
	if (this->isVisible()) \
		gqt4_config->fn(var); \
} while (0)
#endif

void GeneralConfigWindow::on_chkOsdFpsEnable_toggled(bool checked)
	{ GENERIC_OPTION(setOsdFpsEnabled, checked); }
void GeneralConfigWindow::on_chkOsdMsgEnable_toggled(bool checked)
	{ GENERIC_OPTION(setOsdMsgEnabled, checked); }
void GeneralConfigWindow::on_chkAutoFixChecksum_toggled(bool checked)
	{ GENERIC_OPTION(setAutoFixChecksum, checked); }
void GeneralConfigWindow::on_chkAutoPause_toggled(bool checked)
	{ GENERIC_OPTION(setAutoPause, checked); }
void GeneralConfigWindow::on_chkBorderColor_toggled(bool checked)
	{ GENERIC_OPTION(setBorderColor, checked); }
void GeneralConfigWindow::on_chkPauseTint_toggled(bool checked)
	{ GENERIC_OPTION(setPauseTint, checked); }
void GeneralConfigWindow::on_chkNtscV30Rolling_toggled(bool checked)
	{ GENERIC_OPTION(setNtscV30Rolling, checked); }
void GeneralConfigWindow::on_cboIntroStyle_currentIndexChanged(int index)
	{ GENERIC_OPTION(setIntroStyle, index); }
void GeneralConfigWindow::on_cboIntroColor_currentIndexChanged(int index)
	{ GENERIC_OPTION(setIntroColor, index); }

void GeneralConfigWindow::on_chkAspectRatioConstraint_toggled(bool checked)
	{ GENERIC_OPTION(setAspectRatioConstraint, checked); }
void GeneralConfigWindow::on_chkFastBlur_toggled(bool checked)
	{ GENERIC_OPTION(setFastBlur, checked); }
void GeneralConfigWindow::on_chkBilinearFilter_toggled(bool checked)
	{ GENERIC_OPTION(setBilinearFilter, checked); }
void GeneralConfigWindow::on_cboInterlacedMode_currentIndexChanged(int index)
	{ GENERIC_OPTION(setInterlacedMode, (GensConfig::InterlacedMode_t)index); }

void GeneralConfigWindow::on_hsldContrast_valueChanged(int value)
	{ GENERIC_OPTION(setContrast, value); }
void GeneralConfigWindow::on_hsldBrightness_valueChanged(int value)
	{ GENERIC_OPTION(setBrightness, value); }
void GeneralConfigWindow::on_chkGrayscale_toggled(bool checked)
	{ GENERIC_OPTION(setGrayscale, checked); }
void GeneralConfigWindow::on_chkInverted_toggled(bool checked)
	{ GENERIC_OPTION(setInverted, checked); }
void GeneralConfigWindow::on_cboColorScaleMethod_currentIndexChanged(int index)
	{ GENERIC_OPTION(setColorScaleMethod, index); }

}
