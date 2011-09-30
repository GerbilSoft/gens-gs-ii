/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderFastBlur.cpp: OpenGL Shader. (Fast Blur)                        *
 *                                                                         *
 * Copyright (c) 2010-2011 by David Korth.                                 *
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

// C includes.
#include <string.h>

namespace GensQt4
{

#ifdef HAVE_GLEW
/** Shaders. **/

/**
 * ms_GL_ARB_fragment_program: GL_ARB_fragment_program.
 * Based on GLSL code by Damizean.
 */
const char *const GLShaderFastBlur::ms_GL_ARB_fragment_program =
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

#endif /* HAVE_GLEW */

void GLShaderFastBlur::init()
{
#ifdef HAVE_GLEW
	GLenum err;
	
	if (GLEW_ARB_fragment_program || GLEW_VERSION_2_0)
	{
		// GL_ARB_fragment_program is supported.
		// Load the Paused effect fragment program.
		
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_ARB_program);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_ARB_program);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(ms_GL_ARB_fragment_program),
					ms_GL_ARB_fragment_program);
		
		err = glGetError();
		if (err == GL_NO_ERROR)
		{
			// Fragment program loaded.
			setShaderType(ST_GL_ARB_FRAGMENT_PROGRAM);
		}
		else
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Fast Blur effect ARB FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_ARB_program > 0)
			{
				glDeleteProgramsARB(1, &m_ARB_program);
				m_ARB_program = 0;
				setShaderType(ST_NONE);
			}
		}
	}
#endif /* HAVE_GLEW */
}


/**
 * enable(): Enable the shader.
 */
void GLShaderFastBlur::enable(void)
{
	if (shaderType() != ST_GL_ARB_FRAGMENT_PROGRAM)
		return;
	if (m_ARB_program <= 0)
		return;
	
	// Enable the shader.
	// TODO: Get texture width for the parameter!
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, (1/512.0f), 0, 0, 0);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_ARB_program);
}

}
