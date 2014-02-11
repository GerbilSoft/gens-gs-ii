/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ZipSelectDialog.hpp: Multi-File Archive Selection Dialog.               *
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

#ifndef __GENS_QT4_WINDOWS_ZIPSELECTDIALOG_HPP__
#define __GENS_QT4_WINDOWS_ZIPSELECTDIALOG_HPP__

#include <QtGui/QDialog>
#include <QtCore/QModelIndex>

// LibGens includes.
// TODO: Use MDP's mdp_z_entry_t instead of LibGens::Decompressor.
#include "libgens/Decompressor/Decompressor.hpp"

namespace GensQt4
{

class ZipSelectDialogPrivate;

class ZipSelectDialog : public QDialog
{
	Q_OBJECT
	
	public:
		ZipSelectDialog(QWidget *parent = nullptr);
		virtual ~ZipSelectDialog();

	private:
		ZipSelectDialogPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(ZipSelectDialog)
	private:
		Q_DISABLE_COPY(ZipSelectDialog)

	public:
		/**
		 * Set the file list.
		 * @param z_entry File list.
		 */
		void setFileList(const mdp_z_entry_t *z_entry);

		/**
		 * Get the selected file.
		 * @return Selected file, or nullptr if no file was selected.
		 */
		const mdp_z_entry_t *selectedFile(void) const;

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) override;

	private slots:
		virtual void accept(void) override;

		// Widget signals.
		void on_treeView_clicked(const QModelIndex& index);
		void on_treeView_collapsed(const QModelIndex& index);
		void on_treeView_expanded(const QModelIndex& index);
};

}

#endif /* __GENS_QT4_WINDOWS_ZIPSELECTDIALOG_HPP__ */
