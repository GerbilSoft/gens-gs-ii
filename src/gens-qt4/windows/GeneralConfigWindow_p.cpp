/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GeneralConfigWindow_p.cpp: General Configuration Window. (PRIVATE CLASS)*
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

#include "GeneralConfigWindow_p.hpp"
#include "GeneralConfigWindow.hpp"
#include "gqt4_main.hpp"

// Qt includes.
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QLineEdit>

// Gens widgets.
#include "widgets/GensLineEdit.hpp"

// ROM database.
#include "../RomDB/mcd_rom_db.h"

// crc32()
#include <zlib.h>

namespace GensQt4 {

// Static member initialization.
GeneralConfigWindow *GeneralConfigWindowPrivate::ms_GeneralConfigWindow = nullptr;

GeneralConfigWindowPrivate::GeneralConfigWindowPrivate(GeneralConfigWindow *q)
	: q_ptr(q)
{
	// Initialize the "Warning" string.
	// TODO: Needs to be retranslated...
	sWarning = QLatin1String("<span style='color: red'><b>") +
			GeneralConfigWindow::tr("Warning:") +
			QLatin1String("</b></span> ");

	// Create the toolbar action group.
	actgrpToolbar = new QActionGroup(q);

	// TODO: Determine this at runtime for Linux.
#ifdef Q_WS_MAC
	applySettingsImmediately = true;
#else /* !Q_WS_MAC */
	applySettingsImmediately = false;
#endif /* Q_WS_MAC */
}

/**
 * Check if the warranty is void.
 * If it is, we'll show some super secret settings.
 * @return True if the warranty is void.
 */
bool GeneralConfigWindowPrivate::isWarrantyVoid(void)
{
	return (gqt4_cfg->get(QLatin1String("iKnowWhatImDoingAndWillVoidTheWarranty")).toBool());
}

/**
 * Select a color for the OSD.
 * @param color_id	[in] Color ID.
 * @param init_color	[in] Initial color.
 * @return Selected color, or invalid QColor if cancelled.
 */
QColor GeneralConfigWindowPrivate::osdSelectColor(QString color_id, const QColor& init_color)
{
	// Create the dialog title.
	QString title = GeneralConfigWindow::tr("Select OSD %1 Color").arg(color_id);

	// QColorDialog::getColor() returns an invalid color
	// if the dialog is cancelled.
	Q_Q(GeneralConfigWindow);
	return QColorDialog::getColor(init_color, q, title);
}

/**
 * Get the region code order from lstRegionDetect.
 * @return Region code order.
 */
uint16_t GeneralConfigWindowPrivate::regionCodeOrder(void) const
{
	uint16_t ret = 0;
	for (int i = 0; i < ui.lstRegionDetect->count(); i++) {
		const QListWidgetItem *item = ui.lstRegionDetect->item(i);
		ret <<= 4;
		ret |= (uint16_t)item->data(Qt::UserRole).toUInt();
	}
	return ret;
}

/**
 * Select a ROM file.
 * @param rom_desc	[in] ROM file description.
 * @param txtRomFile	[in] ROM file textbox.
 */
void GeneralConfigWindowPrivate::selectRomFile(const QString &rom_desc, QLineEdit *txtRomFile)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"

	// Create the dialog title.
	QString title = GeneralConfigWindow::tr("Select %1").arg(rom_desc);

	// TODO: Specify the current Boot ROM filename as the default filename.
	// TODO: Move the filename filters somewhere else.
	Q_Q(GeneralConfigWindow);
	QString filename = QFileDialog::getOpenFileName(q, title,
			txtRomFile->text(),	// Default filename.
			GeneralConfigWindow::tr("ROM Images") +
			QLatin1String(
				" (*.bin *.gen *.md *.smd"
#ifdef HAVE_ZLIB
				ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
				LZMA_EXT
#endif /* HAVE_LZMA */
				RAR_EXT
				");;") +
			GeneralConfigWindow::tr("All Files") + QLatin1String(" (*.*)"));

	if (filename.isEmpty())
		return;

	// Convert to native pathname separators.
	filename = QDir::toNativeSeparators(filename);

	// Set the filename text.
	// ROM file status will be updated automatically by
	// the textChanged() signal from QLineEdit.
	txtRomFile->setText(filename);

	// Set focus to the textbox.
	txtRomFile->setFocus(Qt::OtherFocusReason);
}

/**
 * Update the ROM file status.
 */
void GeneralConfigWindowPrivate::updateRomFileStatus(void)
{
	// Update Sega Genesis TMSS ROM file status.
	// TODO: Update the display for the last selected ROM.
	QString sNewRomStatus;
	sNewRomStatus = mdUpdateTmssRomFileStatus(ui.txtMDTMSSRom);
	if (!sNewRomStatus.isEmpty())
		sMDTmssRomStatus = sNewRomStatus;

	// Update Sega CD Boot ROM file status.
	// TODO: Update the display for the last selected ROM.
	sNewRomStatus = mcdUpdateRomFileStatus(ui.txtMcdRomUSA, MCD_REGION_USA);
	if (!sNewRomStatus.isEmpty())
		sMcdRomStatus_USA = sNewRomStatus;
	sNewRomStatus = mcdUpdateRomFileStatus(ui.txtMcdRomEUR, MCD_REGION_EUROPE);
	if (!sNewRomStatus.isEmpty())
		sMcdRomStatus_EUR = sNewRomStatus;
	sNewRomStatus = mcdUpdateRomFileStatus(ui.txtMcdRomJPN, MCD_REGION_JAPAN);
	if (!sNewRomStatus.isEmpty())
		sMcdRomStatus_JPN = sNewRomStatus;
	sNewRomStatus = mcdUpdateRomFileStatus(ui.txtMcdRomAsia, MCD_REGION_ASIA);
	if (!sNewRomStatus.isEmpty())
		sMcdRomStatus_Asia = sNewRomStatus;
}

/**
 * Sega Genesis: Update TMSS ROM file status.
 * @param txtRomFile ROM file textbox.
 * @return Updated ROM status.
 */
QString GeneralConfigWindowPrivate::mdUpdateTmssRomFileStatus(GensLineEdit *txtRomFile)
{
	// ROM data.
	uint8_t *rom_data = nullptr;
	z_crc_t rom_crc32;
	int data_len;
	bool is_overdump = false;

	// Line break string.
	static const QString sLineBreak = QLatin1String("<br/>\n");

	// Check if the file exists.
	Q_Q(GeneralConfigWindow);
	QString filename = txtRomFile->text();
	if (!QFile::exists(filename)) {
		// File doesn't exist.
		// NOTE: KDE 4's Oxygen theme doesn't have a question icon.
		// SP_MessageBoxQuestion is redirected to SP_MessageBoxInformation on KDE 4.
		// TODO: Set ROM file notes.
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		if (filename.isEmpty()) {
			return GeneralConfigWindow::tr("No ROM filename specified.");
		} else {
			return GeneralConfigWindow::tr("The specified ROM file was not found.");
		}
	}

	// Check the ROM file.
	// TODO: Decompressor support.
	QStyle::StandardPixmap filename_icon = QStyle::SP_DialogYesButton;
	QString rom_id = GeneralConfigWindow::tr("Unknown");
	QString rom_notes;

	// Open the ROM file using LibGens::Rom.
	QScopedPointer<LibGens::Rom> rom(new LibGens::Rom(filename.toUtf8().constData()));
	if (!rom->isOpen()) {
		// Error opening ROM file.
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		return GeneralConfigWindow::tr("Error opening ROM file.");
	}

	// Multi-file ROM archives are not supported for TMSS ROMs.
	if (rom->isMultiFile()) {
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxWarning));
		return (sWarning +
			GeneralConfigWindow::tr("This archive has multiple files.") + sLineBreak +
			GeneralConfigWindow::tr("Multi-file ROM archives are not currently supported for the TMSS ROM."));
	}

	// Check the ROM filesize.
	// Valid TMSS ROMs are 2 KB.
	static const int TMSS_ROM_FILESIZE = 2048;
	if (rom->romSize() != TMSS_ROM_FILESIZE) {
		// Wrong ROM size.
		// Identify the ROM even if it's too big.
		// (Some copies of the TMSS ROM are overdumped.)
		// Also, don't check ridiculously large TMSS ROMs.
		if (rom->romSize() < TMSS_ROM_FILESIZE || rom->romSize() > 524288) {
			// ROM is too small, so it's guaranteed to not match anything in the database.
			// Also, our TMSS implementation has a maximum size of 512 KB.
			goto rom_identified;
		}
	}

	// Load the ROM data and calculate the CRC32..
	// TODO: LibGens::Rom::loadRom() fails if the ROM buffer isn't >= the ROM size.
	rom_data = (uint8_t*)malloc(rom->romSize());
	data_len = rom->loadRom(rom_data, rom->romSize());
	if (data_len != rom->romSize()) {
		// Error reading data from the file.
		rom_notes = GeneralConfigWindow::tr("Error reading file.") + sLineBreak +
			    GeneralConfigWindow::tr("(expected %L1 bytes; read %L2 bytes)")
			    .arg(rom->romSize()).arg(data_len);
		goto rom_identified;
	}

	// Calculate the CRC32 of the first 2 KB using zlib.
	static const z_crc_t TMSS_ROM_CRC32 = 0x3F888CF4;
	rom_crc32 = crc32(0, rom_data, TMSS_ROM_FILESIZE);

	// Check if this is an overdump.
	// TMSS ROM is 2 KB, but may be overdumped to 4 KB with the
	// second half as open-bus. (usually 0x00 or 0xFF)
	// NOTE: If the TMSS ROM filesize isn't divisible by 2 KB,
	// don't bother checking for overdumps.
	if (data_len > TMSS_ROM_FILESIZE &&
	    (data_len % TMSS_ROM_FILESIZE == 0))
	{
		// TMSS ROM is larger than 2KB, and is a multiple of 2 KB.
		// - Check all 2 KB blocks for 0x00 or 0xFF.
		// - Also check even 2 KB blocks for the TMSS CRC32. [This overrides the 0x00/0xFF check.]
		// TODO: Optimize this.
		is_overdump = true;
		for (int i = TMSS_ROM_FILESIZE; i < data_len; i += TMSS_ROM_FILESIZE) {
			const uint8_t *ptr = &rom_data[i];

			// Check for all 0x00 or all 0xFF.
			const uint8_t first_byte = *ptr++;
			if (first_byte != 0x00 && first_byte != 0xFF) {
				// Not an overdump.
				is_overdump = false;
			} else {
				for (int j = TMSS_ROM_FILESIZE-1; j > 0; j--, ptr++) {
					if (*ptr != first_byte) {
						// Not an overdump.
						is_overdump = false;
						break;
					}
				}
			}

			if (!is_overdump) {
				// Block is not all 0x00 or 0xFF.
				if ((i % (TMSS_ROM_FILESIZE*2)) == 0) {
					// Even block. Check for TMSS CRC32.
					is_overdump = true;
					const z_crc_t overdump_crc32 = crc32(0, &rom_data[i], TMSS_ROM_FILESIZE);
					if (overdump_crc32 != TMSS_ROM_CRC32) {
						// Wrong CRC32.
						is_overdump = false;
						break;
					}
				} else {
					// Odd block. Not an overdump.
					break;
				}
			}
		}
	} else {
		// TMSS ROM is too small, or is not a multiple of 2 KB.
		is_overdump = false;
	}

	// Check what ROM this is.
	switch (rom_crc32) {
		case TMSS_ROM_CRC32:
			// Standard TMSS ROM.
			rom_id = GeneralConfigWindow::tr("Genesis TMSS ROM");
			rom_notes = GeneralConfigWindow::tr("This is a known good dump of the Genesis TMSS ROM.");
			break;

		default:
			// Unknown TMSS ROM.
			// TODO: Add more variants.
			filename_icon = QStyle::SP_MessageBoxQuestion;
			rom_notes = GeneralConfigWindow::tr("Unknown ROM image. May not work properly for TMSS.");
			break;
	}

rom_identified:
	// Free the ROM data if it was allocated.
	free(rom_data);

	// Set the Boot ROM description.
	QString s_ret = GeneralConfigWindow::tr("ROM identified as: %1").arg(rom_id);
	if (!rom_notes.isEmpty()) {
		s_ret += sLineBreak + sLineBreak + rom_notes;
	}

	// Check if the ROM is the right size.
	if (rom->romSize() != TMSS_ROM_FILESIZE) {
		// Wrong ROM size.
		QString rom_size_warning;
		if (is_overdump) {
			// Overdump. This ROM is okay.
			// TODO: Convert bytes to KB, MB, etc.
			rom_size_warning = 
				GeneralConfigWindow::tr("This ROM is larger than the expected size of the TMSS ROM.") + sLineBreak +
				GeneralConfigWindow::tr("(expected %L1 bytes; found %L2 bytes)")
				.arg(TMSS_ROM_FILESIZE).arg(rom->romSize()) + sLineBreak + sLineBreak +
				GeneralConfigWindow::tr("The extra data is either empty space or extra copies of the "
							"TMSS ROM, so it will work correctly.");
		} else {
			// Not an overdump.
			filename_icon = QStyle::SP_MessageBoxWarning;

			rom_size_warning = sWarning +
					GeneralConfigWindow::tr("ROM size is incorrect.") + sLineBreak +
					GeneralConfigWindow::tr("(expected %L1 bytes; found %L2 bytes)")
					.arg(TMSS_ROM_FILESIZE).arg(rom->romSize());
		}

		s_ret += sLineBreak + sLineBreak + rom_size_warning;
	}

	// Set the Boot ROM filename textbox icon.
	txtRomFile->setIcon(q->style()->standardIcon(filename_icon));

	return s_ret;
}

/**
 * Sega CD: Update Boot ROM file status.
 * @param txtRomFile ROM file textbox.
 * @param region_code Expected ROM region code. (bitmask)
 * @return Updated ROM status.
 */
QString GeneralConfigWindowPrivate::mcdUpdateRomFileStatus(GensLineEdit *txtRomFile, int region_code)
{
	// ROM data buffer.
	uint8_t *rom_data = nullptr;
	int data_len;
	uint32_t rom_crc32;
	int boot_rom_id;
	int boot_rom_region_code;
	MCD_RomStatus_t boot_rom_status;

	// Line break string.
	static const QString sLineBreak = QLatin1String("<br/>\n");

	// Check if the file exists.
	Q_Q(GeneralConfigWindow);
	QString filename = txtRomFile->text();
	if (!QFile::exists(filename)) {
		// File doesn't exist.
		// NOTE: KDE 4's Oxygen theme doesn't have a question icon.
		// SP_MessageBoxQuestion is redirected to SP_MessageBoxInformation on KDE 4.
		// TODO: Set ROM file notes.
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		if (filename.isEmpty())
			return GeneralConfigWindow::tr("No ROM filename specified.");
		else
			return GeneralConfigWindow::tr("The specified ROM file was not found.");
	}

	// Check the ROM file.
	// TODO: Decompressor support.
	QStyle::StandardPixmap filename_icon = QStyle::SP_DialogYesButton;
	QString rom_id = GeneralConfigWindow::tr("Unknown");
	QString rom_notes;
	QString rom_size_warning;

	// Open the ROM file using LibGens::Rom.
	QScopedPointer<LibGens::Rom> rom(new LibGens::Rom(filename.toUtf8().constData()));
	if (!rom->isOpen()) {
		// Error opening ROM file.
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
		return GeneralConfigWindow::tr("Error opening ROM file.");
	}

	// Multi-file ROM archives are not supported for Sega CD Boot ROMs.
	if (rom->isMultiFile()) {
		txtRomFile->setIcon(q->style()->standardIcon(QStyle::SP_MessageBoxWarning));
		return (sLineBreak + sWarning +
				GeneralConfigWindow::tr("This archive has multiple files.") + sLineBreak +
				GeneralConfigWindow::tr("Multi-file ROM archives are not currently supported for Sega CD Boot ROMs."));
	}

	// Check the ROM filesize.
	// Valid boot ROMs are 128 KB.
	// Smaller ROMs will not work; larger ROMs may work, but warn anyway.
	if (rom->romSize() != MCD_ROM_FILESIZE) {
		// Wrong ROM size.
		filename_icon = QStyle::SP_MessageBoxWarning;

		rom_size_warning = sWarning + 
				GeneralConfigWindow::tr("ROM size is incorrect.") + sLineBreak +
				GeneralConfigWindow::tr("(expected %L1 bytes; found %L2 bytes)")
				.arg(MCD_ROM_FILESIZE).arg(rom->romSize());

		// Identify the ROM even if it's too big.
		// (Some copies of the TMSS ROM are overdumped.)
		// Also, don't check ridiculously large Sega CD boot ROMs.
		if (rom->romSize() < MCD_ROM_FILESIZE || rom->romSize() > 1048576) {
			// ROM is too small, so it's guaranteed to not match anything in the database.
			goto rom_identified;
		}
	}

	// Load the ROM data and calculate the CRC32..
	// TODO: LibGens::Rom::loadRom() fails if the ROM buffer isn't >= the ROM size.
	rom_data = (uint8_t*)malloc(rom->romSize());
	data_len = rom->loadRom(rom_data, rom->romSize());
	if (data_len != rom->romSize()) {
		// Error reading data from the file.
		rom_notes = GeneralConfigWindow::tr("Error reading file.") + sLineBreak +
			    GeneralConfigWindow::tr("(expected %L1 bytes; read %L2 bytes)")
			    .arg(MCD_ROM_FILESIZE).arg(data_len);
		goto rom_identified;
	}

	// Fix up the ROM's Initial SP and Initial HINT vectors.
	memcpy(&rom_data[0x00], mcd_rom_InitSP, sizeof(mcd_rom_InitSP));
	memcpy(&rom_data[0x70], mcd_rom_InitHINT, sizeof(mcd_rom_InitHINT));

	// Calculate the CRC32 using zlib.
	// NOTE: rom->rom_crc32() will be incorrect if the ROM is too big.
	rom_crc32 = crc32(0, rom_data, MCD_ROM_FILESIZE);

	// Look up the CRC32 in the Sega CD Boot ROM database.
	boot_rom_id = mcd_rom_FindByCRC32(rom_crc32);
	if (boot_rom_id < 0) {
		// Boot ROM was not found in the database.
		filename_icon = QStyle::SP_MessageBoxWarning;
		goto rom_identified;
	}

	// ROM found. Get the description and other information.
	rom_id = QString::fromUtf8(mcd_rom_GetDescription(boot_rom_id));

	// Check the region code.
	boot_rom_region_code = mcd_rom_GetRegion(boot_rom_id);
	if ((boot_rom_region_code & region_code) == 0) {
		// ROM doesn't support this region.
		int boot_rom_region_primary = mcd_rom_GetPrimaryRegion(boot_rom_id);
		QString expected_region = QString::fromUtf8(mcd_rom_GetRegionCodeString(region_code));
		QString boot_rom_region = QString::fromUtf8(mcd_rom_GetRegionCodeString(boot_rom_region_primary));

		rom_notes += sWarning +
			     GeneralConfigWindow::tr("Region code is incorrect.") + sLineBreak +
			     GeneralConfigWindow::tr("(expected %1; found %2)")
			     .arg(expected_region).arg(boot_rom_region) + sLineBreak;

		// Set the icon to warning.
		filename_icon = QStyle::SP_MessageBoxWarning;
	}

	// Check the ROM's support status.
	boot_rom_status = mcd_rom_GetSupportStatus(boot_rom_id);
	switch (boot_rom_status) {
		case RomStatus_Recommended:
		case RomStatus_Supported:
			// ROM is either recommended or supported.
			// TODO: Make a distinction between the two?
			break;

		case RomStatus_Unsupported:
		default:
			// ROM is unsupported.
			rom_notes += sWarning +
				     GeneralConfigWindow::tr("This Boot ROM is not supported by Gens/GS II.") + sLineBreak;
			filename_icon = QStyle::SP_MessageBoxWarning;
			break;

		case RomStatus_Broken:
			// ROM is known to be broken.
			rom_notes += sWarning +
				     GeneralConfigWindow::tr("This Boot ROM is known to be broken on all emulators.") + sLineBreak;
			filename_icon = QStyle::SP_MessageBoxCritical;
			break;
	}

	// Get the ROM's notes.
	rom_notes += QString::fromUtf8(mcd_rom_GetNotes(boot_rom_id)).replace(QChar(L'\n'), sLineBreak);

rom_identified:
	// Free the ROM data buffer if it was allocated.
	free(rom_data);

	// Set the Boot ROM filename textbox icon.
	txtRomFile->setIcon(q->style()->standardIcon(filename_icon));

	// Set the Boot ROM description.
	QString s_ret;
	s_ret = GeneralConfigWindow::tr("ROM identified as: %1").arg(rom_id);
	if (!rom_notes.isEmpty())
		s_ret += sLineBreak + sLineBreak + rom_notes;
	if (!rom_size_warning.isEmpty())
		s_ret += sLineBreak + sLineBreak + rom_size_warning;
	return QString(s_ret);
}

/**
 * Sega CD: Display Boot ROM file status.
 * @param rom_id Sega CD Boot ROM ID.
 * @param rom_desc ROM file description. (detected by examining the ROM)
 */
void GeneralConfigWindowPrivate::mcdDisplayRomFileStatus(const QString &rom_id, const QString &rom_desc)
{
	// Set the ROM description.
	QString sel_rom = GeneralConfigWindow::tr("Selected ROM: %1");
	ui.lblMcdSelectedRom->setText(sel_rom.arg(rom_id) +
				QLatin1String("<br/>\n") + rom_desc);
	ui.lblMcdSelectedRom->setTextFormat(Qt::RichText);
}

/**
 * Display external program file status.
 * @param file_id File ID.
 * @param file_desc File description. (detected by examining the file)
 */
void GeneralConfigWindowPrivate::extprgDisplayFileStatus(const QString &file_id, const QString &file_desc)
{
	// Set the file description.
	QString sel_prg = GeneralConfigWindow::tr("Selected Program: %1");
	ui.lblExtPrgSel->setText(sel_prg.arg(file_id) +
				QLatin1String("<br/>\n") + file_desc);
	ui.lblExtPrgSel->setTextFormat(Qt::RichText);
}

/**
 * Enable or disable the Apply button.
 * @param enabled True to enable; false to disable.
 */
void GeneralConfigWindowPrivate::setApplyButtonEnabled(bool enabled)
{
	QPushButton *btnApply = ui.buttonBox->button(QDialogButtonBox::Apply);
	if (btnApply)
		btnApply->setEnabled(enabled);
}

}
