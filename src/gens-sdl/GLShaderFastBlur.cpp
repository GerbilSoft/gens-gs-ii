/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLShaderFastBlur.cpp: OpenGL Shader. (Fast Blur)                        *
 *                                                                         *
 * Copyright (c) 2010-2015 by David Korth.                                 *
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

#include "GLShaderFastBlur.hpp"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

namespace GensSdl {

class GLShaderFastBlurPrivate {
	private:
		GLShaderFastBlurPrivate() { }
		~GLShaderFastBlurPrivate() { }
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		GLShaderFastBlurPrivate(const GLShaderFastBlurPrivate &);
		GLShaderFastBlurPrivate &operator=(const GLShaderFastBlurPrivate &);

	public:
		// GL_ARB_fragment_program implementation.
		static const char FastBlur_ARB_fragment_program[];
};

/** Shaders. **/

/**
 * GL_ARB_fragment_program implementation.
 * Based on GLSL code by Damizean.
 */
const char GLShaderFastBlurPrivate::FastBlur_ARB_fragment_program[] =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"PARAM div2 = {0.5, 0.5, 0.5, 0.0};\n"			// Divide by 2 vector.
	"PARAM offset = program.local[0];\n"			// Texture offset vector.
	"TEMP t0, tx1, t1;\n"
	"TEX t0, fragment.texcoord[0], texture[0], 2D;\n"	// Get first pixel.
	"ADD tx1, fragment.texcoord[0], offset;\n"		// Calculate offset of next pixel.
	"TEX t1, tx1, texture[0], 2D;\n"			// Get second pixel.
	"ADD t0, t0, t1;\n"					// Add the two colors together.
	"MUL result.color, t0, div2;\n"				// Divide the result by two.
	"END\n";

/**
 * Initialize the shader.
 * This must be run from within a valid GL context!
 * @return 0 on success; non-zero on error.
 */
int GLShaderFastBlur::init()
{
	if (!GLEW_VERSION_2_0 && !GLEW_ARB_fragment_program) {
		// GL_ARB_fragment_program is NOT supported.
		// TODO: POSIX error code?
		return -1;
	}

	// Load the Fast Blur fragment program.
	int ret = 0;

	// TODO: Check for errors!
	GLuint shaderName;
	glGenProgramsARB(1, &shaderName);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, shaderName);
	glGetError();	// Clear the error flag.
	// NOTE: sizeof(str)-1 - NVIDIA's Windows driver doesn't like
	// the NULL terminator, and refuses to compile the shader if
	// it's included. Mesa's r300g driver doesn't care if the NULL
	// terminator is present.
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
				sizeof(GLShaderFastBlurPrivate::FastBlur_ARB_fragment_program)-1,
				GLShaderFastBlurPrivate::FastBlur_ARB_fragment_program);

	GLenum err = glGetError();
	if (err == GL_NO_ERROR) {
		// Fragment program loaded.
		m_shaderName = shaderName;
		setShaderType(ST_GL_ARB_FRAGMENT_PROGRAM);
	} else {
		// An error occured while loading the fragment program.
		// TODO: Remove the extra newline at the end of err_str.
		// TODO: POSIX error code?
		ret = -1;
		const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"Error creating Fast Blur effect ARB FP: %s", (err_str ? err_str : "(unknown)"));

		// Delete the fragment program.
		if (shaderName > 0) {
			glDeleteProgramsARB(1, &shaderName);
			m_shaderName = 0;
			setShaderType(ST_NONE);
		}
	}

	return ret;
}


/**
 * Enable the shader.
 * @return 0 on success; non-zero on error.
 */
int GLShaderFastBlur::enable(void)
{
	if (shaderType() != ST_GL_ARB_FRAGMENT_PROGRAM ||
	    m_shaderName == 0)
	{
		// Shader has not been loaded.
		// TODO: POSIX error code?
		return -1;
	}

	// Enable the shader.
	// TODO: Get texture width for the parameter!
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, (1/512.0f), 0, 0, 0);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_shaderName);
	return 0;
}

}
