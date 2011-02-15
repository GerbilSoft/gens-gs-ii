/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLTex2D.cpp: GL_TEXTURE_2D wrapper class.                               *
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

#include "GLTex2D.hpp"

// Win32 requires GL/glext.h for OpenGL 1.2/1.3.
// TODO: Check the GL implementation to see what functionality is available at runtime.
#ifdef _WIN32
#include <GL/glext.h>
#endif
#include <stdio.h>
// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

namespace GensQt4
{

GLTex2D::GLTex2D()
{
	// Initialize variables.
	m_tex = 0;
	m_pow2_w = 0.0;
	m_pow2_h = 0.0;
	m_img_w = 0;
	m_img_h = 0;
}

GLTex2D::GLTex2D(const QImage& img)
{
	// Do main initialization.
	GLTex2D();
	
	// Set the image.
	setImage(img);
}


GLTex2D::~GLTex2D()
{
	if (m_tex > 0)
	{
		glDeleteTextures(1, &m_tex);
		m_tex = 0;
	}
}


/**
 * setImage(): Set the texture image from a QImage.
 * @param img QImage.
 */
void GLTex2D::setImage(const QImage& img)
{
	if (img.isNull())
		return;
	
	if (m_tex == 0)
		glGenTextures(1, &m_tex);
	
	glBindTexture(GL_TEXTURE_2D, m_tex);
	
	// Set texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// GL filtering.
	// TODO: Add a user-selectable option.
	// We're moving preview images here, so it's always filtered.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Determine the texture format.
	int bytes_per_pixel;
	bool needsPackedPixels = false;
	QImage new_img = img;
	
	switch (new_img.format())
	{
		case QImage::Format_RGB32:
		case QImage::Format_ARGB32:
			m_components = 4;
			m_format = GL_BGRA;
			m_type = GLTEX2D_FORMAT_32BIT;
			bytes_per_pixel = 4;
			break;
		
		case QImage::Format_RGB888:
			// TODO: Verify this!
			m_components = 3;
			m_format = GL_RGB;
			m_type = GL_UNSIGNED_BYTE;	// TODO: Big-endian support.
			bytes_per_pixel = 3;
			break;
		
		case QImage::Format_RGB16:
			m_components = 3;
			m_format = GL_RGB;
			m_type = GL_UNSIGNED_SHORT_5_6_5;
			needsPackedPixels = true;
			bytes_per_pixel = 2;
			break;
		
		case QImage::Format_RGB555:
			m_components = 4;
			m_format = GL_BGRA;
			m_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			needsPackedPixels = true;
			bytes_per_pixel = 2;
			break;
		
		default:
			// Convert to 32-bit color.
			// TODO: Big-endian support.
			new_img = img.convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly);
			m_components = 4;
			m_format = GL_BGRA;
			m_type = GLTEX2D_FORMAT_32BIT;
			bytes_per_pixel = 4;
			break;
	}
	
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
	// GL_UNSIGNED_INT_8_8_8_8_REV requires the packed pixels extension.
	if (m_type == GLTEX2D_FORMAT_32BIT)
		needsPackedPixels = true;
#endif
	
	// 15-bit and 16-bit color requires GL_EXT_packed_pixels.
	// 32-bit color requires GL_EXT_packed_pixels on big-endian systems.
	// TODO: GLEW doesn't have GL_APPLE_packed_pixels.
	// Check if it exists manually.
	const bool hasExtPackedPixels = (GLEW_VERSION_1_2
						|| GLEW_EXT_packed_pixels
						/*|| GLEW_APPLE_packed_pixels*/
						);
	if (needsPackedPixels && !hasExtPackedPixels)
	{
		// Packed pixels extension isn't available.
		// Convert to 32-bit color.
		new_img = img.convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly);
		m_components = 4;
		m_format = GL_BGRA;
		m_type = GLTEX2D_FORMAT_32BIT;
		bytes_per_pixel = 4;
		
		// TODO: Byteswap the data on big-endian.
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
#ifdef Q_WS_MAC
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"GL_APPLE_packed_pixels is missing.");
#else /* !Q_WS_MAC */
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"GL_EXT_packed_pixels is missing.");
#endif /* Q_WS_MAC */
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_WARNING,
				"Textures may appear garbled.");
#endif /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
	}
	
	// Save the original image size.
	m_img_w = new_img.width();
	m_img_h = new_img.height();
	
	// Round the image width and height to the next power of two.
	int pow2_w = next_pow2s(new_img.width());
	int pow2_h = next_pow2s(new_img.height());
	
	void *texBuf;
	if (pow2_w == new_img.width() && pow2_h == new_img.height())
	{
		// Image size is already a power of two.
		m_pow2_w = 1.0;
		m_pow2_h = 1.0;
		
		// Don't allocate a blank texture buffer.
		texBuf = NULL;
	}
	else
	{
		// Image size is not a power of two.
		m_pow2_w = ((double)(new_img.width()) / (double)(pow2_w));
		m_pow2_h = ((double)(new_img.height()) / (double)(pow2_h));
		
		// Allocate a memory buffer to use for texture initialization.
		// This will ensure that the entire texture is initialized to black.
		// (This fixes garbage on the last column when using the Fast Blur shader.)
		const size_t texSize = (pow2_w * pow2_h * bytes_per_pixel);
		texBuf = calloc(1, texSize);
	}
	
	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
			m_components,
			pow2_w, pow2_h,		// Texture size.
			0,			// No border.
			m_format, m_type, texBuf);
	
	// Free the temporary texture buffer.
	free(texBuf);
	
	// Upload the image texture.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, new_img.bytesPerLine() / bytes_per_pixel);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	
	const uchar *img_data = new_img.scanLine(0);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0, 0,		// x/y offset
			new_img.width(), new_img.height(),	// width/height
			m_format, m_type, img_data);
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
}

}

