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
#include "gqt4_main.hpp"

// C includes.
#include <stdint.h>

// zlib
#include <zlib.h>

// Qt4 includes.
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QPainter>

// libgens: RAR decompressor
#include "libgens/Decompressor/DcRar.hpp"

namespace GensQt4
{

// Static member initialization.
GeneralConfigWindow *GeneralConfigWindow::m_GeneralConfigWindow = NULL;

// Button CSS colors.
const QString GeneralConfigWindow::ms_sCssBtnColors =
	QString::fromLatin1("QPushButton { background-color: %1; color: %2; }");

// Warning string.
const QString GeneralConfigWindow::ms_sWarning =
	QString::fromLatin1("<span style='color: red'><b>") + tr("Warning:") + QString::fromLatin1("</b></span> ");

/**
 * GeneralConfigWindow(): Initialize the General Configuration window.
 */
GeneralConfigWindow::GeneralConfigWindow(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	// Sega CD: Initialize Boot ROM textboxes.
	txtMcdRomUSA = new GensLineEdit(this);
	txtMcdRomUSA->setObjectName(QString::fromUtf8("txtMcdRomUSA"));
	txtMcdRomUSA->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	txtMcdRomEUR = new GensLineEdit(this);
	txtMcdRomEUR->setObjectName(QString::fromUtf8("txtMcdRomEUR"));
	txtMcdRomEUR->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	txtMcdRomJPN = new GensLineEdit(this);
	txtMcdRomJPN->setObjectName(QString::fromUtf8("txtMcdRomJPN"));
	txtMcdRomJPN->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	
	// External Programs: Initialize textboxes.
	txtExtPrgUnRAR = new GensLineEdit(this);
	txtExtPrgUnRAR->setObjectName(QString::fromUtf8("txtExtPrgUnRAR"));
	txtExtPrgUnRAR->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	
	// Initialize the Qt4 UI.
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
	// Set up a signal for the Apply button.
	QPushButton *btnApply = buttonBox->button(QDialogButtonBox::Apply);
	if (btnApply)
		connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
	
	/** Intro Effect **/
	
	// Intro Effect Color: Create the icons.
	static const QRgb intro_colors[8] =
		{0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
		 0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF};
	QSize iconSize = cboIntroColor->iconSize();
	for (int i = 0; i < 8; i++)
	{
		// TODO: Determine an appropriate icon size.
		QPixmap pxm(24, 16);
		pxm.fill(QColor(intro_colors[i]));
		
		// Draw a black rectangle outline.
		// TODO: Use a darkened version of the color like Mac OS X's NSColorWell?
		QPainter painter(&pxm);
		painter.setPen(QColor(0, 0, 0));
		painter.drawRect(0, 0, iconSize.width()-1, iconSize.height()-1);
		
		// Convert to a QIcon and add it to cboIntroColor.
		QIcon icon(pxm);
		cboIntroColor->setItemIcon(i, icon);
	}
	
	// TODO
	
	/** Sega CD **/
	
	// Sega CD: Add the Boot ROM textboxes to the grid layout.
	const QString sMcdBootRom_PlaceholderText = tr("Select a %1 Boot ROM...");
	
	// Sega CD: USA Boot ROM
	txtMcdRomUSA->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Sega CD (U)")));
	gridMcdRoms->addWidget(txtMcdRomUSA, 0, 1);
	lblMcdRomUSA->setBuddy(txtMcdRomUSA);
	setTabOrder(lblMcdRomUSA, txtMcdRomUSA);
	setTabOrder(txtMcdRomUSA, btnMcdRomUSA);
	// Sega CD: EUR Boot ROM
	txtMcdRomEUR->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Mega CD (E)")));
	gridMcdRoms->addWidget(txtMcdRomEUR, 1, 1);
	lblMcdRomEUR->setBuddy(txtMcdRomEUR);
	setTabOrder(txtMcdRomEUR, btnMcdRomEUR);
	// Sega CD: JPN Boot ROM
	txtMcdRomJPN->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Mega CD (J)")));
	gridMcdRoms->addWidget(txtMcdRomJPN, 2, 1);
	lblMcdRomJPN->setBuddy(txtMcdRomJPN);
	setTabOrder(txtMcdRomJPN, btnMcdRomJPN);
	
	// External Programs: Add the textboxes to the grid layout.
#ifdef _WIN32
	lblExtPrgUnRAR->setText(tr("UnRAR DLL:"));
	txtExtPrgUnRAR->setPlaceholderText(tr("Select an UnRAR DLL..."));
#else
	txtExtPrgUnRAR->setPlaceholderText(tr("Select a RAR or UnRAR binary..."));
#endif
	gridExtPrg->addWidget(txtExtPrgUnRAR, 0, 1);
	lblExtPrgUnRAR->setBuddy(txtExtPrgUnRAR);
	setTabOrder(lblExtPrgUnRAR, txtExtPrgUnRAR);
	
	// Load configuration.
	reload();
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
 * reload(): Reload configuration.
 */
void GeneralConfigWindow::reload(void)
{
	// Onscreen Display.
	
	// TODO: If grayscale version of color is < 128, make text white.
	// TODO: If grayscale version of color is >= 128, make text black.
	
	int grayI;
	QColor colorText;
	
	// TODO: Split button coloring into a separate function.
	// Alternatively, subclass QPushButton and create GensColorButton.
	
	/** Onscreen display: FPS counter. **/
	chkOsdFpsEnable->setChecked(gqt4_config->osdFpsEnabled());
	m_osdFpsColor = gqt4_config->osdFpsColor();
	grayI = QColor_Grayscale(m_osdFpsColor);
	colorText = (grayI >= 128 ? QColor(0,0,0) : QColor(255,255,255));
	btnOsdFpsColor->setStyleSheet(ms_sCssBtnColors.arg(m_osdFpsColor.name()).arg(colorText.name()));
	
	/** Onscreen display: Messages. **/
	chkOsdMsgEnable->setChecked(gqt4_config->osdMsgEnabled());
	m_osdMsgColor = gqt4_config->osdMsgColor();
	grayI = QColor_Grayscale(m_osdMsgColor);
	colorText = (grayI >= 128 ? QColor(0,0,0) : QColor(255,255,255));
	btnOsdMsgColor->setStyleSheet(ms_sCssBtnColors.arg(m_osdMsgColor.name()).arg(colorText.name()));
	
	// TODO: Get intro effect config from GensConfig.
	cboIntroColor->setCurrentIndex(7);
	
	/** Sega CD Boot ROMs. **/
	txtMcdRomUSA->setText(gqt4_config->mcdRomUSA());
	txtMcdRomEUR->setText(gqt4_config->mcdRomEUR());
	txtMcdRomJPN->setText(gqt4_config->mcdRomJPN());
	
	/** External programs. **/
	txtExtPrgUnRAR->setText(gqt4_config->extprgUnRAR());
	
	/** Graphics settings. **/
	chkAspectRatioConstraint->setChecked(gqt4_config->aspectRatioConstraint());
	chkFastBlur->setChecked(gqt4_config->fastBlur());
	hsldContrast->setValue(gqt4_config->contrast());
	hsldBrightness->setValue(gqt4_config->brightness());
	chkGrayscale->setChecked(gqt4_config->grayscale());
	chkInverted->setChecked(gqt4_config->inverted());
	// TODO: Color Scale Method.
	
	/** General settings. **/
	chkAutoFixChecksum->setChecked(gqt4_config->autoFixChecksum());
	chkAutoPause->setChecked(gqt4_config->autoPause());
	chkBorderColor->setChecked(gqt4_config->borderColor());
	chkPauseTint->setChecked(gqt4_config->pauseTint());
	chkNtscV30Rolling->setChecked(gqt4_config->ntscV30Rolling());
	
	// Disable the Apply button.
	setApplyButtonEnabled(false);
}


/**
 * apply(): Apply dialog settings.
 */
void GeneralConfigWindow::apply(void)
{
	/** Onscreen display. **/
	gqt4_config->setOsdFpsEnabled(chkOsdFpsEnable->isChecked());
	gqt4_config->setOsdFpsColor(m_osdFpsColor);
	gqt4_config->setOsdMsgEnabled(chkOsdMsgEnable->isChecked());
	gqt4_config->setOsdMsgColor(m_osdMsgColor);
	
	// TODO: Save intro effect config to GensConfig.
	//gqt4_config->setIntroColor(cboIntroColor->currentIndex());
	
	/** Sega CD Boot ROMs. **/
	gqt4_config->setMcdRomUSA(txtMcdRomUSA->text());
	gqt4_config->setMcdRomEUR(txtMcdRomEUR->text());
	gqt4_config->setMcdRomJPN(txtMcdRomJPN->text());
	
	/** External programs. **/
	gqt4_config->setExtPrgUnRAR(txtExtPrgUnRAR->text());
	
	/** Graphics settings. **/
	gqt4_config->setAspectRatioConstraint(chkAspectRatioConstraint->isChecked());
	gqt4_config->setFastBlur(chkFastBlur->isChecked());
	gqt4_config->setContrast(hsldContrast->value());
	gqt4_config->setBrightness(hsldBrightness->value());
	gqt4_config->setGrayscale(chkGrayscale->isChecked());
	gqt4_config->setInverted(chkInverted->isChecked());
	// TODO: Color Scale Method.
	
	/** General settings. **/
	gqt4_config->setAutoFixChecksum(chkAutoFixChecksum->isChecked());
	gqt4_config->setAutoPause(chkAutoPause->isChecked());
	gqt4_config->setBorderColor(chkBorderColor->isChecked());
	gqt4_config->setPauseTint(chkPauseTint->isChecked());
	gqt4_config->setNtscV30Rolling(chkNtscV30Rolling->isChecked());
	
	// Disable the Apply button.
	// TODO: If Apply was clicked, set focus back to the main window elements.
	// Otherwise, Cancel will receive focus.
	setApplyButtonEnabled(false);
}


/** Onscreen display **/


/**
 * osdSelectColor(): Select a color for the OSD.
 * @param color_id	[in] Color ID.
 * @param init_color	[in] Initial color.
 * @return Selected color, or invalid QColor if cancelled.
 */
QColor GeneralConfigWindow::osdSelectColor(const QString& color_id, const QColor& init_color)
{
	// Create the dialog title.
	QString title = tr("Select OSD %1 Color").arg(color_id);
	
	// QColorDialog::getColor() returns an invalid color
	// if the dialog is cancelled.
	return QColorDialog::getColor(init_color, this, title);
}

void GeneralConfigWindow::on_btnOsdFpsColor_clicked(void)
{
	QColor color = osdSelectColor(tr("FPS counter"), m_osdFpsColor);
	if (!color.isValid() || m_osdFpsColor == color)
		return;
	
	// TODO: If grayscale version of color is < 128, make text white.
	// TODO: If grayscale version of color is >= 128, make text black.
	m_osdFpsColor = color;
	int grayI = QColor_Grayscale(m_osdFpsColor);
	QColor colorText = (grayI >= 128 ? QColor(0,0,0) : QColor(255,255,255));
	btnOsdFpsColor->setStyleSheet(ms_sCssBtnColors.arg(m_osdFpsColor.name()).arg(colorText.name()));
	
	// Settings have been changed.
	setApplyButtonEnabled(true);
}

void GeneralConfigWindow::on_btnOsdMsgColor_clicked(void)
{
	QColor color = osdSelectColor(tr("Messages"), m_osdMsgColor);
	if (!color.isValid() || m_osdMsgColor == color)
		return;
	
	// TODO: If grayscale version of color is < 128, make text white.
	// TODO: If grayscale version of color is >= 128, make text black.
	m_osdMsgColor = color;
	int grayI = QColor_Grayscale(m_osdMsgColor);
	QColor colorText = (grayI >= 128 ? QColor(0,0,0) : QColor(255,255,255));
	btnOsdMsgColor->setStyleSheet(ms_sCssBtnColors.arg(m_osdMsgColor.name()).arg(colorText.name()));
	
	// Settings have been changed.
	setApplyButtonEnabled(true);
}


/** Sega CD **/


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
	QString title = tr("Select %1 Boot ROM").arg(rom_id);
	
	// TODO: Specify the current Boot ROM filename as the default filename.
	// TODO: Move the filename filters somewhere else.
	QString filename = QFileDialog::getOpenFileName(this, title,
			txtRomFile->text(),	// Default filename.
			tr("Sega CD Boot ROM images") +
			QString::fromLatin1(
				" (*.bin *.gen *.md *.smd"
#ifdef HAVE_ZLIB
				ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
				LZMA_EXT
#endif /* HAVE_LZMA */
				RAR_EXT
				");;") +
			tr("All Files") + QString::fromLatin1("(*.*)"));
	
	if (filename.isEmpty())
		return;
	
	// Convert to native pathname separators.
	filename = QDir::toNativeSeparators(filename);
	
	// Set the filename text.
	// ROM file status will be updated automatically by
	// the textChanged() signal from QLineEdit.
	txtRomFile->setText(filename);
}

void GeneralConfigWindow::on_btnMcdRomUSA_clicked(void)
	{ mcdSelectRomFile(tr("Sega CD (U)"), txtMcdRomUSA); }
void GeneralConfigWindow::on_btnMcdRomEUR_clicked(void)
	{ mcdSelectRomFile(tr("Mega CD (E)"), txtMcdRomEUR); }
void GeneralConfigWindow::on_btnMcdRomJPN_clicked(void)
	{ mcdSelectRomFile(tr("Mega CD (J)"), txtMcdRomJPN); }


/**
 * mcdUpdateRomFileStatus(): Sega CD: Update Boot ROM file status.
 * @param txtRomFile ROM file textbox.
 * @param region_code Expected ROM region code. (bitmask)
 * @return Updated ROM status.
 */
QString GeneralConfigWindow::mcdUpdateRomFileStatus(GensLineEdit *txtRomFile, int region_code)
{
	// ROM data buffer.
	uint8_t *rom_data = NULL;
	qint64 data_len;
	uint32_t rom_crc32;
	int boot_rom_id;
	int boot_rom_region_code;
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
			return tr("No ROM filename specified.");
		else
			return tr("The specified ROM file was not found.");
	}
	
	// Line break string.
	const QString sLineBreak = QString::fromLatin1("<br/>");
	
	// Check the ROM file.
	// TODO: Decompressor support.
	QStyle::StandardPixmap filename_icon = QStyle::SP_DialogYesButton;
	QString rom_id = tr("Unknown");
	QString rom_notes;
	QString rom_size_warning;
	
	// Check the ROM filesize.
	// Valid boot ROMs are 128 KB.
	// Smaller ROMs will not work; larger ROMs may work, but warn anyway.
	if (file.size() != MCD_ROM_FILESIZE)
	{
		// Wrong ROM size.
		filename_icon = QStyle::SP_MessageBoxWarning;
		
		rom_size_warning = ms_sWarning + tr("ROM size is incorrect.") + sLineBreak +
				   tr("(expected %L1 bytes; found %L2 bytes)").arg(MCD_ROM_FILESIZE).arg(file.size());
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
		rom_notes = tr("Error opening file: %1").arg(file.error());
		goto rom_identified;
	}
	
	// Read 128 KB and calculate the CRC32.
	rom_data = (uint8_t*)malloc(MCD_ROM_FILESIZE);
	data_len = file.read((char*)rom_data, MCD_ROM_FILESIZE);
	if (data_len != MCD_ROM_FILESIZE)
	{
		// Error reading data from the file.
		rom_notes = tr("Error reading file.") + sLineBreak +
			    tr("(expected %L1 bytes; read %L2 bytes)").arg(MCD_ROM_FILESIZE).arg(data_len);
		goto rom_identified;
	}
	
	// Fix up the ROM's Initial SP and Initial HINT vector.
	memcpy(&rom_data[0x00], lg_mcd_rom_InitSP, sizeof(lg_mcd_rom_InitSP));
	memcpy(&rom_data[0x70], lg_mcd_rom_InitHINT, sizeof(lg_mcd_rom_InitHINT));
	
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
	if ((boot_rom_region_code & region_code) == 0)
	{
		// ROM doesn't support this region.
		int boot_rom_region_primary = lg_mcd_rom_GetPrimaryRegion(boot_rom_id);
		QString expected_region = QString::fromUtf8(lg_mcd_rom_GetRegionCodeString(region_code));
		QString boot_rom_region = QString::fromUtf8(lg_mcd_rom_GetRegionCodeString(boot_rom_region_primary));
		
		rom_notes += ms_sWarning + tr("Region code is incorrect.") + sLineBreak +
			     tr("(expected %1; found %2)").arg(expected_region).arg(boot_rom_region) + sLineBreak;
		
		// Set the icon to warning.
		filename_icon = QStyle::SP_MessageBoxWarning;
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
			rom_notes += ms_sWarning + tr("This Boot ROM is not supported by Gens/GS II.") + sLineBreak;
			filename_icon = QStyle::SP_MessageBoxWarning;
			break;
		
		case RomStatus_Broken:
			// ROM is known to be broken.
			rom_notes += ms_sWarning + tr("This Boot ROM is known to be broken on all emulators.") + sLineBreak;
			filename_icon = QStyle::SP_MessageBoxCritical;
			break;
	}
	
	// Get the ROM's notes.
	rom_notes += QString::fromUtf8(lg_mcd_rom_GetNotes(boot_rom_id)).replace(QChar(L'\n'), sLineBreak);
	
rom_identified:
	// Free the ROM data buffer if it was allocated.
	free(rom_data);
	
	// Set the Boot ROM filename textbox icon.
	txtRomFile->setIcon(style()->standardIcon(filename_icon));
	
	// Set the Boot ROM description.
	QString s_ret;
	s_ret = tr("ROM identified as: %1").arg(rom_id);
	if (!rom_notes.isEmpty())
		s_ret += sLineBreak + sLineBreak + rom_notes;
	if (!rom_size_warning.isEmpty())
		s_ret += sLineBreak + sLineBreak + rom_size_warning;
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
	QString sel_rom = tr("Selected ROM: %1");
	lblMcdSelectedRom->setText(sel_rom.arg(rom_id) +
				QString::fromLatin1("<br/>\n") + rom_desc);
	lblMcdSelectedRom->setTextFormat(Qt::RichText);
}

void GeneralConfigWindow::on_txtMcdRomUSA_focusIn(void)
	{ mcdDisplayRomFileStatus(tr("Sega CD (U)"), sMcdRomStatus_USA); }
void GeneralConfigWindow::on_txtMcdRomEUR_focusIn(void)
	{ mcdDisplayRomFileStatus(tr("Mega CD (E)"), sMcdRomStatus_EUR); }
void GeneralConfigWindow::on_txtMcdRomJPN_focusIn(void)
	{ mcdDisplayRomFileStatus(tr("Mega CD (J)"), sMcdRomStatus_JPN); }

void GeneralConfigWindow::on_txtMcdRomUSA_textChanged(void)
{
	setApplyButtonEnabled(true);
	
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomUSA, MCD_REGION_USA);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_USA = sNewRomStatus;
		mcdDisplayRomFileStatus(tr("Sega CD (U)"), sMcdRomStatus_USA);
	}
}
void GeneralConfigWindow::on_txtMcdRomEUR_textChanged(void)
{
	setApplyButtonEnabled(true);
	
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomEUR, MCD_REGION_EUROPE);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_EUR = sNewRomStatus;
		mcdDisplayRomFileStatus(tr("Mega CD (E)"), sMcdRomStatus_EUR);
	}
}
void GeneralConfigWindow::on_txtMcdRomJPN_textChanged(void)
{
	setApplyButtonEnabled(true);
	
	QString sNewRomStatus = mcdUpdateRomFileStatus(txtMcdRomJPN, MCD_REGION_JAPAN_NTSC | MCD_REGION_JAPAN_PAL);
	if (!sNewRomStatus.isEmpty())
	{
		sMcdRomStatus_JPN = sNewRomStatus;
		mcdDisplayRomFileStatus(tr("Mega CD (J)"), sMcdRomStatus_JPN);
	}
}


/** External programs **/


/**
 * on_btnExtPrgUnRAR_clicked(): Select a RAR/UnRAR binary.
 */
void GeneralConfigWindow::on_btnExtPrgUnRAR_clicked(void)
{
	// Create the dialog title.
#ifdef _WIN32
	const QString title = tr("Select UnRAR DLL");
#else
	const QString title = tr("Select RAR or UnRAR binary");
#endif
	
	QString filename = QFileDialog::getOpenFileName(this, title,
			txtExtPrgUnRAR->text(),		// Default filename.
#ifdef _WIN32
			tr("DLL files") + QString::fromLatin1(" (*.dll);;")
#else
			tr("rar or unrar") + QString::fromLatin1(" (rar unrar);;")
#endif
			+ tr("All Files") + QString::fromLatin1("(*.*)"));
	
	if (filename.isEmpty())
		return;
	
	// Convert to native pathname separators.
	filename = QDir::toNativeSeparators(filename);
	
	// Set the filename text.
	// Program file status will be updated automatically by
	// the textChanged() signal from QLineEdit.
	txtExtPrgUnRAR->setText(filename);
}


/**
 * extprgDisplayFileStatus(): Display external program file status.
 * @param file_id File ID.
 * @param file_desc File description. (detected by examining the file)
 */
void GeneralConfigWindow::extprgDisplayFileStatus(const QString& file_id, const QString& file_desc)
{
	// Set the file description.
	QString sel_prg = tr("Selected Program: %1");
	lblExtPrgSel->setText(sel_prg.arg(file_id) +
				QString::fromLatin1("<br/>\n") + file_desc);
	lblExtPrgSel->setTextFormat(Qt::RichText);
}

void GeneralConfigWindow::on_txtExtPrgUnRAR_focusIn(void)
{
#ifdef _WIN32
	extprgDisplayFileStatus(tr("UnRAR DLL"), sExtPrgStatus_UnRAR);
#else
	extprgDisplayFileStatus(tr("RAR or UnRAR binary"), sExtPrgStatus_UnRAR);
#endif
}

void GeneralConfigWindow::on_txtExtPrgUnRAR_textChanged(void)
{
	setApplyButtonEnabled(true);
	
	// Check the RAR binary to make sure it's valid.
	// TODO: Split status detection into a separate function like Sega CD Boot ROMs?
	QString prg_id = tr("Unknown");
	QString prg_status;
	QStyle::StandardPixmap filename_icon = QStyle::SP_MessageBoxQuestion;
	LibGens::DcRar::ExtPrgInfo prg_info;
	
	// Check if the file exists.
	const QString& filename = txtExtPrgUnRAR->text();
	if (filename.isEmpty())
		prg_status = tr("No filename specified.");
	else
	{
		int status = LibGens::DcRar::CheckExtPrg(txtExtPrgUnRAR->text().toUtf8().constData(), &prg_info);
		switch (status)
		{
			case 0:
				// RAR is usable.
				filename_icon = QStyle::SP_DialogYesButton;
				break;
			
			case -1:
				// RAR not found.
				prg_status = tr("The specified file was not found.");
				break;
			
			case -2:
				// RAR not executable.
				prg_status = ms_sWarning + tr("The specified file is not executable.");
				filename_icon = QStyle::SP_MessageBoxWarning;
				break;
			
			case -3:
				// File isn't a regular file.
				prg_status = ms_sWarning + tr("The specified file is not a regular file.");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
			
			case -4:
				// Error calling stat().
				// TODO: Get the stat() error code?
				prg_status = ms_sWarning + tr("Error calling stat().");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
			
#ifdef _WIN32
			case -5:
				// UnRAR.dll API version is too old. (Win32 only)
				prg_status = ms_sWarning + tr("UnRAR.dll API version is too old.") + "<br/>\n" +
							   tr("Gens/GS II requires API version %1 or later.").arg(RAR_DLL_VERSION);
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
#endif
			
			case -6:
				// Version information not found.
				prg_status = ms_sWarning + tr("DLL version information not found.");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
			
			case -7:
				// Not UnRAR.dll. (Win32 only)
				prg_status = ms_sWarning + tr("Selected DLL is not UnRAR.dll.");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
			
			default:
				// Unknown error.
				prg_status = ms_sWarning + tr("Unknown error code %1 received from RAR file handler.").arg(status);
				filename_icon = QStyle::SP_MessageBoxWarning;
				break;
		}
	}
	
	// Set program ID.
	if (prg_info.dll_major != 0 || prg_info.dll_minor != 0 ||
	    prg_info.dll_revision != 0 || prg_info.dll_build != 0)
	{
		switch (prg_info.rar_type)
		{
			case LibGens::DcRar::ExtPrgInfo::RAR_ET_UNKNOWN:
			default:
				break;
			
			case LibGens::DcRar::ExtPrgInfo::RAR_ET_UNRAR:
				prg_id = tr("UnRAR");
				break;
			
			case LibGens::DcRar::ExtPrgInfo::RAR_ET_RAR:
				prg_id = tr("RAR");
				break;
			
			case LibGens::DcRar::ExtPrgInfo::RAR_ET_UNRAR_DLL:
				prg_id = tr("UnRAR.dll");
				break;
		}
	}
	sExtPrgStatus_UnRAR = tr("Identified as: %1").arg(prg_id);
	
	// Line break string.
	const QString sLineBreak = QString::fromLatin1("<br/>");
	
	// Print DLL version information, if available.
	if (prg_info.dll_major != 0 || prg_info.dll_minor != 0 ||
	    prg_info.dll_revision != 0 || prg_info.dll_build != 0)
	{
		QString rar_version;
#ifdef _WIN32
		rar_version = tr("%1 version %2.%3.%4.%5");
		rar_version = rar_version.arg(prg_id);
		rar_version = rar_version.arg(prg_info.dll_major);
		rar_version = rar_version.arg(prg_info.dll_minor);
		rar_version = rar_version.arg(prg_info.dll_revision);
		rar_version = rar_version.arg(prg_info.dll_build);
#else
		rar_version = tr("%1 version %2.%3");
		rar_version = rar_version.arg(prg_id);
		rar_version = rar_version.arg(prg_info.dll_major);
		rar_version = rar_version.arg(prg_info.dll_minor);
#endif
		sExtPrgStatus_UnRAR += sLineBreak + sLineBreak + rar_version;
#ifdef _WIN32
		if (prg_info.api_version > 0)
			sExtPrgStatus_UnRAR += sLineBreak + tr("API version %1").arg(prg_info.api_version);
#endif
	}
	
	if (!prg_status.isEmpty())
		sExtPrgStatus_UnRAR += sLineBreak + sLineBreak + prg_status;
	
	// Set the textbox's icon.
	txtExtPrgUnRAR->setIcon(style()->standardIcon(filename_icon));
	
	// TODO: Create a constant string for DLL vs. binary.
	// For now, just call focusIn() to update the description.
	on_txtExtPrgUnRAR_focusIn();
}

}
