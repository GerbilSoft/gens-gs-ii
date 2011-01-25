/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow.cpp: General Configuration Window.                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#include "GeneralConfigWindow.hpp"

// Text translation macro.
#define TR(text) \
	QApplication::translate("GeneralConfigWindow", (text), NULL, QApplication::UnicodeUTF8)

// Qt4 includes.
#include <QtGui/QFileDialog>
#include <QtCore/QFile>

namespace GensQt4
{

// Static member initialization.
GeneralConfigWindow *GeneralConfigWindow::m_GeneralConfigWindow = NULL;

/**
 * GeneralConfigWindow(): Initialize the General Configuration window.
 */
GeneralConfigWindow::GeneralConfigWindow(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	// Sega CD: Boot ROM file textboxes.
	QString sMcdBootRom_PlaceholderText = TR("Select a %1 Boot ROM...");
	
	// Sega CD: Initialize Boot ROM textboxes.
	txtMcdRomUSA = new GensLineEdit(this);
	txtMcdRomUSA->setObjectName(QString::fromUtf8("txtMcdRomUSA"));
	txtMcdRomEUR = new GensLineEdit(this);
	txtMcdRomEUR->setObjectName(QString::fromUtf8("txtMcdRomEUR"));
	txtMcdRomJPN = new GensLineEdit(this);
	txtMcdRomJPN->setObjectName(QString::fromUtf8("txtMcdRomJPN"));
	
	// Initialize the Qt4 UI.
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
	// Sega CD: Add the Boot ROM textboxes to the grid layout.
	
	// Sega CD: USA Boot ROM
	txtMcdRomUSA->setPlaceholderText(sMcdBootRom_PlaceholderText.arg("Sega CD (U)"));
	gridMcdRoms->addWidget(txtMcdRomUSA, 0, 1);
	lblMcdRomUSA->setBuddy(txtMcdRomUSA);
	// Sega CD: EUR Boot ROM
	txtMcdRomEUR->setPlaceholderText(sMcdBootRom_PlaceholderText.arg("Mega CD (E)"));
	gridMcdRoms->addWidget(txtMcdRomEUR, 1, 1);
	lblMcdRomEUR->setBuddy(txtMcdRomEUR);
	// Sega CD: JPN Boot ROM
	txtMcdRomJPN->setPlaceholderText(sMcdBootRom_PlaceholderText.arg("Mega CD (J)"));
	gridMcdRoms->addWidget(txtMcdRomJPN, 2, 1);
	lblMcdRomJPN->setBuddy(txtMcdRomJPN);
	
	// Initialize BIOS ROM filenames.
	// TODO: Copy filenames from configuration.
	mcdUpdateRomFileStatus(txtMcdRomUSA);
	mcdUpdateRomFileStatus(txtMcdRomEUR);
	mcdUpdateRomFileStatus(txtMcdRomJPN);
}


/**
 * ~GeneralConfigWindow(): Shut down the Controller Configuration window.
 */
GeneralConfigWindow::~GeneralConfigWindow()
{
	// Clear the m_GeneralConfigWindow pointer.
	m_GeneralConfigWindow = NULL;
}


/**
 * ShowSingle(): Show a single instance of the General Configuration window.
 * @param parent Parent window.
 */
void GeneralConfigWindow::ShowSingle(QWidget *parent)
{
	if (m_GeneralConfigWindow != NULL)
	{
		// General Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(m_GeneralConfigWindow);
	}
	else
	{
		// General Configuration Window is not displayed.
		m_GeneralConfigWindow = new GeneralConfigWindow(parent);
		m_GeneralConfigWindow->show();
	}
}


/**
 * mcdSelectRomFile(): Select a Sega CD Boot ROM file.
 * @param rom_id Sega CD Boot ROM ID.
 * @param txtRomFile ROM file textbox.
 */
void GeneralConfigWindow::mcdSelectRomFile(const QString& rom_id, GensLineEdit *txtRomFile)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"
	
	// Create the dialog title.
	QString title = TR("Select %1 Boot ROM").arg(rom_id);
	
	// TODO: Specify the current Boot ROM filename as the default filename.
	QString filename = QFileDialog::getOpenFileName(this, title,
			"",						// Default filename.
			TR("Sega CD Boot ROM images") +
			" (*.bin *.gen"
#ifdef HAVE_ZLIB
			ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
			LZMA_EXT
#endif /* HAVE_LZMA */
			RAR_EXT
			");;" +
			TR("All Files") + "(*.*)");
	
	if (filename.isEmpty())
		return;
	
	// Set the filename text.
	txtRomFile->setText(filename);
	
	// Update the ROM file status.
	mcdUpdateRomFileStatus(txtRomFile);
}

void GeneralConfigWindow::on_btnMcdRomUSA_clicked(void)
	{ mcdSelectRomFile(TR("Sega CD (U)"), txtMcdRomUSA); }
void GeneralConfigWindow::on_btnMcdRomEUR_clicked(void)
	{ mcdSelectRomFile(TR("Mega CD (E)"), txtMcdRomEUR); }
void GeneralConfigWindow::on_btnMcdRomJPN_clicked(void)
	{ mcdSelectRomFile(TR("Mega CD (J)"), txtMcdRomJPN); }


/**
 * mcdUpdateRomFileStatus(): Sega CD: Update Boot ROM file status.
 * @param txtRomFile ROM file textbox.
 */
void GeneralConfigWindow::mcdUpdateRomFileStatus(GensLineEdit *txtRomFile)
{
	// Check if the file exists.
	QFile file(txtRomFile->text());
	if (!file.exists())
	{
		// File doesn't exist.
		// NOTE: KDE 4's Oxygen theme doesn't have a question icon.
		// SP_MessageBoxQuestion is redirected to SP_MessageBoxInformation on KDE 4.
		// TODO: Set ROM file notes.
		txtRomFile->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		return;
	}
	
	// TODO: Check the filename.
	txtRomFile->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
}


/**
 * mcdDisplayRomFileStatus(): Sega CD: Display Boot ROM file status.
 * @param rom_id Sega CD Boot ROM ID.
 * @param rom_desc ROM file description. (detected by examining the ROM)
 */
void GeneralConfigWindow::mcdDisplayRomFileStatus(const QString& rom_id, const QString &rom_desc)
{
	// Set the ROM description.
	QString sel_rom = TR("Selected ROM: %1");
	lblMcdSelectedRom->setText(sel_rom.arg(rom_id));
}

void GeneralConfigWindow::on_txtMcdRomUSA_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Sega CD (U)"), "TODO"); }
void GeneralConfigWindow::on_txtMcdRomEUR_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Mega CD (E)"), "TODO"); }
void GeneralConfigWindow::on_txtMcdRomJPN_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Mega CD (J)"), "TODO"); }

}
