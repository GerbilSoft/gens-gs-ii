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

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::u16string;
using std::vector;

// LibGens.
#include "libgens/Util/Timing.hpp"
// LibGensText.
#include "libgenstext/Encoding.hpp"

// GL Texture wrapper.
#include "GLTex.hpp"

// OSD fonts.
#include "OsdFont_VGA.hpp"
#include "OsdFont_C64.hpp"

// ZOMG image data.
#include "libzomg/img_data.h"

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
		GLTex texOsd;			// OSD texture.
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
		void printLine(int x, int y, const string &msg);

		// Timer.
		LibGens::Timing timer;

		// TODO: Get this from the OSD font.
		static const int chrW = 8;	// must be pow2
		static const int chrH = 8;	// must be pow2

		// OSD queue.
		// NOTE: Manually allocating objects.
		// FIXME: duration should probably be int to match the printf functions.
		struct OsdMessage {
			string msg;		// Message. (Converted to internal 8-bit charset.)
			unsigned int duration;	// Duration, in milliseconds.
			bool hasDisplayed;	// Timer starts counting down once the message has been displayed.
			uint64_t endTime;	// End time, using internal Timing object. (microseconds)
		};
		vector<OsdMessage*> osdList;

		// OSD fade-out time, in microseconds.
		// TODO: Make this customizable.
		static const int fadeOutTime = 250000;

		/**
		 * Convert a UTF-8 string to the internal character set.
		 * @param str UTF-8 string.
		 * @param len Length of str.
		 * @return String using the internal character set.
		 */
		static string utf8ToInternal(const char *str, size_t len);

		/**
		 * Convert a UTF-16 string to the internal character set.
		 * @param str UTF-16 string.
		 * @param len Length of str.
		 * @return String using the internal character set.
		 */
		static string utf16ToInternal(const char16_t *str, size_t len);

		// OpenGL Display List.
		GLuint displayList;

		// Is the OSD list dirty?
		// If so, the display list needs to be regenerated.
		// TODO: Use a display list.
		bool dirty;

		/**
		 * Check for expired messages.
		 * Expired messages will be removed from the list.
		 */
		void checkForExpiredMessages(void);

		/** Properties. **/
		bool fpsEnabled;
		bool msgEnabled;

		uint32_t fpsColor;
		uint32_t msgColor;

		static void setGLColor(uint32_t color);
		static void setGLColor(uint32_t color, float alpha);

		// Display offset.
		// This is used for aspect ratio constraints.
		double offset_x, offset_y;
		double ortho[4];	// Order: Left, Right, Bottom, Top
		// Default ortho[] values.
		// New ortho[] values are calculated relative to the defaults.
		static const double ortho_default[4];

		// Preview image.
		// FIXME: duration should probably be int to match the printf functions.
		struct preview_t {
			GLTex tex;
			bool visible;
			bool hasDisplayed;
			unsigned int duration;
			uint64_t endTime;

			// Coordinate arrays.
			GLdouble txc[4][2];	// Texture coordinates.
			GLint vtx[4][2];	// Vertex coordinates.
			GLint vtx_border[4][2];	// Vertex coordinates. (border)
			GLint vtx_sh[4][2];	// Vertex coordinates. (drop shadow)

			preview_t()
				: visible(false)
				, hasDisplayed(false)
				, duration(0)
				, endTime(0)
			{
				// Nothing but initialization here.
			}
		} preview;
};

// Default ortho[] values.
// New ortho[] values are calculated relative to the defaults.
const double OsdGLPrivate::ortho_default[4] = {0.0, 320.0, 240.0, 0.0};

/** OsdGLPrivate **/

OsdGLPrivate::OsdGLPrivate()
	: displayList(0)
	, dirty(true)
	, fpsEnabled(false)
	, msgEnabled(true)
	, fpsColor(0xFFFFFFFF)
	, msgColor(0xFFFFFFFF)
	, offset_x(0), offset_y(0)
{
	// Reserve space for at least 8 OSD messages.
	osdList.reserve(8);

	// Initialize the glOrtho() cache.
	// TODO: Option to change the virtual 320x240 display?
	memcpy(ortho, ortho_default, sizeof(ortho));
}

/**
 * (Re-)Allocate the OSD texture.
 */
void OsdGLPrivate::reallocOsdTexture()
{
	if (displayList == 0) {
		// Create a DisplayList.
		displayList = glGenLists(1);
	}

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
	uint8_t *glImage = (uint8_t*)malloc(256 * chrW * chrH);
	// Converting 1bpp characters to 8bpp.
	// pitch = 8 pixels per character; 16 per line.
	const int pitch = chrW * 16;
	for (int chr = 0; chr < 256; chr++) {
		const int y_pos = (chr / 16) * chrH;
		const int x_pos = (chr & 15) * chrW;

		uint8_t *pos = &glImage[(y_pos * pitch) + x_pos];
		for (int y = 0; y < chrH; y++, pos += (pitch - chrW)) {
			uint8_t chr_data = C64_charset_ASCII[chr][y];
			for (int x = chrW; x > 0; x--, chr_data <<= 1) {
				*pos = ((chr_data & 0x80) ? 0xFF : 0);
				pos++;
			}
		}
	}

	// Allocate the texture.
	// TODO: Texture data parameter?
	texOsd.alloc(GLTex::FMT_ALPHA8, chrW*16, chrH*16);
	texOsd.subImage2D(chrW*16, chrH*16, chrW*16, glImage);
	free(glImage);

	// OSD is dirty.
	dirty = true;
}

/**
 * Print a line of text on the screen.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param msg Line of text. (NOTE: ASCII only right now...)
 */
void OsdGLPrivate::printLine(int x, int y, const std::string &msg)
{
	// TODO: Wordwrapping.

	// TODO: Precalculate vertices?
	const int len = (int)msg.size();
	// TODO: Allocate once, and reallocate if a larger one is needed?
	GLint *vtx = new GLint[len * 8];
	GLfloat *txc = new GLfloat[len * 8];

	// TODO: Optimize this!
	const char *pChr = msg.c_str();
	for (int i = 0; i < (int)msg.size(); i++, x += chrW, pChr++) {
		uint8_t chr = *pChr;
		if (chr == 0)
			break;

		// Vertex coordinates.
		GLTex::toCoords(&vtx[i*8], x, y, chrW, chrH);
		// Texture coordinates.
		memcpy(&txc[i*8], &osdVertex[chr][0], sizeof(osdVertex[chr]));
	}

	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, txc);
	glDrawArrays(GL_QUADS, 0, len*4);

	delete[] txc;
	delete[] vtx;
}

/**
 * Convert a UTF-8 string to the internal character set.
 * @param str UTF-8 string.
 * @param len Length of str.
 * @return String using the internal character set.
 */
string OsdGLPrivate::utf8ToInternal(const char *str, size_t len)
{
	// TODO: Move to after utf16ToInternal()?

	// Convert to UTF-16 first.
	u16string u16 = LibGensText::Utf8_to_Utf16(str, len);
	// Convert to the local 8-bit character set.
	return utf16ToInternal(u16.data(), u16.size());
}

/**
 * Convert a UTF-16 string to the internal character set.
 * @param str UTF-16 string.
 * @param len Length of str.
 * @return String using the internal character set.
 */
string OsdGLPrivate::utf16ToInternal(const char16_t *str, size_t len)
{
	string cp8;	// for return value optimization

	// Convert to the local 8-bit character set.
	cp8.resize(len);
	for (int i = 0; i < (int)len; i++, str++) {
		char16_t chr16 = *str;
		if (chr16 <= 0xFF) {
			// U+0000 - U+00FF.
			cp8[i] = (char)chr16;
		} else {
			// Some Unicode characters over U+00FF are supported.
			// TODO: Replacement character for unsupported characters.

			// Check if this is a supported cp437 character.
			// TODO: Optimize this!
			char chr8;
			switch (chr16) {
				case 0x263A:	chr8 = 0x01; break;
				case 0x263B:	chr8 = 0x02; break;
				case 0x2665:	chr8 = 0x03; break;
				case 0x2666:	chr8 = 0x04; break;
				case 0x2663:	chr8 = 0x05; break;
				case 0x2660:	chr8 = 0x06; break;
				case 0x2022:	chr8 = 0x07; break;
				case 0x2508:	chr8 = 0x08; break;
				case 0x25CB:	chr8 = 0x09; break;
				case 0x25D9:	chr8 = 0x0A; break;
				case 0x2642:	chr8 = 0x0B; break;
				case 0x2640:	chr8 = 0x0C; break;
				case 0x266A:	chr8 = 0x0D; break;
				case 0x266B:	chr8 = 0x0E; break;
				case 0x263C:	chr8 = 0x0F; break;
				case 0x25BA:	chr8 = 0x10; break;
				case 0x25C4:	chr8 = 0x11; break;
				case 0x2195:	chr8 = 0x12; break;
				case 0x203C:	chr8 = 0x13; break;
				//case 0x00B6:	chr8 = 0x14; break;	// This is part of cp1252...
				//case 0x00A7:	chr8 = 0x15; break;	// This is part of cp1252...
				case 0x25AC:	chr8 = 0x16; break;
				case 0x21A8:	chr8 = 0x17; break;
				case 0x2191:	chr8 = 0x18; break;
				case 0x2193:	chr8 = 0x19; break;
				case 0x2192:	chr8 = 0x1A; break;
				case 0x2190:	chr8 = 0x1B; break;
				case 0x221F:	chr8 = 0x1C; break;
				case 0x2194:	chr8 = 0x1D; break;
				case 0x2582:	chr8 = 0x1E; break;
				case 0x25BC:	chr8 = 0x1F; break;

				// VCR symbols.
				// NOTE: MSVC 2010 complains about
				// truncation of constant values.
				case 0x25CF:	chr8 = (char)0x80; break;	// Record. (BLACK CIRCLE)
				case 0xF8FE:	chr8 = (char)0x81; break;	// Pause. (Private Use Area)
				case 0x25A0:	chr8 = (char)0x82; break;	// Stop. (BLACK SQUARE)

				default:	chr8 = 0; break;
			}

			cp8[i] = chr8;
		}
	}

	// Return the new string.
	return cp8;
}

/**
 * Check for expired messages.
 * Expired messages will be removed from the list.
 */
void OsdGLPrivate::checkForExpiredMessages(void)
{
	const uint64_t curTime = timer.getTime();
	bool isAllProcessed = true;

	for (int i = (int)osdList.size() - 1; i >= 0; i--) {
		OsdGLPrivate::OsdMessage *osdMsg = osdList[i];
		if (!osdMsg)
			continue;

		// End time plus fade-out.
		const uint64_t endTimePlusFadeOut = (osdMsg->endTime + fadeOutTime);

		if (!osdMsg->hasDisplayed) {
			// Message hasn't been displayed yet.
			// Reset its end time.
			osdMsg->endTime = curTime + (osdMsg->duration * 1000);
			isAllProcessed = false;
		} else if (curTime >= endTimePlusFadeOut) {
			// Time has elapsed.
			// Remove the message from the list.
			delete osdList[i];
			osdList[i] = nullptr;
			dirty = true;
		} else if (curTime >= osdMsg->endTime) {
			// Time has elapsed, but the message is fading out.
			// Make sure the message is redrawn.
			isAllProcessed = false;
			dirty = true;
		} else {
			// Message has not been processed yet.
			isAllProcessed = false;
		}
	}

	// Process the preview image.
	// TODO: Combine with OSD message processing code above?
	if (preview.visible) {
		// End time plus fade-out.
		const uint64_t endTimePlusFadeOut = (preview.endTime + fadeOutTime);

		if (!preview.hasDisplayed) {
			// Preview image hasn't been displayed yet.
			// Reset its end time.
			preview.endTime = curTime + (preview.duration * 1000);
			isAllProcessed = false;
		} else if (curTime >= endTimePlusFadeOut) {
			// Time has elapsed.
			// Hide the preview image.
			// NOTE: Texture isn't deallocated here...
			preview.visible = false;
			dirty = true;
		} else if (curTime >= preview.endTime) {
			// Time has elapsed, but the preview image is fading out.
			// Make sure the preview image is redrawn.
			isAllProcessed = false;
			dirty = true;
		} else {
			// Preview image has not been processed yet.
			isAllProcessed = false;
		}
	}

	if (isAllProcessed) {
		// TODO: Use an std::list and remove entries as they're processed?
		// All messages have been processed.
		osdList.clear();
	}
}

/**
 * Set the OpenGL color
 * @param color 32-bit ARGB color.
 */
inline void OsdGLPrivate::setGLColor(uint32_t color)
{
	// FIXME: 0xFFFFFF seems to map to (254,254,254)...
	const uint8_t a = (color >> 24) & 0xFF;
	const uint8_t r = (color >> 16) & 0xFF;
	const uint8_t g = (color >>  8) & 0xFF;
	const uint8_t b = (color >>  0) & 0xFF;
	glColor4ub(r, g, b, a);
}

/**
 * Set the OpenGL color
 * @param color 32-bit ARGB color.
 * @param alpha Alpha value.
 */
inline void OsdGLPrivate::setGLColor(uint32_t color, float alpha)
{
	// FIXME: 0xFFFFFF seems to map to (254,254,254)...
	const uint8_t a = (uint8_t)(alpha * 255);
	const uint8_t r = (color >> 16) & 0xFF;
	const uint8_t g = (color >>  8) & 0xFF;
	const uint8_t b = (color >>  0) & 0xFF;
	glColor4ub(r, g, b, a);
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
	d->texOsd.dealloc();
	d->preview.tex.dealloc();
	if (d->displayList > 0) {
		glDeleteLists(d->displayList, 1);
		d->displayList = 0;
	}
}

/**
 * Draw the Onscreen Display.
 * This must be called from a valid GL context.
 */
void OsdGL::draw(void)
{
	if (d->osdList.empty()) {
		// No OSD messages.
		// TODO: Check for FPS and "enabled" values.
		return;
	}

	// Check for expired messages.
	d->checkForExpiredMessages();

	if (!d->dirty) {
		// OSD is not dirty.
		// Call the Display List.
		glCallList(d->displayList);
		return;
	}

	// OSD is dirty.
	// Create a new GL display list.
	glNewList(d->displayList, GL_COMPILE_AND_EXECUTE);

	// Set pixel matrices.
	// Assuming 320x240 for 1x text rendering.
	// TODO: Use a larger matrix when rendering to a larger screen?
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(d->ortho[0], d->ortho[1], d->ortho[2], d->ortho[3], -1.0f, 1.0f);

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

	// Enable vertex and texture coordinate arrays.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Current time is needed for message fade-out.
	// TODO: Drop shadow fade-out looks "weird"...
	const uint64_t curTime = d->timer.getTime();

	// OSD preview image.
	if (d->preview.visible) {
		// Preview image is now being displayed.
		d->preview.hasDisplayed = true;

		// Check for fade-out.
		float alpha = 1.0f;
		if (curTime > d->preview.endTime) {
			// Fading out.
			alpha -= (curTime - d->preview.endTime) / (float)d->fadeOutTime;
			if (alpha < 0.0f) {
				alpha = 0.0f;
			}
		}

		// Bind the preview texture.
		glBindTexture(GL_TEXTURE_2D, d->preview.tex.name);
		// Use linear filtering.
		// TODO: GLTex function for this?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Drop shadow.
		// NOTE: Alpha is divided by 2 due to the larger drop shadow.
		// TODO: Do this with the OSD messages too?
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0f, 0.0f, 0.0f, alpha / 2);
		glVertexPointer(2, GL_INT, 0, d->preview.vtx_sh);
		glDrawArrays(GL_QUADS, 0, 4);

		// Border.
		// FIXME: Move after drop shadow.
		glColor4f(0.0f, 0.0f, 0.0f, alpha);
		glVertexPointer(2, GL_INT, 0, d->preview.vtx_border);
		glDrawArrays(GL_QUADS, 0, 4);

		// Preview image.
		// TODO: Ignore the alpha channel of the preview image?
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glVertexPointer(2, GL_INT, 0, d->preview.vtx);
		glTexCoordPointer(2, GL_DOUBLE, 0, d->preview.txc);
		glDrawArrays(GL_QUADS, 0, 4);
	}

	// Bind the OSD texture.
	glBindTexture(GL_TEXTURE_2D, d->texOsd.name);
	// TODO: Adjust for visible texture size.
	int y = (240 - d->chrH);

	// TODO: Message Enable, Message Color.
	// TODO: Switch from vector to list?
	// TODO: Move to OsdGLPrivate?
	for (int i = (int)d->osdList.size() - 1; i >= 0; i--) {
		OsdGLPrivate::OsdMessage *osdMsg = d->osdList[i];
		if (!osdMsg)
			continue;

		// NOTE: Message expiration is checked at the beginning of the function.

		// Message is now being displayed.
		osdMsg->hasDisplayed = true;

		// Next line.
		y -= d->chrH;

		// Check for fade-out.
		float alpha = 1.0f;
		if (curTime > osdMsg->endTime) {
			// Fading out.
			alpha -= (curTime - osdMsg->endTime) / (float)d->fadeOutTime;
			if (alpha < 0.0f) {
				alpha = 0.0f;
			}
		}
		// TODO: Make the drop shadow optional.
		glColor4f(0.0f, 0.0f, 0.0f, alpha);
		d->printLine(d->chrW+1, y+1, osdMsg->msg);
		d->setGLColor(d->msgColor, alpha);
		d->printLine(d->chrW, y, osdMsg->msg);
	}

	// Done with vertex and texture coordinate arrays.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Reset the GL state.
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);			// Disable GL blending.
	glDisable(GL_TEXTURE_2D);		// Disable 2D textures.

	// Restore the matrices.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// Finished creating the GL display list.
	glEndList();
	d->dirty = false;
}

/**
 * Set the display offset.
 * This is used for aspect ratio constraints.
 * @param x X offset.
 * @param y Y offset.
 */
void OsdGL::setDisplayOffset(double x, double y)
{
	// NOTE: We're not going to bother checking if the
	// values are the same, since they probably aren't.
	d->offset_x = x;
	d->offset_y = y;

	// Update the glOrtho() cache.
	d->ortho[0] = d->ortho_default[0] - d->offset_x;
	d->ortho[1] = d->ortho_default[1] + d->offset_x;
	d->ortho[2] = d->ortho_default[2] + d->offset_y;
	d->ortho[3] = d->ortho_default[3] - d->offset_y;

	// Redraw is needed.
	d->dirty = true;
}

/**
 * Are messages present in the message queue?
 * This should be queried to determine if the
 * video backend needs to be updated.
 * @return True if messages are present; false if not.
 */
bool OsdGL::hasMessages(void) const
{
	return !d->osdList.empty() || d->preview.visible;
}

/**
 * Process messages.
 * This usually only needs to be called if the emulator is paused.
 * @return True if messages were processed; false if not.
 */
bool OsdGL::processMessages(void)
{
	d->checkForExpiredMessages();

	// If messages were processed, OsdGL is dirty.
	return d->dirty;
}

/**
 * Add a message to the OSD queue.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (UTF-8)
 * TODO: printf() function.
 */
void OsdGL::print(unsigned int duration, const char *msg)
{
	// If the OSD is currently idle, reset the timer.
	// TODO: FPS.
	if (d->osdList.empty()) {
		// OSD is empty.
		d->timer.resetBase();
	}

	// Get the timer now, so we have the same time for
	// each line if this message has multiple lines.
	const uint64_t endTime = d->timer.getTime() + (duration * 1000);

	// Check for newlines.
	const char *start = msg;
	while (start != nullptr) {
		const char *delim = strchr(start, '\n');
		
		OsdGLPrivate::OsdMessage *osdMsg = new OsdGLPrivate::OsdMessage();

		// TODO: Convert msg from UTF-8.
		if (delim == nullptr) {
			// No newlines. Take the whole string.
			osdMsg->msg = d->utf8ToInternal(start, strlen(start));
		} else {
			// Newline found.
			osdMsg->msg = d->utf8ToInternal(start, (delim - start));
		}
		start = (delim ? delim + 1 : nullptr);

		// Check if the end of the string is '\r'
		if (!osdMsg->msg.empty() &&
		    osdMsg->msg.at(osdMsg->msg.size() - 1) == '\r')
		{
			// Found a trailing '\r'.
			// Remove it.
			osdMsg->msg.resize(osdMsg->msg.size() - 1);
		}

		// Fill in the remaining OsdMessage fields.
		osdMsg->duration = duration;
		osdMsg->hasDisplayed = false;
		osdMsg->endTime = endTime;
		d->osdList.push_back(osdMsg);
	}

	// OSD is dirty.
	d->dirty = true;
}

/**
 * Display a preview image.
 * @param duration Duration for the preview image to appear, in milliseconds.
 * @param img_data Image data. (If nullptr, or internal data is nullptr, hide the current image.)
 */
void OsdGL::preview_image(int duration, const Zomg_Img_Data_t *img_data)
{
	if (!img_data || !img_data->data) {
			// Null image. Hide the current preview image.
		if (d->preview.visible) {
			// endTime is adjusted so we can fade it out.
			d->preview.endTime = d->timer.getTime();
			d->preview.hasDisplayed = true;
		}
		// Nothing to do.
		return;
	}

	// TODO: Add Zomg_Img_Data_t support to GLTex.
	// TODO: Strip the alpha channel?

	// Determine the image format.
	GLTex::Format format;
	int pxPitch = img_data->pitch;
	switch (img_data->bpp) {
		case 15:
			format = GLTex::FMT_XRGB1555;
			pxPitch /= 2;
			break;
		case 16:
			format = GLTex::FMT_RGB565;
			pxPitch /= 2;
			break;
		case 32:
			format = GLTex::FMT_XRGB8888;
			pxPitch /= 4;
			break;
		default:
			// Unsupported format.
			d->preview.tex.dealloc();
			return;
	}

	// Allocate the texture.
	d->preview.tex.alloc(format, img_data->w, img_data->h);
	// Upload the image data.
	// NOTE: subImage2D takes pitch in pixels, not bytes.
	d->preview.tex.subImage2D(img_data->w, img_data->h,
				pxPitch, img_data->data);

	// Calculate the texture coordinates.
	const double txcW = d->preview.tex.ratioW();
	const double txcH = d->preview.tex.ratioH();
	GLTex::toCoords<double>(d->preview.txc, 0.0, 0.0, txcW, txcH);

	// Handle certain resolutions with specific aspect ratios.
	// - Always use 320px width.
	// - If W is a multiple of 248, 256, or 320,
	//   and H is a multiple of 192, 224, or 240,
	//   use the base H multiple.
	// - Otherwise, use 240px height.
	// - TODO: Left bar for 248px?)
	// - TODO: Adjust Y position?
	// - TODO: Full-size (x448, x480) IM2 support?
	const int imgW = 320;
	int imgH = 240;
	if (img_data->w % 248 == 0 ||
		img_data->w % 256 == 0 ||
		img_data->w % 320 == 0)
	{
		// Width is an expected size.
		if (img_data->h % 240 == 0) {
			// 240px height.
			imgH = 240;
		} else if (img_data->h % 224 == 0) {
			// 224px height.
			imgH = 224;
		} else if (img_data->h % 192 == 0) {
			// 192px height.
			imgH = 192;
		}
	}

	// Calculate the vertex coordinates.
	// In a 320x240 virtual display, we want the preview to be
	// 5/16ths the size (100x75), and located 16px from the
	// top-right border.
	// TODO: Get actual virtual display coordinates.
	// TODO: Use float or double for vtx in case the values aren't even?
	// TODO: Adjust vertical height if the image isn't exactly 320x240.
	const int dispW = 320;
	//const int dispH = 240;
	const int vtxW = (imgW * 5 / 16), vtxH = (imgH * 5 / 16);
	const int vtxX = dispW - vtxW - 16;
	const int vtxY = 16;
	GLTex::toCoords<int>(d->preview.vtx, vtxX, vtxY, vtxW, vtxH);

	// Border.
	GLTex::toCoords<int>(d->preview.vtx_border, vtxX-1, vtxY-1, vtxW+2, vtxH+2);

	// Drop shadow.
	// TODO: Is 4px the best size?
	for (int i = 0; i < 4; i++) {
		d->preview.vtx_sh[i][0] = d->preview.vtx_border[i][0] + 4;
		d->preview.vtx_sh[i][1] = d->preview.vtx_border[i][1] + 4;
	}

	// Set the display parameters.
	d->preview.duration = duration;
	d->preview.endTime = d->timer.getTime() + (duration * 1000);
	d->preview.visible = true;
	d->preview.hasDisplayed = false;

	// OSD is now dirty.
	d->dirty = true;
}

/** Properties. **/

bool OsdGL::isFpsEnabled(void) const
{
	return d->fpsEnabled;
}

void OsdGL::setFpsEnabled(bool fpsEnabled)
{
	if (d->fpsEnabled == fpsEnabled)
		return;

	d->fpsEnabled = fpsEnabled;
	// TODO: Force dirty...
}

bool OsdGL::isMsgEnabled(void) const
{
	return d->msgEnabled;
}

void OsdGL::setMsgEnabled(bool msgEnabled)
{
	if (d->msgEnabled == msgEnabled)
		return;

	d->msgEnabled = msgEnabled;
	if (!d->osdList.empty()) {
		// OSD messages are present.
		// Mark the OSD as dirty, since the messages
		// need to either be drawn if msgEnabled == true,
		// or hidden if msgEnabled == false.
		d->dirty = true;
	}
}

uint32_t OsdGL::fpsColor(void) const
{
	return d->fpsColor;
}

void OsdGL::setFpsColor(uint32_t fpsColor)
{
	if (d->fpsColor == fpsColor)
		return;

	d->fpsColor = fpsColor;
	// TODO: Force dirty...
}

uint32_t OsdGL::msgColor(void) const
{
	return d->msgColor;
}

void OsdGL::setMsgColor(uint32_t msgColor)
{
	if (d->msgColor == msgColor)
		return;

	d->msgColor = msgColor;
	if (d->msgEnabled && !d->osdList.empty()) {
		// OSD messages are present.
		// Mark the OSD as dirty, since the messages
		// need to be redrawn using the new color.
		d->dirty = true;
	}
}

}
