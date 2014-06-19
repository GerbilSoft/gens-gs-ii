/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderPaused.cpp: OpenGL Shader. (Paused Effect)                      *
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

#include "GLShaderPaused.hpp"

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
 * Based on grayscale shader from http://arstechnica.com/civis/viewtopic.php?f=19&t=445912
 *
 * Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
 * Source: http://en.wikipedia.org/wiki/YCbCr
 */
const char *const GLShaderPaused::ms_GL_ARB_fragment_program =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"PARAM grayscale = {0.299, 0.587, 0.114, 0.0};\n"	// Standard RGB to Grayscale vector.
	"TEMP t0, color;\n"
	"TEX t0, fragment.texcoord[0], texture[0], 2D;\n"	// Sample the texture.
	"DP3 color, t0, grayscale;\n"				// Calculate grayscale value.
	"ADD_SAT color.b, color.b, color.b;\n"			// Double the blue component.
	"MOV result.color, color;\n"
	"END\n";

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
/**
 * ms_GL_ATI_text_fragment_shader: GL_ATI_text_fragment_shader.
 * Based on grayscale shader from http://arstechnica.com/civis/viewtopic.php?f=19&t=445912
 *
 * Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
 * Source: http://en.wikipedia.org/wiki/YCbCr
 */
const char *const GLShaderPaused::ms_GL_ATI_text_fragment_shader =
	"!!ATIfs1.0\n"
	"StartConstants;\n"
	"  CONSTANT c0 = {0.299, 0.587, 0.114};\n"	// Standard RGB to Grayscale vector.
	"EndConstants;\n"
	"StartOutputPass;\n"
	"  SampleMap r0, t0.str;\n"			// Sample the texture.
	"  DOT3 r0.rgb, r0, c0;\n"			// Calculate grayscale value.
	"  ADD r0.b.sat, r0.b, r0.b;\n"			// Double the blue component.
	"EndPass;\n";
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

#endif /* HAVE_GLEW */

void GLShaderPaused::init()
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
				"Error creating Paused effect ARB FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_ARB_program > 0)
			{
				glDeleteProgramsARB(1, &m_ARB_program);
				m_ARB_program = 0;
				setShaderType(ST_NONE);
			}
		}
	}
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
	else if (GLEW_ATI_text_fragment_shader)
	{
		// ATI text fragment shaders are supported
		// Load the fragment shaders.
		// TODO: Combine with ARB fragment programs code above.
		// The code is the same, but it uses different values and strings.
		
		// Load the Paused effect fragment shader.
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_ARB_program);
		glBindProgramARB(GL_TEXT_FRAGMENT_SHADER_ATI, m_ARB_program);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_TEXT_FRAGMENT_SHADER_ATI, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(ms_GL_ATI_text_fragment_shader),
					ms_GL_ATI_text_fragment_shader);
		
		err = glGetError();
		if (err == GL_NO_ERROR)
		{
			// Fragment program loaded.
			setShaderType(ST_GL_ATI_TEXT_FRAGMENT_SHADER);
		}
		else
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Paused effect ATI TEXT FS: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_ARB_program > 0)
			{
				glDeleteProgramsARB(1, &m_ARB_program);
				m_ARB_program = 0;
				setShaderType(ST_NONE);
			}
		}
	}
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
	else if (GLEW_ATI_fragment_shader)
	{
		// ATI fragment shaders are supported.
		m_ATI_fragment_shader = glGenFragmentShadersATI(1);
		glBindFragmentShaderATI(m_ATI_fragment_shader);
		glBeginFragmentShaderATI();
		
		// Constants.
		static const float RGBtoGrayscale[4] = {0.299f, 0.587f, 0.114f, 0.0f};
		glSetFragmentShaderConstantATI(GL_CON_0_ATI, RGBtoGrayscale);
		
		// SampleMap r0, t0.str;
		glSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
		
		// DOT3 r0.rgb, r0, c0;
		glColorFragmentOp2ATI(GL_DOT3_ATI,
				GL_REG_0_ATI, (GL_RED_BIT_ATI | GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI), GL_NONE,
				GL_REG_0_ATI, GL_NONE, GL_NONE,
				GL_CON_0_ATI, GL_NONE, GL_NONE);
		
		// ADD r0.b.sat, r0.b, r0.b;
		glColorFragmentOp2ATI(GL_ADD_ATI,
				GL_REG_0_ATI, GL_BLUE_BIT_ATI, GL_SATURATE_BIT_ATI,
				GL_REG_0_ATI, GL_BLUE_BIT_ATI, GL_NONE,
				GL_REG_0_ATI, GL_BLUE_BIT_ATI, GL_NONE);
		
		glEndFragmentShaderATI();
		
		// Set the shader type.
		setShaderType(ST_GL_ATI_FRAGMENT_SHADER);
	}
#endif /* HAVE_GLEW */
}

}


