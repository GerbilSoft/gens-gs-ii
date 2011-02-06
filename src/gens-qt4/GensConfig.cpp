/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensConfig.hpp: Gens configuration.                                     *
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

#include "GensConfig.hpp"

// TODO: Move this to GensConfigHandler.cpp?
#include "libgens/Decompressor/DcRar.hpp"

namespace GensQt4
{

GensConfig::GensConfig()
{
	/** Initialize GensConfig with default settings. **/
	
	/** Onscreen display. **/
	m_osdFpsEnabled = true;
	m_osdFpsColor = QColor(Qt::white);
	m_osdMsgEnabled = true;
	m_osdMsgColor = QColor(Qt::white);
	
	/** External programs. **/
	// TODO: Don't use setExtPrgUnRAR.
	// Instead, set the filename directly.
#ifdef _WIN32
	setExtPrgUnRAR(QString::fromLatin1("UnRAR.dll"));	// TODO: Verify that a relative pathname works!
#else
	// TODO: Check for the existence of unrar and rar.
	// We should:
	// - Default to unrar if it's found.
	// - Fall back to rar if it's found but unrar isn't.
	// - Assume unrar if neither are found.
	setExtPrgUnRAR(QString::fromLatin1("/usr/bin/unrar"));
#endif
	
	/** Graphics settings. **/
	m_aspectRatioConstraint = true;
	m_fastBlur = false;
	m_contrast = 0;
	m_brightness = 0;
	m_grayscale = false;
	m_inverted = false;
	// TODO: Color Scale Method.
	
	/** General settings. **/
	m_autoFixChecksum = true;
	m_autoPause = false;
	m_borderColor = true;
	m_pauseTint = true;
	m_ntscV30Rolling = true;
	
	/** Savestates. **/
	m_saveSlot = 0;
	
	// TODO: Emit signals for all the configuration options?
	// Alternatively, add another function to do that, e.g. refresh().
}

GensConfig::~GensConfig()
{
}


void GensConfig::save(void)
{
	// TODO
}

void GensConfig::reload(void)
{
	// TODO
}


/** Onscreen display. **/


void GensConfig::setOsdFpsEnabled(bool enable)
{
	if (m_osdFpsEnabled == enable)
		return;
	
	m_osdFpsEnabled = enable;
	emit osdFpsEnabled_changed(m_osdFpsEnabled);
}

void GensConfig::setOsdFpsColor(const QColor& color)
{
	if (!color.isValid() || m_osdFpsColor == color)
		return;
	
	m_osdFpsColor = color;
	emit osdFpsColor_changed(m_osdFpsColor);
}

void GensConfig::setOsdMsgEnabled(bool enable)
{
	if (m_osdMsgEnabled == enable)
		return;
	
	m_osdMsgEnabled = enable;
	emit osdMsgEnabled_changed(m_osdMsgEnabled);
}

void GensConfig::setOsdMsgColor(const QColor& color)
{
	if (!color.isValid() || m_osdMsgColor == color)
		return;
	
	m_osdMsgColor = color;
	emit osdMsgColor_changed(m_osdMsgColor);
}


/** Sega CD Boot ROMs. **/


void GensConfig::setMcdRomUSA(const QString& filename)
{
	if (m_mcdRomUSA == filename)
		return;
	
	m_mcdRomUSA = filename;
	emit mcdRomUSA_changed(m_mcdRomUSA);
}

void GensConfig::setMcdRomEUR(const QString& filename)
{
	if (m_mcdRomEUR == filename)
		return;
	
	m_mcdRomEUR = filename;
	emit mcdRomEUR_changed(m_mcdRomEUR);
}

void GensConfig::setMcdRomJPN(const QString& filename)
{
	if (m_mcdRomJPN == filename)
		return;
	
	m_mcdRomJPN = filename;
	emit mcdRomJPN_changed(m_mcdRomJPN);
}


/** External programs. **/


void GensConfig::setExtPrgUnRAR(const QString& filename)
{
	if (m_extprgUnRAR == filename)
		return;
	
	m_extprgUnRAR = filename;
	emit extprgUnRAR_changed(m_extprgUnRAR);
	
	// TODO: Don't set the DcRar filename here.
	// Set it in a signal handler in gqt4_main.cpp or something.
	// Maybe create GensConfigHandler.cpp?
	// (Reasoning is we might have multiple GensConfig instances,
	//  but only one may be active at any given time.)
	LibGens::DcRar::SetExtPrg(m_extprgUnRAR.toUtf8().constData());
}


/** Graphics settings. **/


void GensConfig::setAspectRatioConstraint(bool newAspectRatioConstraint)
{
	if (m_aspectRatioConstraint == newAspectRatioConstraint)
		return;
	
	m_aspectRatioConstraint = newAspectRatioConstraint;
	emit aspectRatioConstraint_changed(m_aspectRatioConstraint);
}

void GensConfig::setFastBlur(bool newFastBlur)
{
	if (m_fastBlur == newFastBlur)
		return;
	
	m_fastBlur = newFastBlur;
	emit fastBlur_changed(m_fastBlur);
}

void GensConfig::setContrast(int newContrast)
{
	if (m_contrast == newContrast)
		return;
	
	m_contrast = newContrast;
	emit contrast_changed(m_contrast);
}

void GensConfig::setBrightness(int newBrightness)
{
	if (m_brightness == newBrightness)
		return;
	
	m_brightness = newBrightness;
	emit brightness_changed(m_brightness);
}

void GensConfig::setGrayscale(bool newGrayscale)
{
	if (m_grayscale == newGrayscale)
		return;
	
	m_grayscale = newGrayscale;
	emit grayscale_changed(m_grayscale);
}

void GensConfig::setInverted(bool newInverted)
{
	if (m_inverted == newInverted)
		return;
	
	m_inverted = newInverted;
	emit inverted_changed(m_inverted);
}

// TODO: Color Scale Method.


/** General settings. **/


void GensConfig::setAutoFixChecksum(bool newAutoFixChecksum)
{
	if (m_autoFixChecksum == newAutoFixChecksum)
		return;
	
	m_autoFixChecksum = newAutoFixChecksum;
	emit autoFixChecksum_changed(m_autoFixChecksum);
}

void GensConfig::setAutoPause(bool newAutoPause)
{
	if (m_autoPause == newAutoPause)
		return;
	
	m_autoPause = newAutoPause;
	emit autoPause_changed(m_autoPause);
}

void GensConfig::setBorderColor(bool newBorderColor)
{
	if (m_borderColor == newBorderColor)
		return;
	
	m_borderColor = newBorderColor;
	emit borderColor_changed(m_borderColor);
}

void GensConfig::setPauseTint(bool newPauseTint)
{
	if (m_pauseTint == newPauseTint)
		return;
	
	m_pauseTint = newPauseTint;
	emit pauseTint_changed(m_pauseTint);
}

void GensConfig::setNtscV30Rolling(bool newNtscV30Rolling)
{
	if (m_ntscV30Rolling == newNtscV30Rolling)
		return;
	
	m_ntscV30Rolling = newNtscV30Rolling;
	emit ntscV30Rolling_changed(m_ntscV30Rolling);
}


/** Savestates. **/


void GensConfig::setSaveSlot(int newSaveSlot)
{
	if (m_saveSlot == newSaveSlot ||
	    newSaveSlot < 0 ||
	    newSaveSlot > 9)
	{
		return;
	}
	
	m_saveSlot = newSaveSlot;
	emit saveSlot_changed(m_saveSlot);
}


}
