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

// Text translation macro.
#define TR(text) \
	QCoreApplication::translate("ZipSelectDialog", (text), NULL, QCoreApplication::UnicodeUTF8)

namespace GensQt4
{

/**
 * ZipSelectDialog(): Initialize the Multi-File Archive Selection Dialog.
 */
ZipSelectDialog::ZipSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	setupUi(this);
}


/**
 * ~ZipSelectDialog(): Shut down the Multi-File Archive Selection Dialog.
 */
ZipSelectDialog::~ZipSelectDialog()
{
}


/**
 * setFileList(): Set the file list.
 * @param z_entry File list.
 */
void ZipSelectDialog::setFileList(const mdp_z_entry_t* z_entry)
{
	// TODO: Figure out Model/View Controller and implement
	// an mdp_z_entry_t model.
	
	// Clear the tree widget first.
	treeWidget->clear();
	
	// Save the list pointer and clear the selected file pointer.
	m_z_entry_list = z_entry;
	m_z_entry_sel = NULL;
	
	// TODO: Hierarchical file view.
	// For now, let's just do a standard list view.
	const mdp_z_entry_t *cur = m_z_entry_list;
	while (cur != NULL)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem();
		if (!cur->filename)
		{
			// No filename. Go to the next file.
			cur = cur->next;
			continue;
		}
		
		QString filename = QString::fromUtf8(cur->filename);
		item->setText(0, filename);
		
		// TODO: Set icon based on file extension.
		item->setIcon(0, this->style()->standardIcon(QStyle::SP_FileIcon));
		
		// TODO: This is probably a bad idea...
		qulonglong data_ptr = (qulonglong)(cur);
		item->setData(0, Qt::UserRole, QVariant(data_ptr));
		treeWidget->addTopLevelItem(item);
		
		// Go to the next file.
		cur = cur->next;
	}
}


/**
 * accept(): User accepted the selection.
 */
void ZipSelectDialog::accept(void)
{
	// Get the selected item.
	QList<QTreeWidgetItem*> lstItems = treeWidget->selectedItems();
	if (lstItems.isEmpty())
	{
		// No item was selected!
		m_z_entry_sel = NULL;
	}
	else
	{
		// An item was selected.
		QTreeWidgetItem *item = lstItems.at(0);
		
		// TODO: This is probably a bad idea...
		qulonglong data_ptr = item->data(0, Qt::UserRole).toULongLong();
		m_z_entry_sel = (const mdp_z_entry_t*)data_ptr;
	}
	
	// Call the base accept() function.
	this->QDialog::accept();
}

}
