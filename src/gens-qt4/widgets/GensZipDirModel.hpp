/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensZipDirModel.hpp: Zip Directory tree model.                          *
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

#ifndef __GENS_QT4_WIDGETS_GENSZIPDIRMODEL_HPP__
#define __GENS_QT4_WIDGETS_GENSZIPDIRMODEL_HPP__

// Qt includes.
#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QString>
#include <QtGui/QIcon>

// GensZipDirItem.
#include "GensZipDirItem.hpp"

namespace GensQt4
{

class GensZipDirModelPrivate;

class GensZipDirModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		GensZipDirModel(QObject *parent = 0);
		virtual ~GensZipDirModel();

	private:
		GensZipDirModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GensZipDirModel)
	private:
		Q_DISABLE_COPY(GensZipDirModel)

	public:
		/** QAbstractItemModel functions. **/

		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		virtual QVariant data(const QModelIndex& index, int role) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation,
				int role = Qt::DisplayRole) const override;

		virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

		virtual QModelIndex index(int row, int column,
				const QModelIndex& parent = QModelIndex()) const override;
		virtual QModelIndex parent(const QModelIndex& index) const override;

		virtual bool insertColumns(int position, int columns,
				const QModelIndex& parent = QModelIndex()) override;
		virtual bool removeColumns(int position, int columns,
				const QModelIndex& parent = QModelIndex()) override;
		virtual bool insertRows(int position, int rows,
				const QModelIndex& parent = QModelIndex()) override;
		virtual bool removeRows(int position, int rows,
				const QModelIndex& parent = QModelIndex()) override;

		virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

		virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

		/** GensZipDirModel functions. **/

		bool clear(void);
		bool insertZEntry(const mdp_z_entry_t *z_entry,
				  const QIcon& icon = QIcon());
		const mdp_z_entry_t *getZEntry(const QModelIndex& index) const;

		bool setDirIconState(const QModelIndex& dirIndex, bool isOpen);
};

}

#endif /* __GENS_QT4_WIDGETS_GENSZIPDIRMODEL_HPP__ */
