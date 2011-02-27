/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensConfig.hpp: Gens configuration.                                     *
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

#include "GensConfig.hpp"

// TODO: Move this to GensConfigHandler.cpp?
#include "libgens/Decompressor/DcRar.hpp"

// Color Scale Method.
#include "libgens/MD/VdpPalette.hpp"

// Version information.
#include "libgens/lg_main.hpp"

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QtCore/QDir>

namespace GensQt4
{

GensConfig::GensConfig()
{
	// Determine the configuration path.
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String("gens-gs-ii"),
				QLatin1String("gens-gs-ii"), this);
	
	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	m_cfgPath = settings.fileName();
	int sepChr = m_cfgPath.lastIndexOf(QChar(L'/'));
	if (sepChr >= 0)
		m_cfgPath.remove(sepChr + 1, m_cfgPath.size());
	
	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir dir(m_cfgPath);
	if (!dir.exists())
		dir.mkpath(m_cfgPath);
	
	// Load the user's configuration file.
	reload();
}

GensConfig::~GensConfig()
{
}


/**
 * reload(): Load the user's configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::reload(void)
{
	// TODO: Combine with save().
	const QString cfgFilename = m_cfgPath + QLatin1String("gens-gs-ii.conf");
	return reload(cfgFilename);
}


/**
 * reload(): Load the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::reload(const QString& filename)
{
	QSettings settings(filename, QSettings::IniFormat, this);
	
	// TODO: Check if the file was opened successfully.
	// TODO: QVariant type checking.
	
	/** General settings. **/
	// NOTE: The "General" section is reserved for keys with no section.
	// In order to use it, we shouldn't set a group name.
	//settings.beginGroup(QLatin1String("General"));
	m_autoFixChecksum = settings.value(QLatin1String("autoFixChecksum"), true).toBool();
	m_autoPause = settings.value(QLatin1String("autoPause"), false).toBool();
	m_borderColor = settings.value(QLatin1String("borderColorEmulation"), true).toBool();
	m_pauseTint = settings.value(QLatin1String("pauseTint"), true).toBool();
	m_ntscV30Rolling = settings.value(QLatin1String("ntscV30Rolling"), true).toBool();
	//settings.endGroup();
	
	/** Onscreen display. **/
	settings.beginGroup(QLatin1String("OSD"));
	m_osdFpsEnabled = settings.value(QLatin1String("fpsEnabled"), true).toBool();
	m_osdFpsColor   = settings.value(QLatin1String("fpsColor"), QColor()).value<QColor>();
	if (!m_osdFpsColor.isValid())
		m_osdFpsColor = QColor(Qt::white);
	m_osdMsgEnabled = settings.value(QLatin1String("msgEnabled"), true).toBool();
	m_osdMsgColor   = settings.value(QLatin1String("msgColor"), QColor()).value<QColor>();
	if (!m_osdMsgColor.isValid())
		m_osdMsgColor = QColor(Qt::white);
	settings.endGroup();
	
	/** Intro effect. **/
	// TODO: Enums.
	settings.beginGroup(QLatin1String("Intro_Effect"));
	m_introStyle = settings.value(QLatin1String("introStyle"), 0).toInt();	// none
	m_introColor = settings.value(QLatin1String("introColor"), 7).toInt();	// white
	settings.endGroup();
	
	/** System. **/
	settings.beginGroup(QLatin1String("System"));
	int regionCode_tmp = settings.value(QLatin1String("regionCode"), (int)CONFREGION_AUTODETECT).toInt();
	if ((regionCode_tmp < (int)CONFREGION_AUTODETECT) || (regionCode_tmp > (int)CONFREGION_EU_PAL))
		regionCode_tmp = (int)CONFREGION_AUTODETECT;
	m_regionCode = (ConfRegionCode_t)regionCode_tmp;
	settings.endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings.beginGroup(QLatin1String("Sega_CD"));
	m_mcdRomUSA = settings.value(QLatin1String("bootRomUSA"), QString()).toString();
	m_mcdRomEUR = settings.value(QLatin1String("bootRomEUR"), QString()).toString();
	m_mcdRomJPN = settings.value(QLatin1String("bootRomJPN"), QString()).toString();
	m_mcdRomAsia = settings.value(QLatin1String("bootRomAsia"), QString()).toString();
	settings.endGroup();
	
	/** External programs. **/
	// TODO: Don't use setExtPrgUnRAR.
	// Instead, set the filename directly.
#ifdef Q_OS_WIN32
	const QLatin1String sExtPrgUnRAR_default("UnRAR.dll");
#else /* !Q_OS_WIN32 */
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	const QLatin1String sExtPrgUnRAR_default("/usr/bin/unrar");
#endif /* Q_OS_WIN32 */
	settings.beginGroup(QLatin1String("External_Programs"));
	setExtPrgUnRAR(settings.value(QLatin1String("UnRAR"), sExtPrgUnRAR_default).toString());
	settings.endGroup();
	
	/** Graphics settings. **/
	settings.beginGroup(QLatin1String("Graphics"));
	m_aspectRatioConstraint = settings.value(QLatin1String("aspectRatioConstraint"), true).toBool();
	m_fastBlur = settings.value(QLatin1String("fastBlur"), false).toBool();
	m_bilinearFilter = settings.value(QLatin1String("bilinearFilter"), false).toBool();
	// TODO: Add support for INTERLACED_2X.
	int interlaced_tmp = settings.value(QLatin1String("interlacedMode"), (int)INTERLACED_FLICKER).toInt();
	if ((interlaced_tmp < (int)INTERLACED_EVEN) || (interlaced_tmp > (int)INTERLACED_FLICKER))
		interlaced_tmp = (int)INTERLACED_FLICKER;
	m_interlacedMode = (InterlacedMode_t)interlaced_tmp;
	m_contrast = settings.value(QLatin1String("contrast"), 0).toInt();
	m_brightness = settings.value(QLatin1String("brightness"), 0).toInt();
	m_grayscale = settings.value(QLatin1String("grayscale"), false).toBool();
	m_inverted = settings.value(QLatin1String("inverted"), false).toBool();
	// using int to prevent Qt issues
	m_colorScaleMethod = settings.value(QLatin1String("colorScaleMethod"),
				(int)LibGens::VdpPalette::COLSCALE_FULL).toInt();
	if ((m_colorScaleMethod < 0) || (m_colorScaleMethod > (int)LibGens::VdpPalette::COLSCALE_FULL_HS))
		m_colorScaleMethod = (int)LibGens::VdpPalette::COLSCALE_FULL;
	int stretch_tmp = settings.value(QLatin1String("stretchMode"), (int)STRETCH_H).toInt();
	if ((stretch_tmp < (int)STRETCH_NONE) || (stretch_tmp > (int)STRETCH_FULL))
		stretch_tmp = (int)STRETCH_H;
	m_stretchMode = (StretchMode_t)stretch_tmp;
	
	settings.endGroup();
	
	/** Savestates. **/
	settings.beginGroup(QLatin1String("Savestates"));
	m_saveSlot = settings.value(QLatin1String("saveSlot"), 0).toInt();
	settings.endGroup();
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	m_keyConfig.load(settings);
	settings.endGroup();
	
	// Finished loading settings.
	// NOTE: Caller must call emitAll() for settings to take effect.
	return 0;
}


/**
 * save(): Save the user's configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::save(void)
{
	// TODO: Combine with reload().
	const QString cfgFilename = m_cfgPath + QLatin1String("gens-gs-ii.conf");
	return save(cfgFilename);
}


/**
 * save(): Save the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::save(const QString& filename)
{
	QSettings settings(filename, QSettings::IniFormat, this);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
	
	// TODO: Check if the file was opened successfully.
	
	/** Application information. **/
	// Stored in the "General" section.
	// TODO: Move "General" settings to another section?
	// ("General" is always moved to the top of the file.)
	// TODO: Get the application information from somewhere else.
	// TODO: Use MDP version macros.
	const QString sVersion = QString::fromLatin1("%1.%2.%3")
					.arg((LibGens::version >> 24) & 0xFF)
					.arg((LibGens::version >> 16) & 0xFF)
					.arg(LibGens::version & 0xFFFF);
	
	settings.setValue(QLatin1String("_Application"), QLatin1String("Gens/GS II"));
	settings.setValue(QLatin1String("_Version"), sVersion);
	
	if (LibGens::version_desc)
	{
		settings.setValue(QLatin1String("_VersionExt"),
					QString::fromUtf8(LibGens::version_desc));
	}
	else
	{
		settings.remove(QLatin1String("_VersionExt"));
	}
	
	if (LibGens::version_vcs)
	{
		settings.setValue(QLatin1String("_VersionVcs"),
					QString::fromUtf8(LibGens::version_vcs));
	}
	else
	{
		settings.remove(QLatin1String("_VersionVcs"));
	}
	
	/** General settings. **/
	// NOTE: The "General" section is reserved for keys with no section.
	// In order to use it, we shouldn't set a group name.
	//settings.beginGroup(QLatin1String("General"));
	settings.setValue(QLatin1String("autoFixChecksum"), m_autoFixChecksum);
	settings.setValue(QLatin1String("autoPause"), m_autoPause);
	settings.setValue(QLatin1String("borderColorEmulation"), m_borderColor);
	settings.setValue(QLatin1String("pauseTint"), m_pauseTint);
	settings.setValue(QLatin1String("ntscV30Rolling"), m_ntscV30Rolling);
	//settings.endGroup();
	
	/** Onscreen display. **/
	settings.beginGroup(QLatin1String("OSD"));
	settings.setValue(QLatin1String("fpsEnabled"), m_osdFpsEnabled);
	settings.setValue(QLatin1String("fpsColor"),   m_osdFpsColor.name());
	settings.setValue(QLatin1String("msgEnabled"), m_osdMsgEnabled);
	settings.setValue(QLatin1String("msgColor"),   m_osdMsgColor.name());
	settings.endGroup();
	
	/** Intro effect. **/
	// TODO: Enums.
	settings.beginGroup(QLatin1String("Intro_Effect"));
	settings.setValue(QLatin1String("introStyle"), m_introStyle);
	settings.setValue(QLatin1String("introColor"), m_introColor);
	settings.endGroup();
	
	/** System. **/
	settings.beginGroup(QLatin1String("System"));
	settings.setValue(QLatin1String("regionCode"), (int)m_regionCode);
	settings.endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings.beginGroup(QLatin1String("Sega_CD"));
	settings.setValue(QLatin1String("bootRomUSA"), m_mcdRomUSA);
	settings.setValue(QLatin1String("bootRomEUR"), m_mcdRomEUR);
	settings.setValue(QLatin1String("bootRomJPN"), m_mcdRomJPN);
	settings.setValue(QLatin1String("bootRomAsia"), m_mcdRomAsia);
	settings.endGroup();
	
	/** External programs. **/
	settings.beginGroup(QLatin1String("External_Programs"));
	settings.setValue(QLatin1String("UnRAR"), m_extprgUnRAR);
	settings.endGroup();
	
	/** Graphics settings. **/
	settings.beginGroup(QLatin1String("Graphics"));
	settings.setValue(QLatin1String("aspectRatioConstraint"), m_aspectRatioConstraint);
	settings.setValue(QLatin1String("fastBlur"), m_fastBlur);
	settings.setValue(QLatin1String("bilinearFilter"), m_bilinearFilter);
	settings.setValue(QLatin1String("interlacedMode"), (int)m_interlacedMode);
	settings.setValue(QLatin1String("contrast"), m_contrast);
	settings.setValue(QLatin1String("brightness"), m_brightness);
	settings.setValue(QLatin1String("grayscale"), m_grayscale);
	settings.setValue(QLatin1String("inverted"), m_inverted);
	// using int to prevent Qt issues
	settings.setValue(QLatin1String("colorScaleMethod"), m_colorScaleMethod);
	settings.setValue(QLatin1String("stretchMode"), (int)m_stretchMode);
	settings.endGroup();
	
	/** Savestates. **/
	settings.beginGroup(QLatin1String("Savestates"));
	settings.setValue(QLatin1String("saveSlot"), m_saveSlot);
	settings.endGroup();
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	m_keyConfig.save(settings);
	settings.endGroup();
	
	// Finished saving settings.
	return 0;
}


/**
 * emitAll(): Emit all configuration settings.
 * Useful when starting the emulator.
 */
void GensConfig::emitAll(void)
{
	/** Onscreen display. **/
	emit osdFpsEnabled_changed(m_osdFpsEnabled);
	emit osdFpsColor_changed(m_osdFpsColor);
	emit osdMsgEnabled_changed(m_osdMsgEnabled);
	emit osdMsgColor_changed(m_osdMsgColor);
	
	/** Intro effect. **/
	emit introStyle_changed(m_introStyle);
	emit introColor_changed(m_introColor);
	
	/** Sega CD Boot ROMs. **/
	emit mcdRomUSA_changed(m_mcdRomUSA);
	emit mcdRomEUR_changed(m_mcdRomEUR);
	emit mcdRomJPN_changed(m_mcdRomJPN);
	emit mcdRomAsia_changed(m_mcdRomAsia);
	
	/** External programs. **/
	emit extprgUnRAR_changed(m_extprgUnRAR);
	
	/** Graphics settings. **/
	// TODO: Optimize palette calculation so it's only done once.
	emit aspectRatioConstraint_changed(m_aspectRatioConstraint);
	emit fastBlur_changed(m_fastBlur);
	emit bilinearFilter_changed(m_bilinearFilter);
	emit interlacedMode_changed(m_interlacedMode);
	emit contrast_changed(m_contrast);
	emit brightness_changed(m_brightness);
	emit grayscale_changed(m_grayscale);
	emit inverted_changed(m_inverted);
	emit colorScaleMethod_changed(m_colorScaleMethod);
	emit stretchMode_changed(m_stretchMode);
	
	/** General settings. **/
	emit autoFixChecksum_changed(m_autoFixChecksum);
	emit autoPause_changed(m_autoPause);
	emit borderColor_changed(m_borderColor);
	emit pauseTint_changed(m_pauseTint);
	emit ntscV30Rolling_changed(m_ntscV30Rolling);
	
	/** Savestates. **/
	emit saveSlot_changed(m_saveSlot);
}


/**
 * userPath(): Get a user configuration path.
 * @param pathID Path ID.
 * @return User configuration path, or empty string on error.
 */
QString GensConfig::userPath(ConfigPath pathID)
{
	if (pathID == GCPATH_CONFIG)
		return m_cfgPath;
	
	// TODO: MDP directories.
	// TODO: Allow users to configure these.
	QString path = m_cfgPath;
	switch (pathID)
	{
		case GCPATH_SAVESTATES:
			path += QLatin1String("Savestates/");
			break;
		case GCPATH_SRAM:
			path += QLatin1String("SRAM/");
			break;
		case GCPATH_BRAM:
			path += QLatin1String("BRAM/");
			break;
		case GCPATH_WAV:
			path += QLatin1String("WAV/");
			break;
		case GCPATH_VGM:
			path += QLatin1String("VGM/");
			break;
		case GCPATH_SCREENSHOTS:
			path += QLatin1String("Screenshots/");
			break;
		default:
			return m_cfgPath;
	}
	
	// Check if the directory exists.
	QDir dir(path);
	if (!dir.exists())
	{
		// Directory does not exist. Create it.
		dir.mkpath(path);
		if (!dir.cd(path))
		{
			// Could not create the directory.
			// Use the default configuration directory.
			return m_cfgPath;
		}
	}
	
	// Return the directory.
	return path;
}


/**
 * GC_PROPERTY_WRITE(): Property write function macro.
 * NOTE: emit *does* work here, since moc doesn't process implementation files.
 */
#define GC_PROPERTY_WRITE(propName, setPropType, setPropName) \
void GensConfig::set##setPropName(setPropType new##setPropName) \
{ \
	if (m_##propName == (new##setPropName)) \
		return; \
	\
	m_##propName = (new##setPropName); \
	emit propName##_changed(m_##propName); \
}


/**
 * GC_PROPERTY_WRITE_RANGE(): Property write function macro, with range checking.
 * NOTE: emit *does* work here, since moc doesn't process implementation files.
 */
#define GC_PROPERTY_WRITE_RANGE(propName, setPropType, setPropName, rangeMin, rangeMax) \
void GensConfig::set##setPropName(setPropType new##setPropName) \
{ \
	if (m_##propName == (new##setPropName) || \
	    (new##setPropName < (rangeMin) || \
	     new##setPropName > (rangeMax))) \
	{ \
		return; \
	} \
	\
	m_##propName = (new##setPropName); \
	emit propName##_changed(m_##propName); \
}

/** Onscreen display. **/
GC_PROPERTY_WRITE(osdFpsEnabled, bool, OsdFpsEnabled)
GC_PROPERTY_WRITE(osdFpsColor, const QColor&, OsdFpsColor)
GC_PROPERTY_WRITE(osdMsgEnabled, bool, OsdMsgEnabled)
GC_PROPERTY_WRITE(osdMsgColor, const QColor&, OsdMsgColor)

/** Intro effect. **/
GC_PROPERTY_WRITE_RANGE(introStyle, int, IntroStyle, 0, 2)
GC_PROPERTY_WRITE_RANGE(introColor, int, IntroColor, 0, 7)

/** System. **/
GC_PROPERTY_WRITE_RANGE(regionCode, ConfRegionCode_t, RegionCode,
			(int)CONFREGION_AUTODETECT,
		        (int)CONFREGION_EU_PAL)

/** Sega CD Boot ROMs. **/
GC_PROPERTY_WRITE(mcdRomUSA, const QString&, McdRomUSA)
GC_PROPERTY_WRITE(mcdRomEUR, const QString&, McdRomEUR)
GC_PROPERTY_WRITE(mcdRomJPN, const QString&, McdRomJPN)
GC_PROPERTY_WRITE(mcdRomAsia, const QString&, McdRomAsia)


/** External programs. **/


void GensConfig::setExtPrgUnRAR(const QString& filename)
{
	if (m_extprgUnRAR == filename)
		return;
	
	m_extprgUnRAR = filename;
	emit extprgUnRAR_changed(m_extprgUnRAR);
	
	// TODO: Don't set the DcRar filename here.
	// Set it in a signal handler in gqt4_main.cpp or something.
	// Maybe create GensConfigHandler.cpp?
	// (Reasoning is we might have multiple GensConfig instances,
	//  but only one may be active at any given time.)
	LibGens::DcRar::SetExtPrg(m_extprgUnRAR.toUtf8().constData());
}


/** Graphics settings. **/
GC_PROPERTY_WRITE(aspectRatioConstraint, bool, AspectRatioConstraint)
GC_PROPERTY_WRITE(fastBlur, bool, FastBlur)
GC_PROPERTY_WRITE(bilinearFilter, bool, BilinearFilter)
// TODO: Add support for INTERLACED_2X.
GC_PROPERTY_WRITE_RANGE(interlacedMode, InterlacedMode_t, InterlacedMode,
			(int)INTERLACED_EVEN, (int)INTERLACED_FLICKER);
GC_PROPERTY_WRITE(contrast, int, Contrast)
GC_PROPERTY_WRITE(brightness, int, Brightness)
GC_PROPERTY_WRITE(grayscale, bool, Grayscale)
GC_PROPERTY_WRITE(inverted, bool, Inverted)
GC_PROPERTY_WRITE_RANGE(colorScaleMethod, int, ColorScaleMethod,
			(int)LibGens::VdpPalette::COLSCALE_RAW,
			(int)LibGens::VdpPalette::COLSCALE_FULL_HS)
GC_PROPERTY_WRITE_RANGE(stretchMode, StretchMode_t, StretchMode,
			(int)STRETCH_NONE, (int)STRETCH_FULL)


/** General settings. **/
GC_PROPERTY_WRITE(autoFixChecksum, bool, AutoFixChecksum)
GC_PROPERTY_WRITE(autoPause, bool, AutoPause)
GC_PROPERTY_WRITE(borderColor, bool, BorderColor)
GC_PROPERTY_WRITE(pauseTint, bool, PauseTint)
GC_PROPERTY_WRITE(ntscV30Rolling, bool, NtscV30Rolling)


/** Savestates. **/


void GensConfig::setSaveSlot(int newSaveSlot)
{
	// Allow setting the same save slot for preview functionality.
	if (/*m_saveSlot == newSaveSlot ||*/
	    newSaveSlot < 0 ||
	    newSaveSlot > 9)
	{
		return;
	}
	
	m_saveSlot = newSaveSlot;
	emit saveSlot_changed(m_saveSlot);
}


}

