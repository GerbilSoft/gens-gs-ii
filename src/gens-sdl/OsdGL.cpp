/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdGL.hpp: Onscreen Display for OpenGL.                                 *
 *                                                                         *
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

#include "OsdGL.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstdio> /* REMOVE LATER */
#include <cstdlib>
#include <cstring>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// OSD font.
#include "OsdFont_VGA.hpp"

namespace GensSdl {

class OsdGLPrivate {
	public:
		OsdGLPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		OsdGLPrivate(const OsdGLPrivate &);
		OsdGLPrivate &operator=(const OsdGLPrivate &);

	public:
		GLuint texOsd;			// OSD texture.
		GLfloat osdVertex[256][8];	// Texture vertex array.

		/**
		 * (Re-)Allocate the OSD texture.
		 */
		void reallocOsdTexture();

		/**
		 * Print a line of text on the screen.
		 * @param x X coordinate.
		 * @param y Y coordinate.
		 * @param msg Line of text. (NOTE: ASCII only right now...)
		 */
		void printLine(int x, int y, const char *msg);
};

/** OsdGLPrivate **/

OsdGLPrivate::OsdGLPrivate()
	: texOsd(0)
{ }

/**
 * (Re-)Allocate the OSD texture.
 */
void OsdGLPrivate::reallocOsdTexture()
{
	if (texOsd == 0) {
		glGenTextures(1, &texOsd);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texOsd);

	// Set texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Texture filtering.
	// TODO: Should we use linear filtering for the OSD text?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glDisable(GL_TEXTURE_2D);

	// Calculate the GL vertex array.
	GLfloat *vtx = &osdVertex[0][0];
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			vtx[0] = ((float)(x+0) / 16.0f);
			vtx[1] = ((float)(y+0) / 16.0f);
			vtx[2] = ((float)(x+1) / 16.0f);
			vtx[3] = ((float)(y+0) / 16.0f);
			vtx[4] = ((float)(x+1) / 16.0f);
			vtx[5] = ((float)(y+1) / 16.0f);
			vtx[6] = ((float)(x+0) / 16.0f);
			vtx[7] = ((float)(y+1) / 16.0f);
			vtx += 8;
		}
	}

	// Create the GL image.
	// Using GL_ALPHA.
	// TODO: Optimize this?
	uint8_t *glImage = (uint8_t*)malloc(256 * 16 * 8);
	// Converting 1bpp characters to 8bpp.
	const int chrW = 8;	// must be pow2
	const int chrH = 16;	// must be pow2
	// pitch = 8 pixels per character; 16 per line.
	const int pitch = chrW * 16;
	for (int chr = 0; chr < 256; chr++) {
		const int y_pos = (chr & ~(chrH - 1));
		const int x_pos = (chr & (chrH - 1)) * chrW;

		uint8_t *pos = &glImage[(y_pos * pitch) + x_pos];
		for (int y = 0; y < chrH; y++, pos += (pitch - chrW)) {
			uint8_t chr_data = VGA_charset_ASCII[chr][y];
			for (int x = chrW; x > 0; x--, chr_data <<= 1) {
				*pos = (chr_data & 0x80 ? 0xFF : 0);
				pos++;
			}
		}
	}

	// Upload the image data.
	glTexImage2D(GL_TEXTURE_2D, 0,
			GL_ALPHA,	// One component.
			128, 256,	// Texture size.
			0,		// No border.
			GL_ALPHA, GL_UNSIGNED_BYTE, glImage);
	free(glImage);
}

/**
 * Print a line of text on the screen.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param msg Line of text. (NOTE: ASCII only right now...)
 */
void OsdGLPrivate::printLine(int x, int y, const char *msg)
{
	// TODO: Font information.
	const int chrW = 8;
	const int chrH = 16;

	// TODO: Wordwrapping.

	// TODO: Precalculate vertices?
	const int len = (int)strlen(msg);
	GLint *vtx = new GLint[len * 8];
	GLfloat *txc = new GLfloat[len * 8];

	for (int i = 0; i < len; i++, x += chrW, msg++) {
		uint8_t chr = *msg;
		if (chr == 0)
			break;

		// Vertex coordinates.
		vtx[(i*8)+0] = x;
		vtx[(i*8)+1] = y;
		vtx[(i*8)+2] = x+chrW;
		vtx[(i*8)+3] = y;
		vtx[(i*8)+4] = x+chrW;
		vtx[(i*8)+5] = y+chrH;
		vtx[(i*8)+6] = x;
		vtx[(i*8)+7] = y+chrH;

		// Texture coordinates.
		memcpy(&txc[i*8], &osdVertex[chr][0], sizeof(osdVertex[chr]));
	}

	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, txc);
	glDrawArrays(GL_QUADS, 0, len*4);

	delete[] txc;
	delete[] vtx;
}

/** OsdGL **/

OsdGL::OsdGL()
	: d(new OsdGLPrivate())
{ }

OsdGL::~OsdGL()
{
	end();	// TODO: Must be in a valid GL context...
	delete d;
}

/**
 * Initialize the Onscreen Display.
 * This must be called from a valid GL context.
 */
void OsdGL::init(void)
{
	// Allocate the OSD texture.
	d->reallocOsdTexture();
}

/**
 * Shut down the Onscreen Display.
 * This must be called from a valid GL context.
 */
void OsdGL::end(void)
{
	if (d->texOsd > 0) {
		glDeleteTextures(1, &d->texOsd);
		d->texOsd = 0;
	}
}

/**
 * Draw the Onscreen Display.
 * This must be called from a valid GL context.
 */
void OsdGL::draw(void)
{
	// Set pixel matrices.
	// Assuming 320x240 for 1x text rendering.
	// TODO: Use a larger matrix when rendering to a larger screen?
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 320, 240, 0, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Trick to fix up pixel alignment.
	// See http://basic4gl.wikispaces.com/2D+Drawing+in+OpenGL
	glTranslatef(0.375f, 0.375f, 0.0f);

	// Enable 2D textures.
	glEnable(GL_TEXTURE_2D);

	// Enable GL blending.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind the OSD texture.
	glBindTexture(GL_TEXTURE_2D, d->texOsd);

	// Enable vertex and texture coordinate arrays.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// TODO: Process the OSD.
	d->printLine(0, 0, "testing 1 2 3");

	// Done with vertex and texture coordinate arrays.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Reset the GL state.
	//glColor4f(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_BLEND);			// Disable GL blending.
	glDisable(GL_TEXTURE_2D);		// Disable 2D textures.

	// Restore the matrices.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

}
