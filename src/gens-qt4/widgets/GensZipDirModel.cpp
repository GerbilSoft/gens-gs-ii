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

// QApplication::QStyle() is needed to get the directory icon.
#include <QtGui/QApplication>
#include <QtGui/QStyle>


namespace GensQt4
{

class GensZipDirModelPrivate
{
	public:
		GensZipDirModelPrivate(GensZipDirModel *q);
		~GensZipDirModelPrivate();

	private:
		GensZipDirModel *const q_ptr;
		Q_DECLARE_PUBLIC(GensZipDirModel)
	private:
		Q_DISABLE_COPY(GensZipDirModelPrivate)

	public:
		GensZipDirItem *rootItem;
		GensZipDirItem *getItem(const QModelIndex& index) const;
		
		QMap<QString, QPersistentModelIndex> dirMap;
};

/** GensZipDirModelPrivate **/

GensZipDirModelPrivate::GensZipDirModelPrivate(GensZipDirModel *q)
	: q_ptr(q)
	, rootItem(new GensZipDirItem(nullptr))
{ }

GensZipDirModelPrivate::~GensZipDirModelPrivate()
{
	delete rootItem;
}

GensZipDirItem *GensZipDirModelPrivate::getItem(const QModelIndex& index) const
{
	if (index.isValid()) {
		GensZipDirItem *item = static_cast<GensZipDirItem*>(index.internalPointer());
		if (item)
			return item;
	}

	return rootItem;
}

/** GensZipDirModel **/

GensZipDirModel::GensZipDirModel(QObject *parent)
	: QAbstractItemModel(parent)
	, d_ptr(new GensZipDirModelPrivate(this))
{ }

GensZipDirModel::~GensZipDirModel()
{
	delete d_ptr;
}

/** QAbstractItemModel functions. **/

int GensZipDirModel::rowCount(const QModelIndex& parent) const
{
	Q_D(const GensZipDirModel);
	GensZipDirItem *item = d->getItem(parent);
	return item->childCount();
}

int GensZipDirModel::columnCount(const QModelIndex& parent) const
{
	//return m_rootItem->columnCount();

	// Only show the first column. (short filename)
	Q_UNUSED(parent);
	return 1;
}

QVariant GensZipDirModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	Q_D(const GensZipDirModel);
	GensZipDirItem *item = d->getItem(index);
	switch (role) {
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
		return nullptr;

	Q_D(const GensZipDirModel);
	GensZipDirItem *item = d->getItem(index);
	return item->getZEntry();
}

Qt::ItemFlags GensZipDirModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;

	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

QVariant GensZipDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
			case 0:		return tr("Filename");
			case 1:		return tr("Full Filename");
			case 2:		return tr("Filesize");
			default:	return QVariant();
		}
	}

	return QVariant();
}

QModelIndex GensZipDirModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	Q_D(const GensZipDirModel);
	GensZipDirItem *parentItem = d->getItem(parent);
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

	Q_D(const GensZipDirModel);
	GensZipDirItem *childItem = d->getItem(index);
	GensZipDirItem *parentItem = childItem->parent();

	if (parentItem == d->rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool GensZipDirModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
	// Columns are fixed.
	Q_UNUSED(position)
	Q_UNUSED(columns)
	Q_UNUSED(parent)
	return false;
}

bool GensZipDirModel::removeColumns(int position, int columns, const QModelIndex& parent)
{
	// Columns are fixed.
	Q_UNUSED(position)
	Q_UNUSED(columns)
	Q_UNUSED(parent)
	return false;
}

bool GensZipDirModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	Q_D(GensZipDirModel);
	GensZipDirItem *parentItem = d->getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, d->rootItem->columnCount());
	endInsertRows();

	return success;
}

bool GensZipDirModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	Q_D(GensZipDirModel);
	GensZipDirItem *parentItem = d->getItem(parent);
	bool success;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

/**
 * Sort the data model.
 * @param column Column to sort by.
 * @param order Sort order.
 */
void GensZipDirModel::sort(int column, Qt::SortOrder order)
{
	Q_D(GensZipDirModel);
	if (!d->rootItem)
		return;

	emit layoutAboutToBeChanged();
	d->rootItem->sort(column, order);
	emit layoutChanged();
}

/** GensZipDirModel functions. **/

bool GensZipDirModel::clear(void)
{
	Q_D(GensZipDirModel);
	int rows = d->rootItem->childCount();
	if (rows == 0)
		return true;
	bool success;

	QModelIndex index = createIndex(0, 0, d->rootItem);
	beginRemoveRows(index, 0, rows - 1);
	success = d->rootItem->removeChildren(0, rows);
	d->dirMap.clear();
	endRemoveRows();

	return success;
}

bool GensZipDirModel::insertZEntry(const mdp_z_entry_t *z_entry,
				   const QIcon& icon)
{
	Q_D(GensZipDirModel);
	GensZipDirItem *parentItem = d->rootItem;
	QModelIndex itemIndex = createIndex(0, 0, parentItem);
	QString full_filename = QString::fromUtf8(z_entry->filename);
	QString disp_filename;

	// TODO: Use '\\' on Win32?
	QStringList dirList = full_filename.split(QChar(L'/'), QString::SkipEmptyParts);
	if (dirList.size() > 1) {
		// More than one component.
		QString cur_path;
		QMap<QString, QPersistentModelIndex>::iterator dirIter;

		for (int i = 0; i < (dirList.size() - 1); i++) {
			// Get the directory component.
			cur_path += dirList.at(i) + QChar(L'/');
			dirIter = d->dirMap.find(cur_path);
			if (dirIter == d->dirMap.end()) {
				// Directory not found. Add it.

				// Insert a row at the end.
				int row = rowCount(itemIndex);
				beginInsertRows(itemIndex, row, row);
				parentItem->insertChildren(row, 1, parentItem->columnCount());
				endInsertRows();

				// Get the item as the new parent item.
				parentItem = parentItem->child(row);
				parentItem->setData(0, dirList[i]);
				itemIndex = createIndex(0, 0, parentItem);

				// Set the icon.
				parentItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));

				// Add the directory to m_dirMap.
				d->dirMap.insert(cur_path, itemIndex);
			} else {
				// Directory found.
				itemIndex = *dirIter;
				parentItem = d->getItem(itemIndex);
			}
		}
	}

	// Set the display filename.
	disp_filename = dirList.at(dirList.size() - 1);

	// Insert a row at the end.
	int row = rowCount(itemIndex);
	beginInsertRows(itemIndex, row, row);
	parentItem->insertChildren(row, 1, parentItem->columnCount());
	endInsertRows();

	GensZipDirItem *item = parentItem->child(row);
	item->setData(0, disp_filename);
	item->setData(1, full_filename);
	item->setData(2, (int)z_entry->filesize);
	item->setIcon(icon);
	item->setZEntry(z_entry);

	// Data has changed.
	emit dataChanged(itemIndex, itemIndex);
	return true;
}

/**
 * Check if a given item has children.
 * @param parent Parent item.
 * @return True if the item has children.
 */
bool GensZipDirModel::hasChildren(const QModelIndex& parent) const
{
	if (!parent.isValid())
		return false;

	Q_D(const GensZipDirModel);
	GensZipDirItem *item = d->getItem(parent);
	return (item->childCount() > 0);
}

/**
 * Set the icon state for a directory item.
 * @param dirIndex Directory item index.
 * @param isOpen If true, directory is open; otherwise, directory is closed.
 * @return True on success; false on failure.
 */
bool GensZipDirModel::setDirIconState(const QModelIndex& dirIndex, bool isOpen)
{
	if (!dirIndex.isValid())
		return false;

	Q_D(GensZipDirModel);
	GensZipDirItem *item = d->getItem(dirIndex);
	if (!item)
		return false;

	// Make sure this is a directory entry.
	if (item->childCount() <= 0)
		return false;

	// Set the directory icon.
	const QStyle::StandardPixmap pxm = (isOpen
			? QStyle::SP_DirOpenIcon
			: QStyle::SP_DirClosedIcon);
	item->setIcon(QApplication::style()->standardIcon(pxm));
	emit dataChanged(dirIndex, dirIndex);
	return true;
}

}
