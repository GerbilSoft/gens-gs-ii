/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_slots.cpp: General Configuration Window.            *
 * Slots for updating configuration settings.                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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
#include "GeneralConfigWindow_p.hpp"

namespace GensQt4 {

/**
 * Apply the configuration changes.
 * Triggered if "Apply" is clicked.
 */
void GeneralConfigWindow::apply(void)
{
	Q_D(GeneralConfigWindow);
	if (d->applySettingsImmediately)
		return;

	/** Onscreen display. **/
	SetValByPath_bool("OSD/fpsEnabled", d->ui.chkOsdFpsEnable->isChecked());
	SetValByPath_QColor("OSD/fpsColor", d->osdFpsColor);
	SetValByPath_bool("OSD/msgEnabled", d->ui.chkOsdMsgEnable->isChecked());
	SetValByPath_QColor("OSD/msgColor", d->osdMsgColor);

	/** Intro effect. **/
	SetValByPath_int("Intro_Effect/introStyle", d->ui.cboIntroStyle->currentIndex());
	SetValByPath_int("Intro_Effect/introColor", d->ui.cboIntroColor->currentIndex());

	/** General settings. **/
	SetValByPath_bool("autoFixChecksum", d->ui.chkAutoFixChecksum->isChecked());
	SetValByPath_bool("autoPause", d->ui.chkAutoPause->isChecked());
	SetValByPath_bool("pauseTint", d->ui.chkPauseTint->isChecked());

	/** Sega Genesis TMSS. **/
	SetValByPath_bool("Genesis/tmssEnabled", d->ui.chkMDTMSS->isChecked());
	SetValByPath_QString("Genesis/tmssRom", d->ui.txtMDTMSSRom->text());

	/** Sega CD Boot ROMs. **/
	SetValByPath_QString("Sega_CD/bootRomUSA", d->ui.txtMcdRomUSA->text());
	SetValByPath_QString("Sega_CD/bootRomEUR", d->ui.txtMcdRomEUR->text());
	SetValByPath_QString("Sega_CD/bootRomJPN", d->ui.txtMcdRomJPN->text());
	SetValByPath_QString("Sega_CD/bootRomAsia", d->ui.txtMcdRomAsia->text());

	/** External programs. **/
	SetValByPath_QString("External_Programs/UnRAR", d->ui.txtExtPrgUnRAR->text());

	/** Graphics settings. **/
	SetValByPath_bool("Graphics/aspectRatioConstraint", d->ui.chkAspectRatioConstraint->isChecked());
	SetValByPath_bool("Graphics/fastBlur", d->ui.chkFastBlur->isChecked());
	SetValByPath_bool("Graphics/bilinearFilter", d->ui.chkBilinearFilter->isChecked());
	SetValByPath_int("Graphics/interlacedMode", d->ui.cboInterlacedMode->currentIndex());

	/** VDP settings. **/
	SetValByPath_bool("VDP/spriteLimits", d->ui.chkSpriteLimits->isChecked());
	SetValByPath_bool("VDP/borderColorEmulation", d->ui.chkBorderColor->isChecked());
	SetValByPath_bool("VDP/ntscV30Rolling", d->ui.chkNtscV30Rolling->isChecked());
	if (d->isWarrantyVoid()) {
		SetValByPath_bool("VDP/zeroLengthDMA", d->ui.chkZeroLengthDMA->isChecked());
		SetValByPath_bool("VDP/vscrollBug", d->ui.chkVScrollBug->isChecked());
		SetValByPath_bool("VDP/updatePaletteInVBlankOnly", d->ui.chkUpdatePaletteInVBlankOnly->isChecked());
		SetValByPath_bool("VDP/enableInterlacedMode", d->ui.chkEnableInterlacedMode->isChecked());
	}

	/** System. **/
	SetValByPath_int("System/regionCode", (d->ui.cboRegionCurrent->currentIndex() - 1));
	SetValByPath_uint("System/regionCodeOrder", d->regionCodeOrder());

	// Disable the Apply button.
	// TODO: If Apply was clicked, set focus back to the main window elements.
	// Otherwise, Cancel will receive focus.
	d->setApplyButtonEnabled(false);
}

// TODO: GNOME applies settings immediately.
// Change from compile-time option to runtime option.
// NOTE: Check this->isVisible() to prevent issues
// with stray signals from e.g. cboIntroColor while
// the window is being initialized.

#define GENERIC_OPTION(path, var) \
do { \
	Q_D(GeneralConfigWindow); \
	if (!d->applySettingsImmediately) { \
		/* Don't apply the setting immediately. */ \
		/* Just enable the "Apply" button. */ \
		d->setApplyButtonEnabled(true); \
	} else { \
		/* Apply the setting immediately. */ \
		if (this->isVisible()) { \
			gqt4_cfg->set(QLatin1String(path), var); \
		} \
	} \
} while (0)

/** Onscreen display. **/
void GeneralConfigWindow::on_chkOsdFpsEnable_toggled(bool checked)
	{ GENERIC_OPTION("OSD/fpsEnabled", checked); }
void GeneralConfigWindow::on_chkOsdMsgEnable_toggled(bool checked)
	{ GENERIC_OPTION("OSD/msgEnabled", checked); }

/** General settings. **/
void GeneralConfigWindow::on_chkAutoFixChecksum_toggled(bool checked)
	{ GENERIC_OPTION("autoFixChecksum", checked); }
void GeneralConfigWindow::on_chkAutoPause_toggled(bool checked)
	{ GENERIC_OPTION("autoPause", checked); }
void GeneralConfigWindow::on_chkPauseTint_toggled(bool checked)
	{ GENERIC_OPTION("pauseTint", checked); }

/** Intro effect. **/
void GeneralConfigWindow::on_cboIntroStyle_currentIndexChanged(int index)
	{ GENERIC_OPTION("Intro_Effect/introStyle", index); }
void GeneralConfigWindow::on_cboIntroColor_currentIndexChanged(int index)
	{ GENERIC_OPTION("Intro_Effect/introColor", index); }

/** System. **/
void GeneralConfigWindow::on_cboRegionCurrent_currentIndexChanged(int index)
	{ GENERIC_OPTION("System/regionCode", (LibGens::SysVersion::RegionCode_t)(index - 1)); }

/** Graphics settings. **/
void GeneralConfigWindow::on_chkAspectRatioConstraint_toggled(bool checked)
	{ GENERIC_OPTION("Graphics/aspectRatioConstraint", checked); }
void GeneralConfigWindow::on_chkFastBlur_toggled(bool checked)
	{ GENERIC_OPTION("Graphics/fastBlur", checked); }
void GeneralConfigWindow::on_chkBilinearFilter_toggled(bool checked)
	{ GENERIC_OPTION("Graphics/bilinearFilter", checked); }
void GeneralConfigWindow::on_cboInterlacedMode_currentIndexChanged(int index)
	{ GENERIC_OPTION("Graphics/interlacedMode", index); }

/** VDP settings. **/
void GeneralConfigWindow::on_chkSpriteLimits_toggled(bool checked)
	{ GENERIC_OPTION("VDP/spriteLimits", checked); }
void GeneralConfigWindow::on_chkBorderColor_toggled(bool checked)
	{ GENERIC_OPTION("VDP/borderColorEmulation", checked); }
void GeneralConfigWindow::on_chkNtscV30Rolling_toggled(bool checked)
	{ GENERIC_OPTION("VDP/ntscV30Rolling", checked); }
void GeneralConfigWindow::on_chkZeroLengthDMA_toggled(bool checked)
	{ GENERIC_OPTION("VDP/zeroLengthDMA", checked); }
void GeneralConfigWindow::on_chkVScrollBug_toggled(bool checked)
	{ GENERIC_OPTION("VDP/vscrollBug", checked); }
void GeneralConfigWindow::on_chkUpdatePaletteInVBlankOnly_toggled(bool checked)
	{ GENERIC_OPTION("VDP/updatePaletteInVBlankOnly", checked); }
void GeneralConfigWindow::on_chkEnableInterlacedMode_toggled(bool checked)
	{ GENERIC_OPTION("VDP/enableInterlacedMode", checked); }

/** Sega Genesis. **/
void GeneralConfigWindow::on_chkMDTMSS_toggled(bool checked)
	{ GENERIC_OPTION("Genesis/tmssEnabled", checked); }

}
