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


namespace GensQt4
{

GensConfig::GensConfig()
{
	// Load the user's configuration file.
	reload();
}

GensConfig::~GensConfig()
{
}


/**
 * reload(): Load the user's configuration file.
 * @param filename Filename. (If empty, the uses default filename.)
 * @return 0 on success; non-zero on error.
 */
int GensConfig::reload(const QString& filename)
{
	QSettings *settings;
	// TODO: Combine filename code with save().
	if (!filename.isEmpty())
	{
		// Filename is specified.
		// Use it instead of the default filename.
		settings = new QSettings(filename, QSettings::IniFormat, this);
	}
	else
	{
		// Filename was not specified.
		// Use the default filename.
		settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
						QLatin1String("gens-gs-ii"),
						QLatin1String("gens-gs-ii"), this);
	}
	settings->setIniCodec(QTextCodec::codecForName("UTF-8"));
	
	// TODO: Check if the file was opened successfully.
	// TODO: QVariant type checking.
	
	/** General settings. **/
	// NOTE: The "General" section is reserved for keys with no section.
	// In order to use it, we shouldn't set a group name.
	//settings->beginGroup(QLatin1String("General"));
	m_autoFixChecksum = settings->value(QLatin1String("autoFixChecksum"), true).toBool();
	m_autoPause = settings->value(QLatin1String("autoPause"), false).toBool();
	m_borderColor = settings->value(QLatin1String("borderColorEmulation"), true).toBool();
	m_pauseTint = settings->value(QLatin1String("pauseTint"), true).toBool();
	m_ntscV30Rolling = settings->value(QLatin1String("ntscV30Rolling"), true).toBool();
	//settings->endGroup();
	
	/** Onscreen display. **/
	settings->beginGroup(QLatin1String("OSD"));
	m_osdFpsEnabled = settings->value(QLatin1String("fpsEnabled"), true).toBool();
	m_osdFpsColor   = settings->value(QLatin1String("fpsColor"), QColor()).value<QColor>();
	if (!m_osdFpsColor.isValid())
		m_osdFpsColor = QColor(Qt::white);
	m_osdMsgEnabled = settings->value(QLatin1String("msgEnabled"), true).toBool();
	m_osdMsgColor   = settings->value(QLatin1String("msgColor"), QColor()).value<QColor>();
	if (!m_osdMsgColor.isValid())
		m_osdMsgColor = QColor(Qt::white);
	settings->endGroup();
	
	/** Intro effect. **/
	// TODO: Enums.
	settings->beginGroup(QLatin1String("IntroEffect"));
	m_introStyle = settings->value(QLatin1String("introStyle"), 0).toInt();	// none
	m_introColor = settings->value(QLatin1String("introColor"), 7).toInt();	// white
	settings->endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings->beginGroup(QLatin1String("Sega_CD"));
	m_mcdRomUSA = settings->value(QLatin1String("bootRomUSA"), QString()).toString();
	m_mcdRomEUR = settings->value(QLatin1String("bootRomEUR"), QString()).toString();
	m_mcdRomJPN = settings->value(QLatin1String("bootRomJPN"), QString()).toString();
	settings->endGroup();
	
	/** External programs. **/
	// TODO: Don't use setExtPrgUnRAR.
	// Instead, set the filename directly.
#ifdef Q_OS_WIN
	const QLatin1String sExtPrgUnRAR_default("UnRAR.dll");
#else
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	const QLatin1String sExtPrgUnRAR_default("/usr/bin/unrar");
#endif
	settings->beginGroup(QLatin1String("External_Programs"));
	setExtPrgUnRAR(settings->value(QLatin1String("UnRAR"), sExtPrgUnRAR_default).toString());
	settings->endGroup();
	
	/** Graphics settings. **/
	settings->beginGroup(QLatin1String("Graphics"));
	m_aspectRatioConstraint = settings->value(QLatin1String("aspectRatioConstraint"), true).toBool();
	m_fastBlur = settings->value(QLatin1String("fastBlur"), false).toBool();
	m_bilinearFilter = settings->value(QLatin1String("bilinearFilter"), false).toBool();
	m_contrast = settings->value(QLatin1String("contrast"), 0).toInt();
	m_brightness = settings->value(QLatin1String("brightness"), 0).toInt();
	m_grayscale = settings->value(QLatin1String("grayscale"), false).toBool();
	m_inverted = settings->value(QLatin1String("inverted"), false).toBool();
	// using int to prevent Qt issues
	m_colorScaleMethod = settings->value(QLatin1String("colorScaleMethod"),
				(int)LibGens::VdpPalette::COLSCALE_FULL).toInt();
	settings->endGroup();
	
	/** Savestates. **/
	settings->beginGroup(QLatin1String("Savestates"));
	m_saveSlot = settings->value(QLatin1String("saveSlot"), 0).toInt();
	settings->endGroup();
	
	// Finished loading settings.
	// NOTE: Caller must call emitAll() for settings to take effect.
	delete settings;
	return 0;
}


/**
 * save(): Save the user's configuration file.
 * @param filename Filename. (If empty, uses the default filename.)
 * @return 0 on success; non-zero on error.
 */
int GensConfig::save(const QString& filename)
{
	QSettings *settings;
	// TODO: Combine filename code with reload().
	if (!filename.isEmpty())
	{
		// Filename is specified.
		// Use it instead of the default filename.
		settings = new QSettings(filename, QSettings::IniFormat, this);
	}
	else
	{
		// Filename was not specified.
		// Use the default filename.
		settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
						QLatin1String("gens-gs-ii"),
						QLatin1String("gens-gs-ii"), this);
	}
	settings->setIniCodec(QTextCodec::codecForName("UTF-8"));
	
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
	
	settings->setValue(QLatin1String("_Application"), QLatin1String("Gens/GS II"));
	settings->setValue(QLatin1String("_Version"), sVersion);
	
	if (LibGens::version_desc)
	{
		settings->setValue(QLatin1String("_VersionExt"),
					QString::fromUtf8(LibGens::version_desc));
	}
	else
	{
		settings->remove(QLatin1String("_VersionExt"));
	}
	
	if (LibGens::version_vcs)
	{
		settings->setValue(QLatin1String("_VersionVcs"),
					QString::fromUtf8(LibGens::version_vcs));
	}
	else
	{
		settings->remove(QLatin1String("_VersionVcs"));
	}
	
	/** General settings. **/
	// NOTE: The "General" section is reserved for keys with no section.
	// In order to use it, we shouldn't set a group name.
	//settings->beginGroup(QLatin1String("General"));
	settings->setValue(QLatin1String("autoFixChecksum"), m_autoFixChecksum);
	settings->setValue(QLatin1String("autoPause"), m_autoPause);
	settings->setValue(QLatin1String("borderColorEmulation"), m_borderColor);
	settings->setValue(QLatin1String("pauseTint"), m_pauseTint);
	settings->setValue(QLatin1String("ntscV30Rolling"), m_ntscV30Rolling);
	//settings->endGroup();
	
	/** Onscreen display. **/
	settings->beginGroup(QLatin1String("OSD"));
	settings->setValue(QLatin1String("fpsEnabled"), m_osdFpsEnabled);
	settings->setValue(QLatin1String("fpsColor"),   m_osdFpsColor.name());
	settings->setValue(QLatin1String("msgEnabled"), m_osdMsgEnabled);
	settings->setValue(QLatin1String("msgColor"),   m_osdMsgColor.name());
	settings->endGroup();
	
	/** Intro effect. **/
	// TODO: Enums.
	settings->beginGroup(QLatin1String("Intro_Effect"));
	settings->setValue(QLatin1String("introStyle"), m_introStyle);
	settings->setValue(QLatin1String("introColor"), m_introColor);
	settings->endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings->beginGroup(QLatin1String("Sega_CD"));
	settings->setValue(QLatin1String("bootRomUSA"), m_mcdRomUSA);
	settings->setValue(QLatin1String("bootRomEUR"), m_mcdRomEUR);
	settings->setValue(QLatin1String("bootRomJPN"), m_mcdRomJPN);
	settings->endGroup();
	
	/** External programs. **/
	settings->beginGroup(QLatin1String("External_Programs"));
	settings->setValue(QLatin1String("UnRAR"), m_extprgUnRAR);
	settings->endGroup();
	
	/** Graphics settings. **/
	settings->beginGroup(QLatin1String("Graphics"));
	settings->setValue(QLatin1String("aspectRatioConstraint"), m_aspectRatioConstraint);
	settings->setValue(QLatin1String("fastBlur"), m_fastBlur);
	settings->setValue(QLatin1String("bilinearFilter"), m_bilinearFilter);
	settings->setValue(QLatin1String("contrast"), m_contrast);
	settings->setValue(QLatin1String("brightness"), m_brightness);
	settings->setValue(QLatin1String("grayscale"), m_grayscale);
	settings->setValue(QLatin1String("inverted"), m_inverted);
	// using int to prevent Qt issues
	settings->setValue(QLatin1String("colorScaleMethod"), m_colorScaleMethod);
	settings->endGroup();
	
	/** Savestates. **/
	settings->beginGroup(QLatin1String("Savestates"));
	settings->setValue(QLatin1String("saveSlot"), m_saveSlot);
	settings->endGroup();
	
	// Finished saving settings.
	delete settings;
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
	
	/** External programs. **/
	emit extprgUnRAR_changed(m_extprgUnRAR);
	
	/** Graphics settings. **/
	// TODO: Optimize palette calculation so it's only done once.
	emit aspectRatioConstraint_changed(m_aspectRatioConstraint);
	emit fastBlur_changed(m_fastBlur);
	emit contrast_changed(m_contrast);
	emit brightness_changed(m_brightness);
	emit grayscale_changed(m_grayscale);
	emit inverted_changed(m_inverted);
	emit colorScaleMethod_changed(m_colorScaleMethod);
	
	/** General settings. **/
	emit autoFixChecksum_changed(m_autoFixChecksum);
	emit autoPause_changed(m_autoPause);
	emit borderColor_changed(m_borderColor);
	emit pauseTint_changed(m_pauseTint);
	emit ntscV30Rolling_changed(m_ntscV30Rolling);
	
	/** Savestates. **/
	emit saveSlot_changed(m_saveSlot);
}


/** Onscreen display. **/


void GensConfig::setOsdFpsEnabled(bool enable)
{
	if (m_osdFpsEnabled == enable)
		return;
	
	m_osdFpsEnabled = enable;
	emit osdFpsEnabled_changed(m_osdFpsEnabled);
}

void GensConfig::setOsdFpsColor(const QColor& color)
{
	if (!color.isValid() || m_osdFpsColor == color)
		return;
	
	m_osdFpsColor = color;
	emit osdFpsColor_changed(m_osdFpsColor);
}

void GensConfig::setOsdMsgEnabled(bool enable)
{
	if (m_osdMsgEnabled == enable)
		return;
	
	m_osdMsgEnabled = enable;
	emit osdMsgEnabled_changed(m_osdMsgEnabled);
}

void GensConfig::setOsdMsgColor(const QColor& color)
{
	if (!color.isValid() || m_osdMsgColor == color)
		return;
	
	m_osdMsgColor = color;
	emit osdMsgColor_changed(m_osdMsgColor);
}


/** Intro effect. **/


void GensConfig::setIntroStyle(int style)
{
	// TODO: Enums.
	if (style < 0 || style > 2)
		return;
	
	m_introStyle = style;
	emit introStyle_changed(m_introStyle);
}


void GensConfig::setIntroColor(int color)
{
	// TODO: Enums.
	if (color < 0 || color > 7)
		return;
	
	m_introColor = color;
	emit introColor_changed(m_introColor);
}


/** Sega CD Boot ROMs. **/


void GensConfig::setMcdRomUSA(const QString& filename)
{
	if (m_mcdRomUSA == filename)
		return;
	
	m_mcdRomUSA = filename;
	emit mcdRomUSA_changed(m_mcdRomUSA);
}

void GensConfig::setMcdRomEUR(const QString& filename)
{
	if (m_mcdRomEUR == filename)
		return;
	
	m_mcdRomEUR = filename;
	emit mcdRomEUR_changed(m_mcdRomEUR);
}

void GensConfig::setMcdRomJPN(const QString& filename)
{
	if (m_mcdRomJPN == filename)
		return;
	
	m_mcdRomJPN = filename;
	emit mcdRomJPN_changed(m_mcdRomJPN);
}


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


void GensConfig::setAspectRatioConstraint(bool newAspectRatioConstraint)
{
	if (m_aspectRatioConstraint == newAspectRatioConstraint)
		return;
	
	m_aspectRatioConstraint = newAspectRatioConstraint;
	emit aspectRatioConstraint_changed(m_aspectRatioConstraint);
}

void GensConfig::setFastBlur(bool newFastBlur)
{
	if (m_fastBlur == newFastBlur)
		return;
	
	m_fastBlur = newFastBlur;
	emit fastBlur_changed(m_fastBlur);
}

void GensConfig::setBilinearFilter(bool newBilinearFilter)
{
	if (m_bilinearFilter == newBilinearFilter)
		return;
	
	m_bilinearFilter = newBilinearFilter;
	emit bilinearFilter_changed(m_bilinearFilter);
}

void GensConfig::setContrast(int newContrast)
{
	if (m_contrast == newContrast)
		return;
	
	m_contrast = newContrast;
	emit contrast_changed(m_contrast);
}

void GensConfig::setBrightness(int newBrightness)
{
	if (m_brightness == newBrightness)
		return;
	
	m_brightness = newBrightness;
	emit brightness_changed(m_brightness);
}

void GensConfig::setGrayscale(bool newGrayscale)
{
	if (m_grayscale == newGrayscale)
		return;
	
	m_grayscale = newGrayscale;
	emit grayscale_changed(m_grayscale);
}

void GensConfig::setInverted(bool newInverted)
{
	if (m_inverted == newInverted)
		return;
	
	m_inverted = newInverted;
	emit inverted_changed(m_inverted);
}

void GensConfig::setColorScaleMethod(int newColorScaleMethod)
{
	if (m_colorScaleMethod == newColorScaleMethod)
		return;
	if (newColorScaleMethod < (int)LibGens::VdpPalette::COLSCALE_RAW ||
	    newColorScaleMethod > (int)LibGens::VdpPalette::COLSCALE_FULL_HS)
	{
		// Invalid color scale method.
		return;
	}
	
	m_colorScaleMethod = newColorScaleMethod;
	emit colorScaleMethod_changed(m_colorScaleMethod);
}


/** General settings. **/


void GensConfig::setAutoFixChecksum(bool newAutoFixChecksum)
{
	if (m_autoFixChecksum == newAutoFixChecksum)
		return;
	
	m_autoFixChecksum = newAutoFixChecksum;
	emit autoFixChecksum_changed(m_autoFixChecksum);
}

void GensConfig::setAutoPause(bool newAutoPause)
{
	if (m_autoPause == newAutoPause)
		return;
	
	m_autoPause = newAutoPause;
	emit autoPause_changed(m_autoPause);
}

void GensConfig::setBorderColor(bool newBorderColor)
{
	if (m_borderColor == newBorderColor)
		return;
	
	m_borderColor = newBorderColor;
	emit borderColor_changed(m_borderColor);
}

void GensConfig::setPauseTint(bool newPauseTint)
{
	if (m_pauseTint == newPauseTint)
		return;
	
	m_pauseTint = newPauseTint;
	emit pauseTint_changed(m_pauseTint);
}

void GensConfig::setNtscV30Rolling(bool newNtscV30Rolling)
{
	if (m_ntscV30Rolling == newNtscV30Rolling)
		return;
	
	m_ntscV30Rolling = newNtscV30Rolling;
	emit ntscV30Rolling_changed(m_ntscV30Rolling);
}


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
