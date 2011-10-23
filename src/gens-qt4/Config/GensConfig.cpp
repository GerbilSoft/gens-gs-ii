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
 * reload(): Load the user's configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int GensConfigPrivate::reload(const QString& filename)
{
	QSettings settings(filename, QSettings::IniFormat, q);
	
	// TODO: Check if the file was opened successfully.
	// TODO: QVariant type checking.
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.load(settings);
	settings.endGroup();
	
	/** Controller configuration. **/
	settings.beginGroup(QLatin1String("Controllers"));
	q->m_ctrlConfig->load(settings);
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
	
	/** Key configuration. **/
	settings.beginGroup(QLatin1String("Shortcut_Keys"));
	keyConfig.save(settings);
	settings.endGroup();
	
	/** Controller configuration. **/
	settings.beginGroup(QLatin1String("Controllers"));
	q->m_ctrlConfig->save(settings);
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


/** Key configuration. **/
int GensConfig::keyToAction(GensKey_t key)
	{ return d->keyConfig.keyToAction(key); }

GensKey_t GensConfig::actionToKey(int action)
	{ return d->keyConfig.actionToKey(action); }

/** Emulation options. (Options menu) **/
GC_PROPERTY_WRITE(bool, enableSRam, bool, EnableSRam)

}

