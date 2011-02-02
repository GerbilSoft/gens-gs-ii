/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensZipDirItem.hpp: Zip Directory tree item.                            *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

// Based on http://doc.trolltech.com/4.7/itemviews-editabletreemodel.html

#ifndef __GENS_QT4_WIDGETS_GENSZIPDIRITEM_HPP__
#define __GENS_QT4_WIDGETS_GENSZIPDIRITEM_HPP__

// C includes.
#include <stdlib.h>

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtGui/QIcon>

// LibGens includes.
// TODO: Use MDP's mdp_z_entry_t instead of LibGens::Decompressor.
#include "libgens/Decompressor/Decompressor.hpp"

namespace GensQt4
{

class GensZipDirItem
{
	public:
		GensZipDirItem(GensZipDirItem *parent = 0);
		~GensZipDirItem();
		
		GensZipDirItem *child(int number)
			{ return m_childItems.value(number); }
		int childCount(void) const
			{ return m_childItems.size(); }
		int childNumber(void) const;
		
		int columnCount(void) const
			{ return 3; }	// Three data columns. (Icon is separate.)
		QVariant data(int column) const;
		
		GensZipDirItem *parent(void)
			{ return m_parentItem; }
		
		bool insertChildren(int position, int count, int columns);
		bool removeChildren(int position, int count);
		bool setData(int column, const QVariant& value);
		
		const QIcon& icon(void) const
			{ return m_itemData.icon; }
		bool setIcon(const QIcon& icon)
		{
			m_itemData.icon = icon;
			return true;
		}
		
		const mdp_z_entry_t *getZEntry(void) const
			{ return m_itemData.z_entry; }
		void setZEntry(const mdp_z_entry_t *z_entry)
			{ m_itemData.z_entry = z_entry; }
	
	protected:
		struct zdata
		{
			QString disp_filename;
			QString full_filename;
			int filesize;
			QIcon icon;
			
			const mdp_z_entry_t *z_entry;
		};
		
		QList<GensZipDirItem*> m_childItems;
		GensZipDirItem *m_parentItem;
		zdata m_itemData;
};

}

#endif /* __GENS_QT4_WIDGETS_GENSZIPDIRITEM_HPP__ */
