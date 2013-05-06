/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromBase.cpp: Find CD-ROM drives: OS-specific base class.          *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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

#include "FindCdromBase.hpp"

namespace GensQt4
{

FindCdromBase::FindCdromBase(QObject *parent)
	: QObject(parent)
{ }

FindCdromBase::~FindCdromBase()
{ }

/**
 * Check if this backend supports OS-specific disc/drive icons.
 * @return True if OS-specific disc/drive icons are supported; false if not.
 */
bool FindCdromBase::isIconSupported(void) const
{
	// By default, backends do not support OS-specific disc/drive icons.
	return false;
}

/**
 * Get the OS-specific disc/drive icon.
 * @param deviceName Device name.
 * @return OS-specific disc/drive icon.
 */
QIcon FindCdromBase::getIcon(QString deviceName) const
{
	// By default, backends do not support OS-specific disc/drive icons.
	return QIcon();
}

}
