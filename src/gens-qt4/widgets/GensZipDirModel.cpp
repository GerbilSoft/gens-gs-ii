/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensZipDirModel.cpp: Zip Directory tree model.                          *
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

#include "GensZipDirModel.hpp"

// Text translation macro.
#include <QtCore/QCoreApplication>
#define TR(text) \
	QCoreApplication::translate("GensZipDirModel", (text), NULL, QCoreApplication::UnicodeUTF8)

namespace GensQt4
{

GensZipDirModel::GensZipDirModel(QObject *parent)
		: QAbstractItemModel(parent)
{
	m_rootItem = new GensZipDirItem(NULL);
}

GensZipDirModel::~GensZipDirModel()
{
	delete m_rootItem;
}

int GensZipDirModel::rowCount(const QModelIndex& parent) const
{
	GensZipDirItem *item = getItem(parent);
	return item->childCount();
}

int GensZipDirModel::columnCount(const QModelIndex& parent) const
{
	//return m_rootItem->columnCount();
	
	// Only show the first column. (short filename)
	return 1;
}

QVariant GensZipDirModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	
	GensZipDirItem *item = getItem(index);
	switch (role)
	{
		case Qt::DecorationRole:
			if (index.column() != 0)
				return QVariant();
			return item->icon();
		
		case Qt::DisplayRole:
			return item->data(index.column());
		
		default:
			return QVariant();
	}
}

const mdp_z_entry_t *GensZipDirModel::getZEntry(const QModelIndex& index) const
{
	if (!index.isValid())
		return NULL;
	
	GensZipDirItem *item = getItem(index);
	return item->getZEntry();
}

Qt::ItemFlags GensZipDirModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;
	
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

GensZipDirItem *GensZipDirModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		GensZipDirItem *item = static_cast<GensZipDirItem*>(index.internalPointer());
		if (item)
			return item;
	}
	
	return m_rootItem;
}

QVariant GensZipDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
			case 0:		return TR("Filename");
			case 1:		return TR("Full Filename");
			case 2:		return TR("Filesize");
			default:	return QVariant();
		}
	}
	
	return QVariant();
}

QModelIndex GensZipDirModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();
	
	GensZipDirItem *parentItem = getItem(parent);
	GensZipDirItem *childItem = parentItem->child(row);
	
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex GensZipDirModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();
	
	GensZipDirItem *childItem = getItem(index);
	GensZipDirItem *parentItem = childItem->parent();
	
	if (parentItem == m_rootItem)
		return QModelIndex();
	
	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool GensZipDirModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
	// Columns are fixed.
	return false;
}

bool GensZipDirModel::removeColumns(int position, int columns, const QModelIndex& parent)
{
	// Columns are fixed.
	return false;
}

bool GensZipDirModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	GensZipDirItem *parentItem = getItem(parent);
	bool success;
	
	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, m_rootItem->columnCount());
	endInsertRows();
	
	return success;
}

bool GensZipDirModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	GensZipDirItem *parentItem = getItem(parent);
	bool success;
	
	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();
	
	return success;
}

bool GensZipDirModel::clear(void)
{
	int rows = m_rootItem->childCount();
	if (rows == 0)
		return true;
	bool success;
	
	QModelIndex index = createIndex(0, 0, m_rootItem);
	beginRemoveRows(index, 0, rows - 1);
	success = m_rootItem->removeChildren(0, rows);
	endRemoveRows();
	
	return success;
}

bool GensZipDirModel::insertZEntry(const mdp_z_entry_t *z_entry,
				   const QIcon& icon)
{
	// TODO: Break up the ZFile into its directory components.
	QModelIndex index = createIndex(0, 0, m_rootItem);
	
	// Insert a row at the end.
	int row = rowCount(index);
	beginInsertRows(index, row, row);
	m_rootItem->insertChildren(row, 1, m_rootItem->columnCount());
	endInsertRows();
	
	QString filename = QString::fromUtf8(z_entry->filename);
	
	GensZipDirItem *item = m_rootItem->child(row);
	item->setData(0, QVariant(filename));
	item->setData(1, QVariant(filename));
	item->setData(2, QVariant((int)z_entry->filesize));
	item->setIcon(icon);
	item->setZEntry(z_entry);
	
	// Data has changed.
	emit dataChanged(index, index);
	return true;
}

}
