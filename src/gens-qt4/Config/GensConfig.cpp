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
#include "libgens/Vdp/VdpPalette.hpp"

// Version information.
#include "libgens/lg_main.hpp"

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QtCore/QDir>

namespace GensQt4
{

class GensConfigPrivate
{
	public:
		GensConfigPrivate(GensConfig *q);
		
		/** Load/save configuration. **/
		int reload(const QString& filename);
		int save(const QString& filename);
		
		/** Configuration path. **/
		QString cfgPath;
		
		/** System. **/
		int regionCode; // LibGens::SysVersion::RegionCode_t
		uint16_t regionCodeOrder;
		static bool IsRegionCodeOrderValid(uint16_t order);
		
		/** Sega CD Boot ROMs. **/
		QString mcdRomUSA;
		QString mcdRomEUR;
		QString mcdRomJPN;
		QString mcdRomAsia;
		
		/** External programs. **/
		QString extprgUnRAR;
		
		/** Graphics settings. **/
		GensConfig::InterlacedMode_t interlacedMode;
		int contrast;
		int brightness;
		bool grayscale;
		bool inverted;
		int colorScaleMethod;
		
		/** General settings. **/
		bool autoFixChecksum;
		bool autoPause;
		bool borderColor;
		bool ntscV30Rolling;
		
		/** Savestates. **/
		int saveSlot;
		
		/** Key configuration. **/
		GensKeyConfig keyConfig;
		
		/** Emulation options. (Options menu) **/
		bool enableSRam;
	
	private:
		GensConfig *const q;
		Q_DISABLE_COPY(GensConfigPrivate)
};

/********************************
 * GensConfigPrivate functions. *
 ********************************/

GensConfigPrivate::GensConfigPrivate(GensConfig* q)
	: q(q)
{}

/**
 * IsRegionCodeOrderValid(): Check if a region code order is valid.
 * @param order Region code order.
 * @return True if the region code order is valid; false if it isn't.
 */
bool GensConfigPrivate::IsRegionCodeOrderValid(uint16_t order)
{
	static const uint16_t ms_RegionCodeOrder_tbl[24] =
	{
		0x4812, 0x4821, 0x4182, 0x4128, 0x4281, 0x4218, 
		0x8412, 0x8421, 0x8124, 0x8142, 0x8241, 0x8214,
		0x1482, 0x1428, 0x1824, 0x1842, 0x1248, 0x1284,
		0x2481, 0x2418,	0x2814, 0x2841, 0x2148, 0x2184
	};
	
	for (size_t i = 0; i < (sizeof(ms_RegionCodeOrder_tbl)/sizeof(ms_RegionCodeOrder_tbl[0])); i++)
	{
		if (ms_RegionCodeOrder_tbl[i] == order)
			return true;
	}
	
	// Region code order is not valid.
	return false;
}


/**
 * reload(): Load the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfigPrivate::reload(const QString& filename)
{
	QSettings settings(filename, QSettings::IniFormat, q);
	
	// TODO: Check if the file was opened successfully.
	// TODO: QVariant type checking.
	
	/** General settings. **/
	// NOTE: The "General" section is reserved for keys with no section.
	// In order to use it, we shouldn't set a group name.
	//settings.beginGroup(QLatin1String("General"));
	autoFixChecksum = settings.value(QLatin1String("autoFixChecksum"), true).toBool();
	autoPause = settings.value(QLatin1String("autoPause"), false).toBool();
	borderColor = settings.value(QLatin1String("borderColorEmulation"), true).toBool();
	ntscV30Rolling = settings.value(QLatin1String("ntscV30Rolling"), true).toBool();
	//settings.endGroup();
	
	/** System. **/
	settings.beginGroup(QLatin1String("System"));
	
	// NOTE: Uses LibGens::SysVersion::RegionCode_t, but Q_ENUMS requires a QObject for storage.
	regionCode = settings.value(QLatin1String("regionCode"), (int)LibGens::SysVersion::REGION_AUTO).toInt();
	if ((regionCode < (int)LibGens::SysVersion::REGION_AUTO) ||
	    (regionCode > (int)LibGens::SysVersion::REGION_EU_PAL))
	{
		regionCode = (int)LibGens::SysVersion::REGION_AUTO;
	}
	
	uint16_t regionCodeOrder_tmp = settings.value(
			QLatin1String("regionCodeOrder"), QLatin1String("0x4812")).toString().toUShort(NULL, 0);
	if (!IsRegionCodeOrderValid(regionCodeOrder_tmp))
		regionCodeOrder_tmp = 0x4812;
	regionCodeOrder = regionCodeOrder_tmp;
	settings.endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings.beginGroup(QLatin1String("Sega_CD"));
	mcdRomUSA = settings.value(QLatin1String("bootRomUSA"), QString()).toString();
	mcdRomEUR = settings.value(QLatin1String("bootRomEUR"), QString()).toString();
	mcdRomJPN = settings.value(QLatin1String("bootRomJPN"), QString()).toString();
	mcdRomAsia = settings.value(QLatin1String("bootRomAsia"), QString()).toString();
	settings.endGroup();
	
	/** External programs. **/
	// TODO: Don't use setExtPrgUnRAR.
	// Instead, set the filename directly.
#ifdef Q_OS_WIN32
#ifdef __amd64__
	const QLatin1String sExtPrgUnRAR_default("UnRAR64.dll");
#else
	const QLatin1String sExtPrgUnRAR_default("UnRAR.dll");
#endif
#else /* !Q_OS_WIN32 */
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	const QLatin1String sExtPrgUnRAR_default("/usr/bin/unrar");
#endif /* Q_OS_WIN32 */
	settings.beginGroup(QLatin1String("External_Programs"));
	q->setExtPrgUnRAR(settings.value(QLatin1String("UnRAR"), sExtPrgUnRAR_default).toString());
	settings.endGroup();
	
	/** Graphics settings. **/
	settings.beginGroup(QLatin1String("Graphics"));
	// TODO: Add support for INTERLACED_2X.
	int interlaced_tmp = settings.value(QLatin1String("interlacedMode"), (int)GensConfig::INTERLACED_FLICKER).toInt();
	if ((interlaced_tmp < (int)GensConfig::INTERLACED_EVEN) || (interlaced_tmp > (int)GensConfig::INTERLACED_FLICKER))
		interlaced_tmp = (int)GensConfig::INTERLACED_FLICKER;
	interlacedMode = (GensConfig::InterlacedMode_t)interlaced_tmp;
	contrast = settings.value(QLatin1String("contrast"), 0).toInt();
	brightness = settings.value(QLatin1String("brightness"), 0).toInt();
	grayscale = settings.value(QLatin1String("grayscale"), false).toBool();
	inverted = settings.value(QLatin1String("inverted"), false).toBool();
	// using int to prevent Qt issues
	colorScaleMethod = settings.value(QLatin1String("colorScaleMethod"),
				(int)LibGens::VdpPalette::COLSCALE_FULL).toInt();
	if ((colorScaleMethod < 0) || (colorScaleMethod > (int)LibGens::VdpPalette::COLSCALE_FULL_SH))
		colorScaleMethod = (int)LibGens::VdpPalette::COLSCALE_FULL;
	
	settings.endGroup();
	
	/** Savestates. **/
	settings.beginGroup(QLatin1String("Savestates"));
	saveSlot = settings.value(QLatin1String("saveSlot"), 0).toInt();
	settings.endGroup();
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.load(settings);
	settings.endGroup();
	
	/** Controller configuration. **/
	settings.beginGroup(QLatin1String("Controllers"));
	q->m_ctrlConfig->load(settings);
	settings.endGroup();
	
	/** Recent ROMs. **/
	settings.beginGroup(QLatin1String("Recent_ROMs"));
	q->m_recentRoms->load(settings);
	settings.endGroup();
	
	/** Emulation options. (Options menu) **/
	settings.beginGroup(QLatin1String("Options"));
	enableSRam = settings.value(QLatin1String("enableSRam"), true).toBool();
	settings.endGroup();
	
	// Finished loading settings.
	// NOTE: Caller must call emitAll() for settings to take effect.
	return 0;
}


/**
 * save(): Save the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfigPrivate::save(const QString& filename)
{
	// TODO: Migrate to ConfigItem.
	return 0;
	
	QSettings settings(filename, QSettings::IniFormat, q);
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
	settings.setValue(QLatin1String("autoFixChecksum"), autoFixChecksum);
	settings.setValue(QLatin1String("autoPause"), autoPause);
	settings.setValue(QLatin1String("borderColorEmulation"), borderColor);
	settings.setValue(QLatin1String("ntscV30Rolling"), ntscV30Rolling);
	//settings.endGroup();
	
	/** System. **/
	settings.beginGroup(QLatin1String("System"));
	settings.setValue(QLatin1String("regionCode"), (int)regionCode);
	QString sRegionCodeOrder = QLatin1String("0x") +
			QString::number(regionCodeOrder, 16).toUpper().rightJustified(4, QChar(L'0'));
	settings.setValue(QLatin1String("regionCodeOrder"), sRegionCodeOrder);
	settings.endGroup();
	
	/** Sega CD Boot ROMs. **/
	settings.beginGroup(QLatin1String("Sega_CD"));
	settings.setValue(QLatin1String("bootRomUSA"), mcdRomUSA);
	settings.setValue(QLatin1String("bootRomEUR"), mcdRomEUR);
	settings.setValue(QLatin1String("bootRomJPN"), mcdRomJPN);
	settings.setValue(QLatin1String("bootRomAsia"), mcdRomAsia);
	settings.endGroup();
	
	/** External programs. **/
	settings.beginGroup(QLatin1String("External_Programs"));
	settings.setValue(QLatin1String("UnRAR"), extprgUnRAR);
	settings.endGroup();
	
	/** Graphics settings. **/
	settings.beginGroup(QLatin1String("Graphics"));
	settings.setValue(QLatin1String("interlacedMode"), (int)interlacedMode);
	settings.setValue(QLatin1String("contrast"), contrast);
	settings.setValue(QLatin1String("brightness"), brightness);
	settings.setValue(QLatin1String("grayscale"), grayscale);
	settings.setValue(QLatin1String("inverted"), inverted);
	// using int to prevent Qt issues
	settings.setValue(QLatin1String("colorScaleMethod"), colorScaleMethod);
	settings.endGroup();
	
	/** Savestates. **/
	settings.beginGroup(QLatin1String("Savestates"));
	settings.setValue(QLatin1String("saveSlot"), saveSlot);
	settings.endGroup();
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.save(settings);
	settings.endGroup();
	
	/** Controller configuration. **/
	settings.beginGroup(QLatin1String("Controllers"));
	q->m_ctrlConfig->save(settings);
	settings.endGroup();
	
	/** Recent ROMs. **/
	settings.beginGroup(QLatin1String("Recent_ROMs"));
	q->m_recentRoms->save(settings);
	settings.endGroup();
	
	/** Emulation options. (Options menu) **/
	settings.beginGroup(QLatin1String("Options"));
	settings.setValue(QLatin1String("enableSRam"), enableSRam);
	settings.endGroup();
	
	// Finished saving settings.
	return 0;
}


/*************************
 * GensConfig functions. *
 *************************/

GensConfig::GensConfig(QObject *parent)
	: QObject(parent)
	, d(new GensConfigPrivate(this))
	, m_ctrlConfig(new CtrlConfig(this))
	, m_recentRoms(new RecentRoms(this))
{
	// Determine the configuration path.
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String("gens-gs-ii"),
				QLatin1String("gens-gs-ii"), this);
	
	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	d->cfgPath = settings.fileName();
	int sepChr = d->cfgPath.lastIndexOf(QChar(L'/'));
	if (sepChr >= 0)
		d->cfgPath.remove(sepChr + 1, d->cfgPath.size());
	
	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir dir(d->cfgPath);
	if (!dir.exists())
		dir.mkpath(d->cfgPath);
	
	// Load the user's configuration file.
	reload();
}

GensConfig::~GensConfig()
{
	delete d;
}


/**
 * reload(): Load the user's configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::reload(void)
{
	// TODO: Combine with save().
	const QString cfgFilename = d->cfgPath + QLatin1String("gens-gs-ii.conf");
	return d->reload(cfgFilename);
}

/**
 * reload(): Load the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::reload(const QString& filename)
	{ return d->reload(filename); }


/**
 * save(): Save the user's configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::save(void)
{
	// TODO: Combine with reload().
	const QString cfgFilename = d->cfgPath + QLatin1String("gens-gs-ii.conf");
	return save(cfgFilename);
}


/**
 * save(): Save the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfig::save(const QString& filename)
	{ return d->save(filename); }


/**
 * emitAll(): Emit all configuration settings.
 * Useful when starting the emulator.
 */
void GensConfig::emitAll(void)
{
	/** System. **/
	emit regionCode_changed(d->regionCode);
	emit regionCodeOrder_changed(d->regionCodeOrder);
	
	/** Sega CD Boot ROMs. **/
	emit mcdRomUSA_changed(d->mcdRomUSA);
	emit mcdRomEUR_changed(d->mcdRomEUR);
	emit mcdRomJPN_changed(d->mcdRomJPN);
	emit mcdRomAsia_changed(d->mcdRomAsia);
	
	/** External programs. **/
	emit extprgUnRAR_changed(d->extprgUnRAR);
	
	/** Graphics settings. **/
	// TODO: Optimize palette calculation so it's only done once.
	emit interlacedMode_changed(d->interlacedMode);
	emit contrast_changed(d->contrast);
	emit brightness_changed(d->brightness);
	emit grayscale_changed(d->grayscale);
	emit inverted_changed(d->inverted);
	emit colorScaleMethod_changed(d->colorScaleMethod);
	
	/** General settings. **/
	emit autoFixChecksum_changed(d->autoFixChecksum);
	emit autoPause_changed(d->autoPause);
	emit borderColor_changed(d->borderColor);
	emit ntscV30Rolling_changed(d->ntscV30Rolling);
	
	/** Savestates. **/
	emit saveSlot_changed(d->saveSlot);
	
	/** Emulation options. (Options menu) **/
	emit enableSRam_changed(d->enableSRam);
}


/**
 * cfgPath(): Get the base configuration path.
 * @return Base configuration path.
 */
QString GensConfig::cfgPath(void) const
	{ return d->cfgPath; }

/**
 * userPath(): Get a user configuration path.
 * @param pathID Path ID.
 * @return User configuration path, or empty string on error.
 */
QString GensConfig::userPath(ConfigPath pathID)
{
	if (pathID == GCPATH_CONFIG)
		return d->cfgPath;
	
	// TODO: MDP directories.
	// TODO: Allow users to configure these.
	QString path = d->cfgPath;
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
			return d->cfgPath;
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
			return d->cfgPath;
		}
	}
	
	// Return the directory.
	return path;
}


/**
 * GC_PROPERTY_WRITE(): Property read/write function macro.
 * NOTE: emit *does* work here, since moc doesn't process implementation files.
 */
#define GC_PROPERTY_WRITE(propType, propName, setPropType, setPropName) \
propType GensConfig::propName(void) const \
	{ return d->propName; } \
void GensConfig::set##setPropName(setPropType new##setPropName) \
{ \
	if (d->propName == (new##setPropName)) \
		return; \
	\
	d->propName = (new##setPropName); \
	emit propName##_changed(new##setPropName); \
}


/**
 * GC_PROPERTY_WRITE_RANGE(): Property read/write function macro, with range checking.
 * NOTE: emit *does* work here, since moc doesn't process implementation files.
 */
#define GC_PROPERTY_WRITE_RANGE(propType, propName, setPropType, setPropName, rangeMin, rangeMax) \
propType GensConfig::propName(void) const \
	{ return d->propName; } \
void GensConfig::set##setPropName(setPropType new##setPropName) \
{ \
	if (d->propName == (new##setPropName) || \
	    (new##setPropName < (rangeMin) || \
	     new##setPropName > (rangeMax))) \
	{ \
		return; \
	} \
	\
	d->propName = (new##setPropName); \
	emit propName##_changed(new##setPropName); \
}

/** System. **/
// NOTE: Uses LibGens::SysVersion::RegionCode_t, but Q_ENUMS requires a QObject for storage.
GC_PROPERTY_WRITE_RANGE(int, regionCode,
			int, RegionCode,
			(int)LibGens::SysVersion::REGION_AUTO,
			(int)LibGens::SysVersion::REGION_EU_PAL)

// Region code auto-detection order.
uint16_t GensConfig::regionCodeOrder(void) const
	{ return d->regionCodeOrder; }
void GensConfig::setRegionCodeOrder(uint16_t newRegionCodeOrder)
{
	if (d->regionCodeOrder == newRegionCodeOrder)
		return;
	if (!GensConfigPrivate::IsRegionCodeOrderValid(newRegionCodeOrder))
		return;
	
	d->regionCodeOrder = newRegionCodeOrder;
	emit regionCodeOrder_changed(newRegionCodeOrder);
}


/** Sega CD Boot ROMs. **/
GC_PROPERTY_WRITE(QString, mcdRomUSA, const QString&, McdRomUSA)
GC_PROPERTY_WRITE(QString, mcdRomEUR, const QString&, McdRomEUR)
GC_PROPERTY_WRITE(QString, mcdRomJPN, const QString&, McdRomJPN)
GC_PROPERTY_WRITE(QString, mcdRomAsia, const QString&, McdRomAsia)


/** External programs. **/


QString GensConfig::extprgUnRAR(void) const
	{ return d->extprgUnRAR; }
void GensConfig::setExtPrgUnRAR(const QString& filename)
{
	if (d->extprgUnRAR == filename)
		return;
	
	d->extprgUnRAR = filename;
	emit extprgUnRAR_changed(d->extprgUnRAR);
	
	// TODO: Don't set the DcRar filename here.
	// Set it in a signal handler in gqt4_main.cpp or something.
	// Maybe create GensConfigHandler.cpp?
	// (Reasoning is we might have multiple GensConfig instances,
	//  but only one may be active at any given time.)
	LibGens::DcRar::SetExtPrg(d->extprgUnRAR.toUtf8().constData());
}


/** Graphics settings. **/
// TODO: Add support for INTERLACED_2X.
GC_PROPERTY_WRITE_RANGE(GensConfig::InterlacedMode_t, interlacedMode,
			GensConfig::InterlacedMode_t, InterlacedMode,
			(int)INTERLACED_EVEN, (int)INTERLACED_FLICKER);
GC_PROPERTY_WRITE(int, contrast, int, Contrast)
GC_PROPERTY_WRITE(int, brightness, int, Brightness)
GC_PROPERTY_WRITE(bool, grayscale, bool, Grayscale)
GC_PROPERTY_WRITE(bool, inverted, bool, Inverted)
GC_PROPERTY_WRITE_RANGE(int, colorScaleMethod,
			int, ColorScaleMethod,
			(int)LibGens::VdpPalette::COLSCALE_RAW,
			(int)LibGens::VdpPalette::COLSCALE_FULL_SH)


/** General settings. **/
GC_PROPERTY_WRITE(bool, autoFixChecksum, bool, AutoFixChecksum)
GC_PROPERTY_WRITE(bool, autoPause, bool, AutoPause)
GC_PROPERTY_WRITE(bool, borderColor, bool, BorderColor)
GC_PROPERTY_WRITE(bool, ntscV30Rolling, bool, NtscV30Rolling)


/** Savestates. **/


int GensConfig::saveSlot(void) const
	{ return d->saveSlot; }
void GensConfig::setSaveSlot(int newSaveSlot)
{
	// Allow setting the same save slot for preview functionality.
	if (/*d->saveSlot == newSaveSlot ||*/
	    newSaveSlot < 0 ||
	    newSaveSlot > 9)
	{
		return;
	}
	
	d->saveSlot = newSaveSlot;
	emit saveSlot_changed(newSaveSlot);
}

void GensConfig::setSaveSlot_Prev(void)
	{ setSaveSlot((d->saveSlot + 9) % 10); }
void GensConfig::setSaveSlot_Next(void)
	{ setSaveSlot((d->saveSlot + 1) % 10); }


/** Key configuration. **/
int GensConfig::keyToAction(GensKey_t key)
	{ return d->keyConfig.keyToAction(key); }

GensKey_t GensConfig::actionToKey(int action)
	{ return d->keyConfig.actionToKey(action); }

/** Emulation options. (Options menu) **/
GC_PROPERTY_WRITE(bool, enableSRam, bool, EnableSRam)

}

