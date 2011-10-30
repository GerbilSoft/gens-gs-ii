/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * Screenshot.hpp: Screenshot handler.                                     *
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

#include "Screenshot.hpp"

// LibGens includes.
#include "libgens/Vdp/Vdp.hpp"
#include "libgens/Vdp/VdpPalette.hpp"

// Qt includes.
#include <QtGui/QImageWriter>

namespace GensQt4
{

Screenshot::Screenshot(LibGens::Rom *rom, LibGens::EmuContext *context, QObject *parent)
	: QObject(parent)
	, m_rom(rom)
	, m_context(context)
{
	// Update the internal image.
	update();
}


Screenshot::~Screenshot()
{
	// TODO: Unreference the ROM and EmuContext.
	// (This requires adding reference counting...)
}


/**
 * Update the internal image using the given ROM and EmuContext.
 */
void Screenshot::update(void)
{
	if (!m_rom || !m_context)
	{
		// Missing ROM or EmuContext.
		m_img = QImage();
		return;
	}
	
	// VDP object.
	const LibGens::Vdp *vdp = m_context->m_vdp;
	
	// Get the color depth.
	const LibGens::VdpPalette::ColorDepth bpp = vdp->m_palette.bpp();
	
	// Create the QImage.
	const uint8_t *start;
	const int startY = ((240 - vdp->GetVPix()) / 2);
	const int startX = (vdp->GetHPixBegin());
	int bytesPerLine;
	QImage::Format imgFormat;
	
	if (bpp == LibGens::VdpPalette::BPP_32)
	{
		start = (const uint8_t*)(vdp->MD_Screen->lineBuf32(startY) + startX);
		bytesPerLine = (vdp->MD_Screen->pxPitch() * sizeof(uint32_t));
		imgFormat = QImage::Format_RGB32;
	}
	else
	{
		start = (const uint8_t*)(vdp->MD_Screen->lineBuf16(startY) + startX);
		bytesPerLine = (vdp->MD_Screen->pxPitch() * sizeof(uint16_t));
		if (bpp == LibGens::VdpPalette::BPP_16)
			imgFormat = QImage::Format_RGB16;
		else
			imgFormat = QImage::Format_RGB555;
	}
	
	// TODO: Check for errors.
	m_img = QImage(start, vdp->GetHPix(), vdp->GetVPix(),
			bytesPerLine, imgFormat);	
}


/**
 * Save the image to a file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int Screenshot::save(QString filename)
{
	if (m_img.isNull())
		return -1;
	
	QImageWriter imgWriter(filename, "png");
	imgWriter.write(m_img);
	return 0;
}


/**
 * Save the image to a QIODevice.
 * @param device QIODevice.
 * @return 0 on success; non-zero on error.
 */
int Screenshot::save(QIODevice *device)
{
	if (m_img.isNull())
		return -1;
	
	QImageWriter imgWriter(device, "png");
	imgWriter.write(m_img);
	return 0;
}

}
