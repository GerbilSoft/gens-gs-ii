/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ZipSelectDialog.hpp: Multi-File Archive Selection Dialog.               *
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

#include <config.h>

#ifndef __GENS_QT4_ZIPSELECTDIALOG_HPP__
#define __GENS_QT4_ZIPSELECTDIALOG_HPP__

#include <QtGui/QDialog>
#include "ui_ZipSelectDialog.h"

// LibGens includes.
// TODO: Use MDP's mdp_z_entry_t instead of LibGens::Decompressor.
#include "libgens/Decompressor/Decompressor.hpp"

namespace GensQt4
{

class ZipSelectDialog : public QDialog, public Ui::ZipSelectDialog
{
	Q_OBJECT
	
	public:
		ZipSelectDialog(QWidget *parent = NULL);
		virtual ~ZipSelectDialog();
		
		/**
		 * setFileList(): Set the file list.
		 * @param z_entry File list.
		 */
		void setFileList(const mdp_z_entry_t *z_entry);
		
		/**
		 * selectedFile(): Get the selected file.
		 * @return Selected file, or NULL if no file was selected.
		 */
		const mdp_z_entry_t *selectedFile(void) const { return m_z_entry_sel; }
	
	public slots:
		void accept(void);
	
	protected:
		const mdp_z_entry_t *m_z_entry_list;
		const mdp_z_entry_t *m_z_entry_sel;
};

}

#endif /* __GENS_QT4_ABOUTWINDOW_HPP__ */
