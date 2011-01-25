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

// C includes.
#include <stdint.h>

// zlib
#include <zlib.h>

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
	txtMcdRomUSA->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(TR("Sega CD (U)")));
	gridMcdRoms->addWidget(txtMcdRomUSA, 0, 1);
	lblMcdRomUSA->setBuddy(txtMcdRomUSA);
	setTabOrder(lblMcdRomUSA, txtMcdRomUSA);
	setTabOrder(txtMcdRomUSA, btnMcdRomUSA);
	// Sega CD: EUR Boot ROM
	txtMcdRomEUR->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(TR("Mega CD (E)")));
	gridMcdRoms->addWidget(txtMcdRomEUR, 1, 1);
	lblMcdRomEUR->setBuddy(txtMcdRomEUR);
	setTabOrder(txtMcdRomEUR, btnMcdRomEUR);
	// Sega CD: JPN Boot ROM
	txtMcdRomJPN->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(TR("Mega CD (J)")));
	gridMcdRoms->addWidget(txtMcdRomJPN, 2, 1);
	lblMcdRomJPN->setBuddy(txtMcdRomJPN);
	setTabOrder(txtMcdRomJPN, btnMcdRomJPN);
	
	// Initialize BIOS ROM filenames.
	// TODO: Copy filenames from configuration.
	sMcdRomStatus_USA = mcdUpdateRomFileStatus(txtMcdRomUSA, Region_USA);
	sMcdRomStatus_EUR = mcdUpdateRomFileStatus(txtMcdRomEUR, Region_Europe);
	sMcdRomStatus_JPN = mcdUpdateRomFileStatus(txtMcdRomJPN, Region_Japan_NTSC);
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
 * @param rom_id	[in] Sega CD Boot ROM ID.
 * @param txtRomFile	[in] ROM file textbox.
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
	// ROM file status will be updated automatically by
	// the textChanged() signal from QLineEdit.
	txtRomFile->setText(filename);
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
 * @param region_code Expected ROM region code.
 * @return Updated ROM status.
 */
QString GeneralConfigWindow::mcdUpdateRomFileStatus(GensLineEdit *txtRomFile, MCD_RegionCode_t region_code)
{
	// ROM data buffer.
	uint8_t *rom_data = NULL;
	qint64 data_len;
	uint32_t rom_crc32;
	int boot_rom_id;
	MCD_RegionCode_t boot_rom_region_code;
	MCD_RomStatus_t boot_rom_status;
	
	// Check if the file exists.
	const QString& filename = txtRomFile->text();
	QFile file(filename);
	if (!file.exists())
	{
		// File doesn't exist.
		// NOTE: KDE 4's Oxygen theme doesn't have a question icon.
		// SP_MessageBoxQuestion is redirected to SP_MessageBoxInformation on KDE 4.
		// TODO: Set ROM file notes.
		txtRomFile->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		if (filename.isEmpty())
			return TR("No ROM filename specified.");
		else
			return TR("The specified ROM file was not found.");
	}
	
	// Check the ROM file.
	// TODO: Decompressor support.
	QStyle::StandardPixmap filename_icon = QStyle::SP_DialogYesButton;
	QString rom_id = TR("Unknown");
	QString rom_notes;
	QString rom_size_warning;
	
	// Warning string.
	const QString sWarning = "<span style='color: red'><b>" + TR("Warning:") + "</b></span> ";
	
	// Check the ROM filesize.
	// Valid boot ROMs are 128 KB.
	// Smaller ROMs will not work; larger ROMs may work, but warn anyway.
	if (file.size() != MCD_ROM_FILESIZE)
	{
		// Wrong ROM size.
		filename_icon = QStyle::SP_MessageBoxWarning;
		
		rom_size_warning = sWarning + TR("ROM size is incorrect.") + "<br/>\n" +
				   TR("(expected %L1 bytes; found %L2 bytes)").arg(MCD_ROM_FILESIZE).arg(file.size());
		if (file.size() < MCD_ROM_FILESIZE)
		{
			// ROM is too small, so it's guaranteed to not match anything in the database.
			goto rom_identified;
		}
	}
	
	// Open the file.
	if (!file.open(QIODevice::ReadOnly))
	{
		// Error opening the file.
		rom_notes = TR("Error opening file: %1").arg(file.error());
		goto rom_identified;
	}
	
	// Read 128 KB and calculate the CRC32.
	rom_data = (uint8_t*)malloc(MCD_ROM_FILESIZE);
	data_len = file.read((char*)rom_data, MCD_ROM_FILESIZE);
	if (data_len != MCD_ROM_FILESIZE)
	{
		// Error reading data from the file.
		rom_notes = TR("Error reading file.") + "<br/>\n" +
			    TR("(expected %L1 bytes; read %L2 bytes)").arg(MCD_ROM_FILESIZE).arg(data_len);
		goto rom_identified;
	}
	
	// Calculate the CRC32 using zlib.
	rom_crc32 = crc32(0, rom_data, MCD_ROM_FILESIZE);
	
	// Look up the CRC32 in the Sega CD Boot ROM database.
	boot_rom_id = lg_mcd_rom_FindByCRC32(rom_crc32);
	if (boot_rom_id < 0)
	{
		// Boot ROM was not found in the database.
		filename_icon = QStyle::SP_MessageBoxWarning;
		goto rom_identified;
	}
	
	// ROM found. Get the description and other information.
	rom_id = QString::fromUtf8(lg_mcd_rom_GetDescription(boot_rom_id));
	
	// Check the region code.
	boot_rom_region_code = lg_mcd_rom_GetRegion(boot_rom_id);
	if (boot_rom_region_code != region_code)
	{
		// Region code doesn't match.
		if ((region_code == Region_Japan_NTSC && boot_rom_region_code == Region_Japan_PAL) ||
		    (region_code == Region_Japan_PAL && boot_rom_region_code == Region_Japan_NTSC))
		{
			// Japanese Boot ROM. NTSC/PAL doesn't affect region lock.
			// Do nothing here.
		}
		else
		{
			// USA or Europe Boot ROM. Region is incorrect.
			QString expected_region = QString::fromUtf8(lg_mcd_rom_GetRegionCodeString(region_code));
			QString boot_rom_region = QString::fromUtf8(lg_mcd_rom_GetRegionCodeString(boot_rom_region_code));
			
			rom_notes += sWarning + TR("Region code is incorrect.") + "<br/>\n" +
				     TR("(expected %1; found %2)").arg(expected_region).arg(boot_rom_region) + "<br/>\n";
			
			// Set the icon to warning.
			filename_icon = QStyle::SP_MessageBoxWarning;
		}
	}
	
	// Check the ROM's support status.
	boot_rom_status = lg_mcd_rom_GetSupportStatus(boot_rom_id);
	switch (boot_rom_status)
	{
		case RomStatus_Recommended:
		case RomStatus_Supported:
			// ROM is either recommended or supported.
			// TODO: Make a distinction between the two?
			break;
		
		case RomStatus_Unsupported:
		default:
			// ROM is unsupported.
			rom_notes += sWarning + TR("This Boot ROM is not supported by Gens/GS II.") + "<br/>\n";
			filename_icon = QStyle::SP_MessageBoxWarning;
			break;
		
		case RomStatus_Broken:
			// ROM is known to be broken.
			rom_notes += sWarning + TR("This Boot ROM is known to be broken on all emulators.") + "<br/>\n";
			filename_icon = QStyle::SP_MessageBoxCritical;
			break;
	}
	
	// Get the ROM's notes.
	rom_notes += QString::fromUtf8(lg_mcd_rom_GetNotes(boot_rom_id)).replace('\n', "<br/>\n");
	
rom_identified:
	// Free the ROM data buffer if it was allocated.
	free(rom_data);
	
	// Set the Boot ROM filename textbox icon.
	txtRomFile->setIcon(style()->standardIcon(filename_icon));
	
	// Set the Boot ROM description.
	QString s_ret;
	s_ret = TR("ROM identified as: %1").arg(rom_id);
	if (!rom_notes.isEmpty())
		s_ret += "<br/>\n<br/>\n" + rom_notes;
	if (!rom_size_warning.isEmpty())
		s_ret += "<br/>\n<br/>\n" + rom_size_warning;
	return QString(s_ret);
}


/**
 * mcdDisplayRomFileStatus(): Sega CD: Display Boot ROM file status.
 * @param rom_id Sega CD Boot ROM ID.
 * @param rom_desc ROM file description. (detected by examining the ROM)
 */
void GeneralConfigWindow::mcdDisplayRomFileStatus(const QString& rom_id, const QString& rom_desc)
{
	// Set the ROM description.
	QString sel_rom = TR("Selected ROM: %1");
	lblMcdSelectedRom->setText(sel_rom.arg(rom_id) + "<br/>\n" + rom_desc);
	lblMcdSelectedRom->setTextFormat(Qt::RichText);
}

void GeneralConfigWindow::on_txtMcdRomUSA_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Sega CD (U)"), sMcdRomStatus_USA); }
void GeneralConfigWindow::on_txtMcdRomEUR_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Mega CD (E)"), sMcdRomStatus_EUR); }
void GeneralConfigWindow::on_txtMcdRomJPN_focusIn(void)
	{ mcdDisplayRomFileStatus(TR("Mega CD (J)"), sMcdRomStatus_JPN); }

void GeneralConfigWindow::on_txtMcdRomUSA_textChanged(void)
{
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomUSA, Region_USA);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_USA = sNewRomStatus;
		mcdDisplayRomFileStatus(TR("Sega CD (U)"), sMcdRomStatus_USA);
	}
}
void GeneralConfigWindow::on_txtMcdRomEUR_textChanged(void)
{
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomEUR, Region_Europe);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_EUR = sNewRomStatus;
		mcdDisplayRomFileStatus(TR("Mega CD (E)"), sMcdRomStatus_EUR);
	}
}
void GeneralConfigWindow::on_txtMcdRomJPN_textChanged(void)
{
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomJPN, Region_Japan_NTSC);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_JPN = sNewRomStatus;
		mcdDisplayRomFileStatus(TR("Mega CD (J)"), sMcdRomStatus_JPN);
	}
}

}
