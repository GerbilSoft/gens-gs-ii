/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigHandler.hpp: General configuration signal handler.                *
 *                                                                         *
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

#include "ConfigHandler.hpp"
#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/Decompressor/DcRar.hpp"

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QVariant>

namespace GensQt4
{

ConfigHandler::ConfigHandler(QObject *parent)
	: QObject(parent)
{
	// Initialize the external programs.
	extprgUnRAR_changed_slot(gqt4_cfg->get(QLatin1String("External_Programs/UnRAR")));
	
	// Connect the notification signals.
	gqt4_cfg->registerChangeNotification(QLatin1String("External_Programs/UnRAR"),
					this, SLOT(extprgUnRAR_changed_slot(QVariant)));

	// Boot ROMs.
	gqt4_cfg->registerChangeNotification(QLatin1String("Genesis/tmssRom"),
					this, SLOT(tmssRomFilename_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Genesis/tmssEnabled"),
					this, SLOT(tmssEnabled_changed_slot(QVariant)));
}

/**
 * UnRAR program filename has changed.
 * @param extprgUnRAR New UnRAR program.
 */
void ConfigHandler::extprgUnRAR_changed_slot(const QVariant &extprgUnRAR)
{
	LibGens::DcRar::SetExtPrg(extprgUnRAR.toString().toUtf8().constData());
}

void ConfigHandler::tmssRomFilename_changed_slot(const QVariant &tmssRomFilename)
{
	LibGens::EmuContext::SetTmssRomFilename(tmssRomFilename.toString().toUtf8().constData());
}

void ConfigHandler::tmssEnabled_changed_slot(const QVariant &tmssEnabled)
{
	LibGens::EmuContext::SetTmssEnabled(tmssEnabled.toBool());
}

/**
 * A configuration path has been changed.
 * @param path Configuration path.
 * @param dir New directory.
 */
void ConfigHandler::pathChanged(PathConfig::ConfigPath path, const QString &dir)
{
	switch (path)
	{
		case PathConfig::GCPATH_SRAM:
			// Update the SRam path in LibGens.
			LibGens::EmuContext::SetPathSRam(dir.toUtf8().constData());
			break;
		
		default:
			break;
	}
}

}
