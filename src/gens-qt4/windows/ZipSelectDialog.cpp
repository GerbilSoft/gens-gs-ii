/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ZipSelectDialog.cpp: Multi-File Archive Selection Dialog.               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#include "ZipSelectDialog.hpp"

// Qt includes.
#include <QtGui/QPushButton>

// Zip Directory Tree Model.
#include "widgets/GensZipDirModel.hpp"

#include "ui_ZipSelectDialog.h"
namespace GensQt4
{

class ZipSelectDialogPrivate
{
	public:
		ZipSelectDialogPrivate(ZipSelectDialog *q);
		~ZipSelectDialogPrivate();

	private:
		ZipSelectDialog *const q_ptr;
		Q_DECLARE_PUBLIC(ZipSelectDialog)
	private:
		Q_DISABLE_COPY(ZipSelectDialogPrivate)

	public:
		Ui::ZipSelectDialog ui;

		const mdp_z_entry_t *z_entry_list;
		const mdp_z_entry_t *z_entry_sel;

		GensZipDirModel *dirModel;
};

/** ZipSelectDialogPrivate **/

ZipSelectDialogPrivate::ZipSelectDialogPrivate(ZipSelectDialog *q)
	: q_ptr(q)
	, z_entry_list(nullptr)
	, z_entry_sel(nullptr)
	, dirModel(new GensZipDirModel(q))
{ }

ZipSelectDialogPrivate::~ZipSelectDialogPrivate()
{
	delete dirModel;
}

/** ZipSelectDialog **/

/**
 * Initialize the Multi-File Archive Selection Dialog.
 */
ZipSelectDialog::ZipSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
	, d_ptr(new ZipSelectDialogPrivate(this))
{
	Q_D(ZipSelectDialog);
	d->ui.setupUi(this);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Disable the "OK" button initially.
	QPushButton *button = d->ui.buttonBox->button(QDialogButtonBox::Ok);
	if (button)
		button->setEnabled(false);

	// Set the tree model.
	d->ui.treeView->setModel(d->dirModel);
}

ZipSelectDialog::~ZipSelectDialog()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void ZipSelectDialog::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(ZipSelectDialog);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	QDialog::changeEvent(event);
}

/**
 * Set the file list.
 * @param z_entry File list.
 */
void ZipSelectDialog::setFileList(const mdp_z_entry_t* z_entry)
{
	Q_D(ZipSelectDialog);

	// Clear the tree model first.
	d->dirModel->clear();

	// Save the list pointer and clear the selected file pointer.
	d->z_entry_list = z_entry;
	d->z_entry_sel = nullptr;

	// For now, let's just do a standard list view.
	const mdp_z_entry_t *cur = d->z_entry_list;
	for (; cur != nullptr; cur = cur->next) {
		if (!cur->filename) {
			// No filename. Go to the next file.
			continue;
		}

		QString filename = QString::fromUtf8(cur->filename);

		// TODO: Set icon based on file extension.
		QIcon icon = this->style()->standardIcon(QStyle::SP_FileIcon);
		d->dirModel->insertZEntry(cur, icon);
	}

	// Sort the tree model.
	d->dirModel->sort(0);
}

/**
 * Get the selected file.
 * @return Selected file, or nullptr if no file was selected.
 */
const mdp_z_entry_t *ZipSelectDialog::selectedFile(void) const
{
	Q_D(const ZipSelectDialog);
	return d->z_entry_sel;
}

/**
 * User accepted the selection.
 */
void ZipSelectDialog::accept(void)
{
	Q_D(ZipSelectDialog);

	// Get the selected item.
	QModelIndexList indexList = d->ui.treeView->selectionModel()->selectedIndexes();
	if (indexList.size() != 1)
		return;

	if (d->dirModel->hasChildren(indexList[0])) {
		// This is a directory.
		// Don't do anything.
		return;
	}

	// This is a file.
	// Get the selected z_entry.
	d->z_entry_sel = d->dirModel->getZEntry(indexList[0]);

	// Call the base accept() function.
	QDialog::accept();
}

/**
 * An item in the QTreeView was clicked.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_clicked(const QModelIndex& index)
{
	// If this item is a directory, disable the "OK" button.
	// If this item is a file, enable the "OK" button.
	Q_D(ZipSelectDialog);
	QPushButton *button = d->ui.buttonBox->button(QDialogButtonBox::Ok);
	if (button)
		button->setEnabled(!d->dirModel->hasChildren(index));
}

/**
 * An item in the QTreeView was collapsed.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_collapsed(const QModelIndex& index)
{
	Q_D(ZipSelectDialog);
	d->dirModel->setDirIconState(index, false);
}

/**
 * An item in the QTreeView was expanded.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_expanded(const QModelIndex& index)
{
	Q_D(ZipSelectDialog);
	d->dirModel->setDirIconState(index, true);
}

}
