/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigDefault.hpp: Default configuration.                               *
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

#include "ConfigDefaults.hpp"

namespace GensQt4
{

/**
 * Single instance of ConfigDefaults.
 */
ConfigDefaults *ConfigDefaults::ms_Instance = nullptr;

/**
 * Default configuration filename.
 */
const char *const ConfigDefaults::DefaultConfigFilename = "gens-gs-ii.conf";

/**
 * Default settings.
 */
const ConfigDefaults::DefaultSetting ConfigDefaults::DefaultSettings[] =
{
	/** Super hidden settings! **/
	{"iKnowWhatImDoingAndWillVoidTheWarranty", "false", 0, DefaultSetting::DEF_NO_SAVE, DefaultSetting::VT_BOOL, 0, 0},

	/** General settings. **/
	{"autoFixChecksum",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"autoPause",			"false", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"pauseTint",			"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},

	/** Onscreen display. **/
	{"OSD/fpsEnabled",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"OSD/fpsColor",		"#ffffff", 0, 0,	DefaultSetting::VT_COLOR, 0, 0},
	{"OSD/msgEnabled",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"OSD/msgColor",		"#ffffff", 0, 0,	DefaultSetting::VT_COLOR, 0, 0},

	/** Intro effect. **/
	// TODO: Use enum constants for range.
	{"Intro_Effect/introStyle",	"0", 0, 0,		DefaultSetting::VT_RANGE, 0, 2},	// none
	{"Intro_Effect/introColor",	"7", 0, 0,		DefaultSetting::VT_RANGE, 0, 7},	// white

	/** System. **/
	// TODO: Use enum constants for range.
	{"System/regionCode",		"-1", 0, 0,		DefaultSetting::VT_RANGE, -1, 4},		// LibGens::SysVersion::REGION_AUTO
	{"System/regionCodeOrder",	"0x4812", 4, 0,		DefaultSetting::VT_REGIONCODEORDER, 0, 0},	// US, Europe, Japan, Asia

	/** Genesis-specific settings. **/
	{"Genesis/tmssEnabled",	"false", 0, 0,			DefaultSetting::VT_BOOL, 0, 0},
	{"Genesis/tmssRom",	"", 0, 0,			DefaultSetting::VT_NONE, 0, 0},

	/** Sega CD Boot ROMs. **/
	{"Sega_CD/bootRomUSA", 	"", 0, 0,			DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomEUR",	"", 0, 0,			DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomJPN",	"", 0, 0,			DefaultSetting::VT_NONE, 0, 0},
	{"Sega_CD/bootRomAsia",	"", 0, 0,			DefaultSetting::VT_NONE, 0, 0},
	
	/** External programs. **/
#ifdef Q_OS_WIN32
#ifdef __amd64__
	{"External_Programs/UnRAR", "UnRAR64.dll", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
#else
	{"External_Programs/UnRAR", "UnRAR.dll", 0, 0,		DefaultSetting::VT_NONE, 0, 0},
#endif
#else /* !Q_OS_WIN32 */
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	{"External_Programs/UnRAR", "/usr/bin/unrar", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
#endif /* Q_OS_WIN32 */

	/** Graphics settings. **/
	// TODO: Use enum constants for range.
	{"Graphics/aspectRatioConstraint",	"true", 0, 0,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/fastBlur",			"false", 0, 0,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/bilinearFilter",		"false", 0, 0,	DefaultSetting::VT_BOOL, 0, 0},
	{"Graphics/interlacedMode",		"2", 0, 0,	DefaultSetting::VT_RANGE, 0, 2}, // INTERLACED_FLICKER
	{"Graphics/stretchMode",		"1", 0, 0,	DefaultSetting::VT_RANGE, 0, 3}, // STRETCH_H

	/** VDP settings. **/
	{"VDP/borderColorEmulation",	"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/ntscV30Rolling",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/spriteLimits",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	// The following options should not be changed
	// unless the user knows what they're doing!
	{"VDP/zeroLengthDMA",		"false", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/vscrollBug",		"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/updatePaletteInVBlankOnly", "false", 0, 0,	DefaultSetting::VT_BOOL, 0, 0},
	{"VDP/enableInterlacedMode",	"true", 0, 0,		DefaultSetting::VT_BOOL, 0, 0},

	/** Savestates. **/
	{"Savestates/saveSlot", "0", 0, DefaultSetting::DEF_ALLOW_SAME_VALUE, DefaultSetting::VT_RANGE, 0, 9},

	/** GensWindow configuration. **/
	{"GensWindow/showMenuBar", "true", 0, 0, DefaultSetting::VT_BOOL, 0, 0},

	/** Emulation options. (Options menu) **/
	{"Options/enableSRam", "true", 0, 0, DefaultSetting::VT_BOOL, 0, 0},

	/** End of array. **/
	{nullptr, nullptr, 0, 0, DefaultSetting::VT_NONE, 0, 0}
};

ConfigDefaults::ConfigDefaults()
{
	// Populate the default settings hash.
	defaultSettingsHash.clear();
	for (const DefaultSetting *def = &DefaultSettings[0]; def->key != nullptr; def++)
	{
		const QString key = QLatin1String(def->key);
		defaultSettingsHash.insert(key, def);
	}
}

/**
 * Return a single instance of ConfigDefaults.
 * @return Single instance of ConfigDefaults.
 */
ConfigDefaults *ConfigDefaults::Instance(void)
{
	if (ms_Instance)
		return ms_Instance;
	
	// Initialize ms_Instance.
	ms_Instance = new ConfigDefaults();
	return ms_Instance;
}

}
