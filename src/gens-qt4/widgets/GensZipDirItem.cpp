/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensZipDirItem.cpp: Zip Directory tree item.                            *
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

#include "GensZipDirItem.hpp"

namespace GensQt4
{

GensZipDirItem::GensZipDirItem(GensZipDirItem *parent)
{
	m_parentItem = parent;
	
	// Initialize internal data.
	m_itemData.filesize = 0;
	m_itemData.z_entry = NULL;
}

GensZipDirItem::~GensZipDirItem()
{
	qDeleteAll(m_childItems);
}

int GensZipDirItem::childNumber(void) const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<GensZipDirItem*>(this));
	
	return 0;
}

QVariant GensZipDirItem::data(int column) const
{
	QVariant ret;
	
	switch (column)
	{
		case 0:		return QVariant(m_itemData.disp_filename);
		case 1:		return QVariant(m_itemData.full_filename);
		case 2:		return QVariant(m_itemData.filesize);
		default:	return QVariant();
	}
}

bool GensZipDirItem::insertChildren(int position, int count, int columns)
{
	if (position < 0 || position > m_childItems.size())
		return false;
	
	for (int row = 0; row < count; row++)
	{
		GensZipDirItem *item = new GensZipDirItem(this);
		m_childItems.insert(position, item);
	}
	
	return true;
}

bool GensZipDirItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > m_childItems.size())
		return false;
	
	for (int row = 0; row < count; row++)
		delete m_childItems.takeAt(position);
	
	return true;
}

bool GensZipDirItem::setData(int column, const QVariant& value)
{
	// TODO: Add a size() function to m_itemData().
	if (column < 0 || column >= 3)
		return false;
	
	switch (column)
	{
		case 0:		m_itemData.disp_filename = value.toString(); break;
		case 1:		m_itemData.full_filename = value.toString(); break;
		case 2:		m_itemData.filesize = value.toInt(); break;
		default:	return false;
	}
	
	return true;
}

/**
 * sort(): Sort the data item.
 * @param column Column to sort by.
 * @param order Sort order.
 */
void GensZipDirItem::sort(int column, Qt::SortOrder order)
{
	// Sort the child items.
	// NOTE: We're ignoring the column argument for now.
	// We're always sorting by display filename.
	if (order == Qt::AscendingOrder)
		qSort(m_childItems.begin(), m_childItems.end(), SortFilenameLessThan);
	else
		qSort(m_childItems.begin(), m_childItems.end(), SortFilenameGreaterThan);
	
	// Sort child items of child items.
	GensZipDirItem *item;
	foreach(item, m_childItems)
		item->sort(column, order);
}

}
