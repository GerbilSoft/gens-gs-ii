/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * Screenshot.hpp: Screenshot handler.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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
// TODO: Move stuff to MdFb.
#include "gqt4_main.hpp"

// Qt includes.
#include <QtCore/QDateTime>
#include <QtGui/QImageWriter>

using LibGens::MdFb;

namespace GensQt4 {

Screenshot::Screenshot(LibGens::Rom *rom, LibGens::MdFb *fb, QObject *parent)
	: QObject(parent)
	, m_rom(rom)
	, m_fb(fb)
{
	// Update the internal image.
	update();
}

Screenshot::~Screenshot()
{
	// TODO: Unreference the ROM and MdFb.
	// (This requires adding reference counting...)
}

/**
 * Update the internal image using the given ROM and MdFb.
 */
void Screenshot::update(void)
{
	if (!m_rom || !m_fb) {
		// Missing ROM or MdFb.
		m_img = QImage();
		return;
	}

	// VDP object.
	// TODO: Store VPix and HPixBegin in the MdFb.
	const LibGens::Vdp *vdp = gqt4_emuContext->m_vdp;

	// Create the QImage.
	// TODO: Store VPix and HPixBegin in the MdFb.
	const uint8_t *start;
	const int startY = ((240 - vdp->getVPix()) / 2);
	const int startX = (vdp->getHPixBegin());
	int bytesPerLine;
	QImage::Format imgFormat;
	LibGens::MdFb *fb = vdp->MD_Screen;

	const MdFb::ColorDepth bpp = fb->bpp();
	if (bpp == MdFb::BPP_32) {
		start = (const uint8_t*)(fb->lineBuf32(startY) + startX);
		bytesPerLine = (fb->pxPitch() * sizeof(uint32_t));
		imgFormat = QImage::Format_RGB32;
	} else {
		start = (const uint8_t*)(fb->lineBuf16(startY) + startX);
		bytesPerLine = (fb->pxPitch() * sizeof(uint16_t));
		if (bpp == MdFb::BPP_16)
			imgFormat = QImage::Format_RGB16;
		else
			imgFormat = QImage::Format_RGB555;
	}

	// TODO: Check for errors.
	m_img = QImage(start, vdp->getHPix(), vdp->getVPix(),
			bytesPerLine, imgFormat);	
}

/**
 * Save the image to a file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int Screenshot::save(const QString &filename)
{
	if (m_img.isNull())
		return -1;

	QImageWriter writer(filename, "png");
	return save_int(writer);
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

	QImageWriter writer(device, "png");
	return save_int(writer);
}

/**
 * Save the image using a QImageWriter.
 * @param writer QImageWriter.
 * @return 0 on success; non-zero on error.
 */
int Screenshot::save_int(QImageWriter& writer)
{
	// TODO: Figure out what should be saved in the Description field.
	// TODO: Save extra data.
	writer.write(m_img);
	return 0;
}

}
