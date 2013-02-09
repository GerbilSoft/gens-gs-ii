/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * PathConfig.cpp: Path configuration.                                     *
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

#include "PathConfig.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Qt includes.
#include <QtCore/QVector>
#include <QtCore/QDir>

// Directory case-sensitivity.
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
#define DIR_SENSITIVITY Qt::CaseInsensitive
#else
#define DIR_SENSITIVITY Qt::CaseSensitive
#endif

namespace GensQt4
{

class PathConfigPrivate
{
	public:
		PathConfigPrivate(PathConfig *q);
	
	private:
		PathConfig *const q;
		Q_DISABLE_COPY(PathConfigPrivate);
	
	public:
		/**
		 * Load the configuration paths from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return Number of paths loaded on success; negative on error.
		 */
		int load(const QSettings *qSettings);
		
		/**
		 * Save the configuration paths to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return Number of paths saved on success; negative on error.
		 */
		int save(QSettings *qSettings) const;
		
		/**
		 * Convert an absolute path to relative if it's located within GCPATH_CONFIG.
		 * @param path Path to convert.
		 * @return Relative path. (with leading "./" and trailing slash)
		 */
		QString toRelativePath(QString path) const;
		
		/**
		 * Convert a relative path to absolute (within GCPATH_CONFIG).
		 * @param path Path to convert.
		 * @return Absolute path, with trailing slash.
		 */
		QString toAbsolutePath(QString path) const;
		
		/** Variables. **/
		
		/**
		 * Vector of configuration paths.
		 * This vector has absolute pathnames, with trailing slashes.
		 */
		QVector<QString> configPaths;
		
		/**
		 * Default configuration paths.
		 */
		struct DefConfigPath
		{
			const char *cfg_name;	// Configuration name.
			const char *def_path;	// Default path.
		};
		static const DefConfigPath DefConfigPaths[PathConfig::GCPATH_MAX];
};

/**
 * Default configuration paths.
 */
const PathConfigPrivate::DefConfigPath PathConfigPrivate::DefConfigPaths[PathConfig::GCPATH_MAX] =
{
	{nullptr, nullptr},			// GCPATH_CONFIG
	{"Savestates", "./Savestates/"},	// GCPATH_SAVESTATES
	{"SRAM", "./SRAM/"},			// GCPATH_SRAM
	{"BRAM", "./BRAM/"},			// GCPATH_BRAM
	{"WAV", "./WAV/"},			// GCPATH_WAV
	{"VGM", "./VGM/"},			// GCPATH_VGM
	{"Screenshots", "./Screenshots/"},	// GCPATH_SCREENSHOTS
};

PathConfigPrivate::PathConfigPrivate(PathConfig *q)
	: q(q)
{
	// Initialize the vector of configuration paths.
	configPaths.resize(PathConfig::GCPATH_MAX);
	
	// Determine the configuration path.
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String("gens-gs-ii"),
				QLatin1String("gens-gs-ii"));
	
	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	QString configPath = settings.fileName();
	int sepChr = configPath.lastIndexOf(QChar(L'/'));
	if (sepChr >= 0)
		configPath.remove(sepChr + 1, configPath.size());
	
	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir configDir(configPath);
	if (!configDir.exists())
		configDir.mkpath(configDir.absolutePath());
	
	// Save the main configuration path.
	configPath = configDir.absolutePath();
	if (!configPath.endsWith(QChar(L'/')))
		configPath.append(QChar(L'/'));
	configPaths.replace(PathConfig::GCPATH_CONFIG, configPath);
	emit q->pathChanged(PathConfig::GCPATH_CONFIG, configPath);
	
	// Initialize the other configuration paths.
	// TODO: Add support for plugin configuration paths.
	for (int i = (PathConfig::GCPATH_CONFIG + 1);
	     i < PathConfig::GCPATH_MAX; i++)
	{
		QString otherConfigPath = toAbsolutePath(QLatin1String(DefConfigPaths[i].def_path));
		QDir otherConfigDir(otherConfigPath);
		if (!otherConfigDir.exists())
			otherConfigDir.mkpath(otherConfigDir.absolutePath());
		
		otherConfigPath = otherConfigDir.absolutePath();
		if (!otherConfigPath.endsWith(QChar(L'/')))
			otherConfigPath.append(QChar(L'/'));
		configPaths.replace(i, otherConfigPath);
	}
}


/**
 * Load the configuration paths from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return Number of paths loaded on success; negative on error.
 */
int PathConfigPrivate::load(const QSettings *qSettings)
{
	// Load all paths as relative paths to configPath.
	// NOTE: The config path isn't saved.
	const QString configPath = configPaths.at(PathConfig::GCPATH_CONFIG);
	QDir configDir(configPath);
	
	for (int i = (PathConfig::GCPATH_CONFIG + 1);
	     i < PathConfig::GCPATH_MAX; i++)
	{
		QString relPath = qSettings->value(
			QLatin1String(DefConfigPaths[i].cfg_name),
			QLatin1String(DefConfigPaths[i].def_path)).toString();
		
		QString absPath = toAbsolutePath(relPath);
		configPaths.replace(i, absPath);
		emit q->pathChanged((PathConfig::ConfigPath)i, absPath);
	}
	
	// Paths loaded.
	return (PathConfig::GCPATH_MAX - 2);
}


/**
 * Save the configuration paths to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return Number of paths saved on success; negative on error.
 */
int PathConfigPrivate::save(QSettings *qSettings) const
{
	// Save all paths as relative paths to configPath.
	// NOTE: The config path isn't saved.
	const QString configPath = configPaths.at(PathConfig::GCPATH_CONFIG);
	QDir configDir(configPath);
	
	for (int i = (PathConfig::GCPATH_CONFIG + 1);
	     i < PathConfig::GCPATH_MAX; i++)
	{
		QString relPath = toRelativePath(configPaths.at(i));
		qSettings->setValue(QLatin1String(DefConfigPaths[i].cfg_name), relPath);
	}
	
	// Paths saved.
	return (PathConfig::GCPATH_MAX - 2);
}


/**
 * Convert an absolute path to relative if it's located within GCPATH_CONFIG.
 * @param path Path to convert.
 * @return Relative path. (with leading "./" and trailing slash)
 */
QString PathConfigPrivate::toRelativePath(QString path) const
{
	// Make sure the path is an absolute path.
	QString fullPath = QDir(path).absolutePath();
	
	// Check if GCPATH_CONFIG matches the beginning of this path.
	// NOTE: This will only work if no filename is attached...
	const QString configPath = configPaths.at(PathConfig::GCPATH_CONFIG);
	if (fullPath.startsWith(configPath, DIR_SENSITIVITY))
	{
		// It's a match! Convert to a relative pathname.
		QString relPath = fullPath.mid(configPath.length());
		
		// Make sure the beginning of the path is "./".
		if (relPath.isEmpty())
			relPath = QLatin1String("./");
		else if (relPath.startsWith(QChar(L'/')))
			relPath.prepend(QChar(L'.'));
		else if (!relPath.startsWith(QChar(L'.')))
			relPath.prepend(QLatin1String("./"));
		
		// Make sure the end of the path is "/".
		if (!relPath.endsWith(QChar(L'/')))
			relPath.append(QChar(L'/'));
		
		// Return the path.
		return relPath;
	}
	
	// Not a relative path.
	// Ensure that it has a trailing slash.
	if (!path.endsWith(QChar(L'/')))
		path.append(QChar(L'/'));
	return path;
}

/**
 * Convert a relative path to absolute (within GCPATH_CONFIG).
 * @param path Path to convert.
 * @return Absolute path, with trailing slash.
 */
QString PathConfigPrivate::toAbsolutePath(QString path) const
{
	// Check if the specified path is absolute or relative.
	QDir relDir(path);
	if (relDir.isAbsolute())
	{
		// Absolute path. Return it as-is.
		return path;
	}
	
	// Relative path. Convert it to absolute.
	const QString configPath = configPaths.at(PathConfig::GCPATH_CONFIG);
	QDir configDir(configPath);
	QString newPath = configDir.absoluteFilePath(path);
	QDir newDir(newPath);
	
	newPath = newDir.absolutePath();
	if (!newPath.endsWith(QChar(L'/')))
		newPath.append(QChar(L'/'));
	return newPath;
}


/** PathConfig **/

PathConfig::PathConfig(QObject *parent)
	: QObject(parent)
	, d(new PathConfigPrivate(this))
{ }

PathConfig::~PathConfig()
{
	delete d;
}


/**
 * Load the configuration paths from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return Number of paths loaded on success; negative on error.
 */
int PathConfig::load(const QSettings *qSettings)
	{ return d->load(qSettings); }

/**
 * Save the configuration paths to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return Number of paths saved on success; negative on error.
 */
int PathConfig::save(QSettings *qSettings) const
	{ return d->save(qSettings); }


/**
 * Get the main configuration path. (GCPATH_CONFIG)
 * @return Main configuration path.
 */
QString PathConfig::configPath(void)
	{ return d->configPaths.at(GCPATH_CONFIG); }

/**
 * Get the specified configuration path.
 * @param path Configuration path to get. (Invalid paths act like GCPATH_CONFIG.)
 * @return Configuration path.
 */
QString PathConfig::configPath(ConfigPath path)
{
	assert(path >= GCPATH_CONFIG && path < GCPATH_MAX);
	if (path < GCPATH_CONFIG || path >= GCPATH_MAX)
		path = GCPATH_CONFIG;
	
	return d->configPaths.at(path);
}

}
