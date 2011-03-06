/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ZipSelectDialog.cpp: Multi-File Archive Selection Dialog.               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

namespace GensQt4
{

/**
 * ZipSelectDialog(): Initialize the Multi-File Archive Selection Dialog.
 */
ZipSelectDialog::ZipSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	setupUi(this);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif
	
	// Disable the "OK" button initially.
	QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
	if (button)
		button->setEnabled(false);
	
	m_dirModel = new GensZipDirModel(this);
	treeView->setModel(m_dirModel);
}


/**
 * changeEvent(): Widget state has changed.
 * @param event State change event.
 */
void ZipSelectDialog::changeEvent(QEvent *event)
{
	if (event->type() != QEvent::LanguageChange)
		return;
	
	// Retranslate the UI.
	retranslateUi(this);
}


/**
 * setFileList(): Set the file list.
 * @param z_entry File list.
 */
void ZipSelectDialog::setFileList(const mdp_z_entry_t* z_entry)
{
	// Clear the tree model first.
	m_dirModel->clear();
	
	// Save the list pointer and clear the selected file pointer.
	m_z_entry_list = z_entry;
	m_z_entry_sel = NULL;
	
	// For now, let's just do a standard list view.
	const mdp_z_entry_t *cur = m_z_entry_list;
	for (; cur != NULL; cur = cur->next)
	{
		if (!cur->filename)
		{
			// No filename. Go to the next file.
			continue;
		}
		
		QString filename = QString::fromUtf8(cur->filename);
		
		// TODO: Set icon based on file extension.
		QIcon icon = this->style()->standardIcon(QStyle::SP_FileIcon);
		m_dirModel->insertZEntry(cur, icon);
	}
	
	// Sort the tree model.
	m_dirModel->sort(0);
}


/**
 * accept(): User accepted the selection.
 */
void ZipSelectDialog::accept(void)
{
	// Get the selected item.
	QModelIndexList indexList = treeView->selectionModel()->selectedIndexes();
	if (indexList.size() != 1)
		return;
	
	if (m_dirModel->hasChildren(indexList[0]))
	{
		// This is a directory.
		// Don't do anything.
		return;
	}
	
	// This is a file.
	// Get the selected z_entry.
	m_z_entry_sel = m_dirModel->getZEntry(indexList[0]);
	
	// Call the base accept() function.
	this->QDialog::accept();
}


/**
 * on_treeView_clicked(): An item in the QTreeView was clicked.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_clicked(const QModelIndex& index)
{
	// If this item is a directory, disable the "OK" button.
	// If this item is a file, enable the "OK" button.
	QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
	if (button)
		button->setEnabled(!m_dirModel->hasChildren(index));
}


/**
 * on_treeView_collapsed(): An item in the QTreeView was collapsed.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_collapsed(const QModelIndex& index)
{
	m_dirModel->setDirIconState(index, false);
}


/**
 * on_treeView_expanded(): An item in the QTreeView was expanded.
 * @param index Item index.
 */
void ZipSelectDialog::on_treeView_expanded(const QModelIndex& index)
{
	m_dirModel->setDirIconState(index, true);
}

}
