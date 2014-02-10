/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GeneralConfigWindow.cpp: General Configuration Window.                     *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2014 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include "GeneralConfigWindow.hpp"
#include "GeneralConfigWindow_p.hpp"
#include "gqt4_main.hpp"
#include "GensQApplication.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cmath>
#include <cstdlib>

// zlib
#include <zlib.h>

// Qt4 includes.
#include <QtGui/QFileDialog>
#include <QtGui/QKeyEvent>

// libgens: RAR decompressor
#include "libgens/Decompressor/DcRar.hpp"

// EmuManager is needed for region code strings.
#include "EmuManager.hpp"

namespace GensQt4
{

/**
 * Initialize the General Configuration window.
 */
GeneralConfigWindow::GeneralConfigWindow(QWidget *parent)
	: QMainWindow(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new GeneralConfigWindowPrivate(this))
{
	// Initialize the Qt4 UI.
	setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	Q_D(GeneralConfigWindow);
	if (!d->isWarrantyVoid()) {
		// Hide the super secret settings.
		delete grpAdvancedVDP;
		grpAdvancedVDP = nullptr;
	}

	// Set up the action group for the toolbar buttons.
	d->actgrpToolbar->setExclusive(true);
	connect(d->actgrpToolbar, SIGNAL(triggered(QAction*)),
		this, SLOT(toolbarTriggered(QAction*)));

	// FreeDesktop.org icon names for the toolbar buttons.
	static const char *const icon_fdo[] = {
		"configure",			// General
		"applications-graphics",	// Graphics
		"cpu",				// VDP
		"applications-system",		// System
		"",				// Genesis (TODO)
		"media-optical",		// Sega CD
		"utilities-terminal",		// External Programs
		nullptr
	};

	// Initialize the toolbar buttons.
	int i = 0;
	foreach (QAction *action, toolBar->actions()) {
		action->setIcon(GensQApplication::IconFromTheme(QLatin1String(icon_fdo[i])));
		action->setData(i);
		d->actgrpToolbar->addAction(action);

		// Next action.
		i++;
	}

	// Select the "General" tab.
	actionGeneral->setChecked(true);

#ifndef GCW_APPLY_IMMED
	// Set up a signal for the Apply button.
	QPushButton *btnApply = buttonBox->button(QDialogButtonBox::Apply);
	if (btnApply)
		connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
#else
	{
		// Apply settings immediately.
		// Remove the buttonBox.
		QSize szBbox = buttonBox->size();
		delete buttonBox;
		buttonBox = nullptr;

		// Reduce the size of the QMainWindow.
		QSize szWindow = this->maximumSize();
		int left, top, right, bottom;
		vboxDialog->getContentsMargins(&left, &top, &right, &bottom);
		szWindow.rheight() -= (szBbox.height() + ((top + bottom) / 2));
		this->setMinimumSize(szWindow);
		this->setMaximumSize(szWindow);
		this->setBaseSize(szWindow);
	}
#endif

#ifdef Q_WS_MAC
	// Set up the Mac OS X-specific UI elements.
	d->setupUi_mac();
#endif

	/** Intro effect. **/

	// Intro Effect Color: Add the color entries.
	cboIntroColor->addItem(Qt::black);
	cboIntroColor->addItem(Qt::blue);
	cboIntroColor->addItem(Qt::green);
	cboIntroColor->addItem(Qt::cyan);
	cboIntroColor->addItem(Qt::red);
	cboIntroColor->addItem(Qt::magenta);
	cboIntroColor->addItem(Qt::yellow);
	cboIntroColor->addItem(Qt::white);

	/** System. **/

	// Set the icons for the up/down buttons.
	btnRegionDetectUp->setIcon(GensQApplication::IconFromTheme(QLatin1String("arrow-up")));
	btnRegionDetectDown->setIcon(GensQApplication::IconFromTheme(QLatin1String("arrow-down")));

	/** Sega Genesis. **/

	// Sega Genesis: TMSS ROM.
	txtMDTMSSRom->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	const QString sMDTMSSRom_PlaceholderText = tr("Select a %1 TMSS ROM...");
	txtMDTMSSRom->setPlaceholderText(sMDTMSSRom_PlaceholderText.arg(tr("Genesis")));

	/** Sega CD. **/

	// Sega CD: Initialize the Boot ROM textbox icons.
	txtMcdRomUSA->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	txtMcdRomEUR->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	txtMcdRomJPN->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	txtMcdRomAsia->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));

	// Sega CD: Set the placeholder text.
#if QT_VERSION >= 0x040700
	const QString sMcdBootRom_PlaceholderText = tr("Select a %1 Boot ROM...");
	txtMcdRomUSA->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Sega CD (U)")));
	txtMcdRomEUR->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Mega CD (E)")));
	txtMcdRomJPN->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Mega CD (J)")));
	txtMcdRomAsia->setPlaceholderText(sMcdBootRom_PlaceholderText.arg(tr("Mega CD (Asia)")));
#endif /* QT_VERSION >= 0x040700 */

	// External Programs: Set the textbox icon and placeholder text.
	txtExtPrgUnRAR->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
#ifdef Q_OS_WIN32
	lblExtPrgUnRAR->setText(tr("UnRAR DLL:"));
#if QT_VERSION >= 0x040700
	txtExtPrgUnRAR->setPlaceholderText(tr("Select an UnRAR DLL..."));
#endif /* QT_VERSION >= 0x040700 */
#else /* !Q_OS_WIN32 */
#if QT_VERSION >= 0x040700
	txtExtPrgUnRAR->setPlaceholderText(tr("Select a RAR or UnRAR binary..."));
#endif /* QT_VERSION >= 0x040700 */
#endif /* Q_OS_WIN32 */

	// Load configuration.
	reload();
}


/**
 * Shut down the Controller Configuration window.
 */
GeneralConfigWindow::~GeneralConfigWindow()
{
	delete d_ptr;

	// Clear the ms_GeneralConfigWindow pointer.
	GeneralConfigWindowPrivate::ms_GeneralConfigWindow = nullptr;
}

/**
 * Show a single instance of the General Configuration window.
 * @param parent Parent window.
 */
void GeneralConfigWindow::ShowSingle(QWidget *parent)
{
	if (GeneralConfigWindowPrivate::ms_GeneralConfigWindow != nullptr) {
		// General Configuration Window is already displayed.
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(GeneralConfigWindowPrivate::ms_GeneralConfigWindow);
	} else {
		// General Configuration Window is not displayed.
		GeneralConfigWindowPrivate::ms_GeneralConfigWindow = new GeneralConfigWindow(parent);
		GeneralConfigWindowPrivate::ms_GeneralConfigWindow->show();
	}
}

/**
 * Key press handler.
 * @param event Key event.
 */
void GeneralConfigWindow::keyPressEvent(QKeyEvent *event)
{
	// TODO: Handle Cmd-Period on Mac?
	// NOTE: Cmd-W triggers the "Close ROM" action...
#ifndef GCW_APPLY_IMMED
	// Changes are not applied immediately.
	// Check for special dialog keys.
	// Adapted from QDialog::keyPressEvent().

	if (!event->modifiers() || ((event->modifiers() & Qt::KeypadModifier) && event->key() == Qt::Key_Enter))
	{
		switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
				// Accept the dialog changes.
				accept();
				break;

			case Qt::Key_Escape:
				// Reject the dialog changes.
				reject();
				break;

			default:
				// Pass the event to the base class.
				this->QMainWindow::keyPressEvent(event);
				return;
		}
	} else {
		// Pass the event to the base class.
		this->QMainWindow::keyPressEvent(event);
	}
#else /* !GCW_APPLY_IMMED */
	// Changes are applied immediately.
	// Don't handle special dialog keys.
	Q_UNUSED(event)
	return;
#endif /* GCW_APPLY_IMMED */
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void GeneralConfigWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		retranslateUi(this);

		// Update the ROM file status.
		// TODO: Update labels?
		Q_D(GeneralConfigWindow);
		d->updateRomFileStatus();

		// Update external program status.
		// TODO: Split the RAR check code out of the on_txtExtPrgUnRAR_textChanged() function.
		// NOTE: Calling on_txtExtPrgUnRAR_textChanged() will enable the Apply button!
		on_txtExtPrgUnRAR_textChanged();
	}

	// Pass the event to the base class.
	QMainWindow::changeEvent(event);
}

/**
 * Accept the configuration changes.
 * Triggered if "OK" is clicked.
 */
void GeneralConfigWindow::accept(void)
{
	apply();
	this->close();
}

/**
 * Reject the configuration changes.
 * Triggered if "Cancel" is clicked.
 */
void GeneralConfigWindow::reject(void)
{
	this->close();
}

static inline bool ValByPath_bool(const char *path)
	{ return gqt4_cfg->get(QLatin1String(path)).toBool(); }
static inline QColor ValByPath_QColor(const char *path)
	{ return gqt4_cfg->get(QLatin1String(path)).value<QColor>(); }
static inline int ValByPath_int(const char *path)
	{ return gqt4_cfg->getInt(QLatin1String(path)); }
static inline int ValByPath_uint(const char *path)
	{ return gqt4_cfg->getUInt(QLatin1String(path)); }
static inline QString ValByPath_QString(const char *path)
	{ return gqt4_cfg->get(QLatin1String(path)).toString(); }

/**
 * Reload configuration.
 */
void GeneralConfigWindow::reload(void)
{
	Q_D(GeneralConfigWindow);

	// Onscreen Display.
	QColor colorText;

	/** Onscreen display: FPS counter. **/
	chkOsdFpsEnable->setChecked(ValByPath_bool("OSD/fpsEnabled"));
	d->osdFpsColor = ValByPath_QColor("OSD/fpsColor");
	btnOsdFpsColor->setBgColor(d->osdFpsColor);
	btnOsdFpsColor->setText(d->osdFpsColor.name().toUpper());

	/** Onscreen display: Messages. **/
	chkOsdMsgEnable->setChecked(ValByPath_bool("OSD/msgEnabled"));
	d->osdMsgColor = ValByPath_QColor("OSD/msgColor");
	btnOsdMsgColor->setBgColor(d->osdMsgColor);
	btnOsdMsgColor->setText(d->osdMsgColor.name().toUpper());

	/** General settings. **/
	chkAutoFixChecksum->setChecked(ValByPath_bool("autoFixChecksum"));
	chkAutoPause->setChecked(ValByPath_bool("autoPause"));
	chkPauseTint->setChecked(ValByPath_bool("pauseTint"));

	/** Intro effect. **/
	cboIntroStyle->setCurrentIndex(ValByPath_int("Intro_Effect/introStyle"));
	cboIntroColor->setCurrentIndex(ValByPath_int("Intro_Effect/introColor"));

	/** Sega Genesis TMSS. **/
	chkMDTMSS->setChecked(ValByPath_bool("Genesis/tmssEnabled"));
	txtMDTMSSRom->setText(ValByPath_QString("Genesis/tmssRom"));

	/** Sega CD Boot ROMs. **/
	txtMcdRomUSA->setText(ValByPath_QString("Sega_CD/bootRomUSA"));
	txtMcdRomEUR->setText(ValByPath_QString("Sega_CD/bootRomEUR"));
	txtMcdRomJPN->setText(ValByPath_QString("Sega_CD/bootRomJPN"));
	txtMcdRomAsia->setText(ValByPath_QString("Sega_CD/bootRomAsia"));
	on_txtMcdRomUSA_focusIn();

	/** External programs. **/
	txtExtPrgUnRAR->setText(ValByPath_QString("External_Programs/UnRAR"));

	/** Graphics settings. **/
	chkAspectRatioConstraint->setChecked(ValByPath_bool("Graphics/aspectRatioConstraint"));
	chkFastBlur->setChecked(ValByPath_bool("Graphics/fastBlur"));
	chkBilinearFilter->setChecked(ValByPath_bool("Graphics/bilinearFilter"));
	cboInterlacedMode->setCurrentIndex(ValByPath_int("Graphics/interlacedMode"));

	/** VDP settings. **/
	chkSpriteLimits->setChecked(ValByPath_bool("VDP/spriteLimits"));
	chkBorderColor->setChecked(ValByPath_bool("VDP/borderColorEmulation"));
	chkNtscV30Rolling->setChecked(ValByPath_bool("VDP/ntscV30Rolling"));
	if (d->isWarrantyVoid()) {
		chkVScrollBug->setChecked(ValByPath_bool("VDP/vscrollBug"));
		chkZeroLengthDMA->setChecked(ValByPath_bool("VDP/zeroLengthDMA"));
	}

	/** System. **/
	cboRegionCurrent->setCurrentIndex(ValByPath_int("System/regionCode") + 1);

	// Region auto-detection settings.
	lstRegionDetect->clear();
	uint16_t regionCodeOrder = (uint16_t)ValByPath_uint("System/regionCodeOrder");
	for (int i = 0; i < 4; i++, regionCodeOrder >>= 4) {
		const QString str = EmuManager::LgRegionCodeStrMD(regionCodeOrder & 0xF);
		if (!str.isEmpty()) {
			QListWidgetItem *item = new QListWidgetItem(str);
			item->setData(Qt::UserRole, (regionCodeOrder & 0xF));
			lstRegionDetect->insertItem(0, item);
		}
	}

#ifndef GCW_APPLY_IMMED
	// Disable the Apply button.
	d->setApplyButtonEnabled(false);
#endif
}

static inline void SetValByPath_bool(const char *path, bool value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_QColor(const char *path, const QColor& value)
	{ gqt4_cfg->set(QLatin1String(path), value.name()); }
static inline void SetValByPath_int(const char *path, int value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_uint(const char *path, unsigned int value)
	{ gqt4_cfg->set(QLatin1String(path), value); }
static inline void SetValByPath_QString(const char *path, QString value)
	{ gqt4_cfg->set(QLatin1String(path), value); }

/**
 * Apply the configuration changes.
 * Triggered if "Apply" is clicked.
 */
void GeneralConfigWindow::apply(void)
{
#ifndef GCW_APPLY_IMMED
	Q_D(GeneralConfigWindow);

	/** Onscreen display. **/
	SetValByPath_bool("OSD/fpsEnabled", chkOsdFpsEnable->isChecked());
	SetValByPath_QColor("OSD/fpsColor", d->osdFpsColor);
	SetValByPath_bool("OSD/msgEnabled", chkOsdMsgEnable->isChecked());
	SetValByPath_QColor("OSD/msgColor", d->osdMsgColor);

	/** Intro effect. **/
	SetValByPath_int("Intro_Effect/introStyle", cboIntroStyle->currentIndex());
	SetValByPath_int("Intro_Effect/introColor", cboIntroColor->currentIndex());

	/** General settings. **/
	SetValByPath_bool("autoFixChecksum", chkAutoFixChecksum->isChecked());
	SetValByPath_bool("autoPause", chkAutoPause->isChecked());
	SetValByPath_bool("pauseTint", chkPauseTint->isChecked());

	/** Sega Genesis TMSS. **/
	SetValByPath_bool("Genesis/tmssEnabled", chkMDTMSS->isChecked());
	SetValByPath_QString("Genesis/tmssRom", txtMDTMSSRom->text());

	/** Sega CD Boot ROMs. **/
	SetValByPath_QString("Sega_CD/bootRomUSA", txtMcdRomUSA->text());
	SetValByPath_QString("Sega_CD/bootRomEUR", txtMcdRomEUR->text());
	SetValByPath_QString("Sega_CD/bootRomJPN", txtMcdRomJPN->text());
	SetValByPath_QString("Sega_CD/bootRomAsia", txtMcdRomAsia->text());

	/** External programs. **/
	SetValByPath_QString("External_Programs/UnRAR", txtExtPrgUnRAR->text());

	/** Graphics settings. **/
	SetValByPath_bool("Graphics/aspectRatioConstraint", chkAspectRatioConstraint->isChecked());
	SetValByPath_bool("Graphics/fastBlur", chkFastBlur->isChecked());
	SetValByPath_bool("Graphics/bilinearFilter", chkBilinearFilter->isChecked());
	SetValByPath_int("Graphics/interlacedMode", cboInterlacedMode->currentIndex());

	/** VDP settings. **/
	SetValByPath_bool("VDP/spriteLimits", chkSpriteLimits->isChecked());
	SetValByPath_bool("VDP/borderColorEmulation", chkBorderColor->isChecked());
	SetValByPath_bool("VDP/ntscV30Rolling", chkNtscV30Rolling->isChecked());
	if (d->isWarrantyVoid()) {
		SetValByPath_bool("VDP/vscrollBug", chkVScrollBug->isChecked());
		SetValByPath_bool("VDP/zeroLengthDMA", chkZeroLengthDMA->isChecked());
	}

	/** System. **/
	SetValByPath_int("System/regionCode", (cboRegionCurrent->currentIndex() - 1));
	SetValByPath_uint("System/regionCodeOrder", d->regionCodeOrder());

	// Disable the Apply button.
	// TODO: If Apply was clicked, set focus back to the main window elements.
	// Otherwise, Cancel will receive focus.
	d->setApplyButtonEnabled(false);
#endif
}

/**
 * A toolbar button was clicked.
 * @param action Toolbar button.
 */
void GeneralConfigWindow::toolbarTriggered(QAction *action)
{
	QVariant data = action->data();
	if (!data.isValid() || !data.canConvert(QVariant::Int))
		return;
	
	int tab = data.toInt();
	if (tab < 0 || tab >= stackedWidget->count())
		return;
	
	stackedWidget->setCurrentIndex(tab);
}

/** Onscreen display **/

void GeneralConfigWindow::on_btnOsdFpsColor_clicked(void)
{
	Q_D(GeneralConfigWindow);
	QColor color = d->osdSelectColor(tr("FPS counter"), d->osdFpsColor);
	if (!color.isValid() || color == d->osdFpsColor)
		return;

	d->osdFpsColor = color;
	btnOsdFpsColor->setBgColor(d->osdFpsColor);
	btnOsdFpsColor->setText(d->osdFpsColor.name().toUpper());

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QColor("OSD/fpsColor", d->osdMsgColor);
#endif
}

void GeneralConfigWindow::on_btnOsdMsgColor_clicked(void)
{
	Q_D(GeneralConfigWindow);
	QColor color = d->osdSelectColor(tr("messages"), d->osdMsgColor);
	if (!color.isValid() || color == d->osdMsgColor)
		return;

	d->osdMsgColor = color;
	btnOsdMsgColor->setBgColor(color);
	btnOsdMsgColor->setText(d->osdMsgColor.name().toUpper());

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QColor("OSD/msgColor", d->osdMsgColor);
#endif
}

/** System. **/
// TODO: Detect drag-and-drop of items within the QListWidget.

/**
 * Region Code Order: Up button was clicked.
 */
void GeneralConfigWindow::on_btnRegionDetectUp_clicked(void)
{
	QListWidgetItem *cur = lstRegionDetect->currentItem();
	if (!cur)
		return;
	int cur_idx = lstRegionDetect->row(cur);

	QListWidgetItem *prev = lstRegionDetect->takeItem(cur_idx - 1);
	if (!prev)
		return;

	lstRegionDetect->insertItem(cur_idx, prev);

	// Settings have been changed.
	Q_D(GeneralConfigWindow);
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_uint("System/regionCodeOrder", d->regionCodeOrder());
#endif
}

/**
 * Region Code Order: Down button was clicked.
 */
void GeneralConfigWindow::on_btnRegionDetectDown_clicked(void)
{
	QListWidgetItem *cur = lstRegionDetect->currentItem();
	if (!cur)
		return;
	int cur_idx = lstRegionDetect->row(cur);

	QListWidgetItem *next = lstRegionDetect->takeItem(cur_idx + 1);
	if (!next)
		return;

	lstRegionDetect->insertItem(cur_idx, next);

	// Settings have been changed.
	Q_D(GeneralConfigWindow);
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_uint("System/regionCodeOrder", d->regionCodeOrder());
#endif
}

/** Sega Genesis. **/

void GeneralConfigWindow::on_btnMDTMSSRom_clicked(void)
{
	Q_D(GeneralConfigWindow);
	const QString tmssRom = tr("%1 TMSS ROM");
	d->selectRomFile(tmssRom.arg(tr("Genesis")), txtMDTMSSRom);
}

void GeneralConfigWindow::on_txtMDTMSSRom_focusIn(void)
{
	// TODO
	//mcdDisplayRomFileStatus(tr("Sega CD (U)"), sMcdRomStatus_USA);
}

void GeneralConfigWindow::on_txtMDTMSSRom_textChanged(void)
{
	Q_D(GeneralConfigWindow);
	QString sNewRomStatus = d->mdUpdateTmssRomFileStatus(txtMDTMSSRom);
	if (!sNewRomStatus.isEmpty()) {
		d->sMDTmssRomStatus = sNewRomStatus;
		lblMDTMSSSelectedRom->setText(d->sMDTmssRomStatus);
		lblMDTMSSSelectedRom->setTextFormat(Qt::RichText);
	}

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("Genesis/tmssRom", txtMDTMSSRom->text());
#endif
}

/** Sega CD. **/

void GeneralConfigWindow::on_btnMcdRomUSA_clicked(void)
{
	Q_D(GeneralConfigWindow);
	const QString bootRom = tr("%1 Boot ROM");
	d->selectRomFile(bootRom.arg(tr("Sega CD (U)")), txtMcdRomUSA);
}
void GeneralConfigWindow::on_btnMcdRomEUR_clicked(void)
{
	Q_D(GeneralConfigWindow);
	const QString bootRom = tr("%1 Boot ROM");
	d->selectRomFile(bootRom.arg(tr("Mega CD (E)")), txtMcdRomEUR);
}
void GeneralConfigWindow::on_btnMcdRomJPN_clicked(void)
{
	Q_D(GeneralConfigWindow);
	const QString bootRom = tr("%1 Boot ROM");
	d->selectRomFile(bootRom.arg(tr("Mega CD (J)")), txtMcdRomJPN);
}
void GeneralConfigWindow::on_btnMcdRomAsia_clicked(void)
{
	Q_D(GeneralConfigWindow);
	const QString bootRom = tr("%1 Boot ROM");
	d->selectRomFile(bootRom.arg(tr("Mega CD (Asia)")), txtMcdRomAsia);
}

void GeneralConfigWindow::on_txtMcdRomUSA_focusIn(void)
{
	Q_D(GeneralConfigWindow);
	d->mcdDisplayRomFileStatus(tr("Sega CD (U)"), d->sMcdRomStatus_USA);
}
void GeneralConfigWindow::on_txtMcdRomEUR_focusIn(void)
{
	Q_D(GeneralConfigWindow);
	d->mcdDisplayRomFileStatus(tr("Mega CD (E)"), d->sMcdRomStatus_EUR);
}
void GeneralConfigWindow::on_txtMcdRomJPN_focusIn(void)
{
	Q_D(GeneralConfigWindow);
	d->mcdDisplayRomFileStatus(tr("Mega CD (J)"), d->sMcdRomStatus_JPN);
}
void GeneralConfigWindow::on_txtMcdRomAsia_focusIn(void)
{
	Q_D(GeneralConfigWindow);
	d->mcdDisplayRomFileStatus(tr("Mega CD (Asia)"), d->sMcdRomStatus_Asia);
}

void GeneralConfigWindow::on_txtMcdRomUSA_textChanged(void)
{
	Q_D(GeneralConfigWindow);
	QString sNewRomStatus = d->mcdUpdateRomFileStatus(txtMcdRomUSA, MCD_REGION_USA);
	if (!sNewRomStatus.isEmpty()) {
		d->sMcdRomStatus_USA = sNewRomStatus;
		d->mcdDisplayRomFileStatus(tr("Sega CD (U)"), d->sMcdRomStatus_USA);
	}

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("Sega_CD/bootRomUSA", txtMcdRomUSA->text());
#endif
}

void GeneralConfigWindow::on_txtMcdRomEUR_textChanged(void)
{
	Q_D(GeneralConfigWindow);
	QString sNewRomStatus = d->mcdUpdateRomFileStatus(txtMcdRomEUR, MCD_REGION_EUROPE);
	if (!sNewRomStatus.isEmpty()) {
		d->sMcdRomStatus_EUR = sNewRomStatus;
		d->mcdDisplayRomFileStatus(tr("Mega CD (E)"), d->sMcdRomStatus_EUR);
	}

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("Sega_CD/bootRomEUR", txtMcdRomEUR->text());
#endif
}

void GeneralConfigWindow::on_txtMcdRomJPN_textChanged(void)
{
	Q_D(GeneralConfigWindow);
	QString sNewRomStatus = d->mcdUpdateRomFileStatus(txtMcdRomJPN, MCD_REGION_JAPAN);
	if (!sNewRomStatus.isEmpty()) {
		d->sMcdRomStatus_JPN = sNewRomStatus;
		d->mcdDisplayRomFileStatus(tr("Mega CD (J)"), d->sMcdRomStatus_JPN);
	}

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("Sega_CD/bootRomJPN", txtMcdRomJPN->text());
#endif
}

void GeneralConfigWindow::on_txtMcdRomAsia_textChanged(void)
{
	Q_D(GeneralConfigWindow);
	QString sNewRomStatus = d->mcdUpdateRomFileStatus(txtMcdRomAsia, MCD_REGION_ASIA);
	if (!sNewRomStatus.isEmpty()) {
		d->sMcdRomStatus_Asia = sNewRomStatus;
		d->mcdDisplayRomFileStatus(tr("Mega CD (Asia)"), d->sMcdRomStatus_Asia);
	}

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("Sega_CD/bootRomAsia", txtMcdRomAsia->text());
#endif
}

/** External programs **/

/**
 * Select a RAR/UnRAR binary.
 */
void GeneralConfigWindow::on_btnExtPrgUnRAR_clicked(void)
{
	// Create the dialog title.
#ifdef Q_OS_WIN32
	const QString title = tr("Select UnRAR DLL");
#else
	const QString title = tr("Select RAR or UnRAR binary");
#endif

	QString filename = QFileDialog::getOpenFileName(this, title,
			txtExtPrgUnRAR->text(),		// Default filename.
#ifdef Q_OS_WIN32
			tr("DLL files") + QLatin1String(" (*.dll);;") +
#else
			tr("rar or unrar") + QLatin1String(" (rar unrar);;") +
#endif
			tr("All Files") + QLatin1String(" (*.*)"));

	if (filename.isEmpty())
		return;

	// Convert to native pathname separators.
	filename = QDir::toNativeSeparators(filename);

	// Set the filename text.
	// Program file status will be updated automatically by
	// the textChanged() signal from QLineEdit.
	txtExtPrgUnRAR->setText(filename);

	// Set focus to the textbox.
	txtExtPrgUnRAR->setFocus(Qt::OtherFocusReason);
}

void GeneralConfigWindow::on_txtExtPrgUnRAR_focusIn(void)
{
	Q_D(GeneralConfigWindow);
#ifdef Q_OS_WIN32
	d->extprgDisplayFileStatus(tr("UnRAR DLL"), d->sExtPrgStatus_UnRAR);
#else
	d->extprgDisplayFileStatus(tr("RAR or UnRAR binary"), d->sExtPrgStatus_UnRAR);
#endif
}

void GeneralConfigWindow::on_txtExtPrgUnRAR_textChanged(void)
{
	// Check the RAR binary to make sure it's valid.
	// TODO: Split status detection into a separate function like Sega CD Boot ROMs?
	QString prg_id = tr("Unknown");
	QString prg_status;
	QStyle::StandardPixmap filename_icon = QStyle::SP_MessageBoxQuestion;
	LibGens::DcRar::ExtPrgInfo prg_info;
	
	// Check if the file exists.
	Q_D(GeneralConfigWindow);
	QString filename = txtExtPrgUnRAR->text();
	if (filename.isEmpty()) {
		prg_status = tr("No filename specified.");
	} else {
		int status = LibGens::DcRar::CheckExtPrg(txtExtPrgUnRAR->text().toUtf8().constData(), &prg_info);
		switch (status) {
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
				prg_status = d->sWarning + tr("The specified file is not executable.");
				filename_icon = QStyle::SP_MessageBoxWarning;
				break;

			case -3:
				// File isn't a regular file.
				prg_status = d->sWarning + tr("The specified file is not a regular file.");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;

			case -4:
				// Error calling stat().
				// TODO: Get the stat() error code?
				prg_status = d->sWarning + tr("Error calling stat().");
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;

#ifdef Q_OS_WIN32
			case -5:
				// UnRAR.dll API version is too old. (Win32 only)
				prg_status = d->sWarning + tr("UnRAR.dll API version is too old.") + QLatin1String("<br/>\n") +
							   tr("Gens/GS II requires API version %1 or later.").arg(RAR_DLL_VERSION);
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;
#endif

			case -6:
				// Version information not found.
#ifdef Q_OS_WIN32
				prg_status = d->sWarning + tr("DLL version information not found.");
#else
				prg_status = d->sWarning + tr("Program version information not found.");
#endif
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;

			case -7:
				// Not RAR, UnRAR, or UnRAR.dll.
#ifdef Q_OS_WIN32
				prg_status = d->sWarning + tr("Selected DLL is not UnRAR.dll.");
#else
				prg_status = d->sWarning + tr("Selected program is neither RAR nor UnRAR.");
#endif
				filename_icon = QStyle::SP_MessageBoxCritical;
				break;

			default:
				// Unknown error.
				prg_status = d->sWarning + tr("Unknown error code %1 received from RAR file handler.").arg(status);
				filename_icon = QStyle::SP_MessageBoxWarning;
				break;
		}
	}

	// Set program ID.
	if (prg_info.dll_major != 0 || prg_info.dll_minor != 0 ||
	    prg_info.dll_revision != 0 || prg_info.dll_build != 0)
	{
		switch (prg_info.rar_type) {
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
	d->sExtPrgStatus_UnRAR = tr("Identified as: %1").arg(prg_id);

	// Line break string.
	static const QString sLineBreak = QLatin1String("<br/>\n");

	// Print DLL version information, if available.
	if (prg_info.dll_major != 0 || prg_info.dll_minor != 0 ||
	    prg_info.dll_revision != 0 || prg_info.dll_build != 0)
	{
		QString rar_version;
#ifdef Q_OS_WIN32
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
		d->sExtPrgStatus_UnRAR += sLineBreak + sLineBreak + rar_version;
#ifdef Q_OS_WIN32
		if (prg_info.api_version > 0)
			d->sExtPrgStatus_UnRAR += sLineBreak + tr("API version %1").arg(prg_info.api_version);
#endif
	}

	if (!prg_status.isEmpty())
		d->sExtPrgStatus_UnRAR += sLineBreak + sLineBreak + prg_status;

	// Set the textbox's icon.
	txtExtPrgUnRAR->setIcon(style()->standardIcon(filename_icon));

	// TODO: Create a constant string for DLL vs. binary.
	// For now, just call focusIn() to update the description.
	on_txtExtPrgUnRAR_focusIn();

	// Settings have been changed.
#ifndef GCW_APPLY_IMMED
	d->setApplyButtonEnabled(true);
#else
	SetValByPath_QString("External_Programs/UnRAR", txtExtPrgUnRAR->text());
#endif
}

}
