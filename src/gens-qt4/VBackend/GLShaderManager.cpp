/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderManager.cpp: OpenGL Shader Manager.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "GLShaderManager.hpp"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

// C includes.
#include <string.h>

namespace GensQt4
{

#ifdef HAVE_GLEW
/** Fragment programs. **/

/**
 * ms_Paused_ARB_fragment_program_src: Paused effect shader.
 * GL_ARB_fragment_program
 * Based on grayscale shader from http://arstechnica.com/civis/viewtopic.php?f=19&t=445912
 *
 * Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
 * Source: http://en.wikipedia.org/wiki/YCbCr
 */
const char *GLShaderManager::ms_Paused_ARB_fragment_program_src =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"PARAM grayscale = {0.299, 0.587, 0.114, 0.0};\n"	// Standard RGB to Grayscale vector.
	"TEMP t0, color;\n"
	"TEX t0, fragment.texcoord[0], texture[0], 2D;\n"	// Sample the texture.
	"DP3 color, t0, grayscale;\n"				// Calculate grayscale value.
	"ADD_SAT color.z, color.z, color.z;\n"			// Double the blue component.
	"MOV result.color, color;\n"
	"END\n";

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
/**
 * ms_Paused_ATI_text_fragment_shader_src(): Paused effect shader.
 * GL_ATI_text_fragment_shader
 * Based on grayscale shader from http://arstechnica.com/civis/viewtopic.php?f=19&t=445912
 *
 * Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
 * Source: http://en.wikipedia.org/wiki/YCbCr
 */
const char *GLShaderManager::ms_Paused_ATI_text_fragment_shader_src =
	"!!ATIfs1.0\n"
	"StartConstants;\n"
	"  CONSTANT c0 = {0.299, 0.587, 0.114};\n"	// Standard RGB to Grayscale vector.
	"EndConstants;\n"
	"StartOutputPass;\n"
	"  SampleMap r0, t0.str;\n"			// Sample the texture.
	"  DOT3 r0.rgb, r0, c0;\n"			// Calculate grayscale value.
	"  ADD r0.b.sat, r0.b, r0.b;\n"			// Double the blue component.
	"EndPass;\n";
#endif

/**
 * ms_FastBlur_ARB_fragment_program_src(): Fast Blur effect shader.
 * GL_ARB_fragment_program
 */
const char *GLShaderManager::ms_FastBlur_ARB_fragment_program_src =
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


GLShaderManager::GLShaderManager()
{
	m_init = false;
	
#ifdef HAVE_GLEW
	// Initialize shader variables.
	m_paused_enabled = false;
	m_paused_type = ST_NONE;
	
	m_fastBlur_enabled = false;
	m_fastBlur_type = ST_NONE;
	
	// Clear the shader pointers.
	m_paused_ARB = 0;
	m_fastBlur_ARB = 0;
#endif
}


GLShaderManager::~GLShaderManager()
{
	if (m_init)
		end();
}


/**
 * init(): Initialize the GL Shader Manager.
 * This must be run from within a valid GL context!
 */
void GLShaderManager::init(void)
{
#ifdef HAVE_GLEW
	// NOTE: GLEW must have been initialized previously.
	GLenum err;
	
	// Check what extensions are supported.
	if (GLEW_ARB_fragment_program || GLEW_VERSION_2_0)
	{
		// ARB fragment programs are supported.
		// Load the fragment programs.
		
		// Load the Paused effect fragment program.
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_paused_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_paused_ARB);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(ms_Paused_ARB_fragment_program_src),
					ms_Paused_ARB_fragment_program_src);
		
		err = glGetError();
		if (err == GL_NO_ERROR)
		{
			// Fragment program loaded.
			m_paused_type = ST_GL_ARB_FRAGMENT_PROGRAM;
		}
		else
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Paused effect ARB FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_paused_ARB > 0)
			{
				glDeleteProgramsARB(1, &m_paused_ARB);
				m_paused_ARB = 0;
				m_paused_type = ST_NONE;
			}
		}
		
		// Load the Fast Blur effect fragment program.
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_fastBlur_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fastBlur_ARB);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(ms_FastBlur_ARB_fragment_program_src),
					ms_FastBlur_ARB_fragment_program_src);
		
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Fast Blur effect ARB FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_fastBlur_ARB > 0)
			{
				glDeleteProgramsARB(1, &m_fastBlur_ARB);
				m_fastBlur_ARB = 0;
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
		glGenProgramsARB(1, &m_paused_ARB);
		glBindProgramARB(GL_TEXT_FRAGMENT_SHADER_ATI, m_paused_ARB);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_TEXT_FRAGMENT_SHADER_ATI, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(ms_Paused_ATI_text_fragment_shader_src),
					ms_Paused_ATI_text_fragment_shader_src);
		
		err = glGetError();
		if (err == GL_NO_ERROR)
		{
			// Fragment program loaded.
			m_paused_type = ST_GL_ATI_TEXT_FRAGMENT_SHADER;
		}
		else
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Paused effect ATI TEXT FS: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_paused_ARB > 0)
			{
				glDeleteProgramsARB(1, &m_paused_ARB);
				m_paused_ARB = 0;
				m_paused_type = ST_NONE;
			}
		}
	}
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
#endif /* HAVE_GLEW */
	
	// OpenGL Shader Manager is initialized.
	m_init = true;
}


/**
 * end(): Shut down the GL Shader Manager.
 * This must be run from within a valid GL context!
 */
void GLShaderManager::end(void)
{
	if (!m_init)
		return;
	
	// Shut down the OpenGL Shader Manager.
	
#ifdef HAVE_GLEW
	// Delete any allocated shaders.
	switch (m_paused_type)
	{
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif
			if (m_paused_ARB > 0)
			{
				glDeleteProgramsARB(1, &m_paused_ARB);
				m_paused_ARB = 0;
			}
			break;
		
		default:
			break;
	}
	
	switch (m_fastBlur_type)
	{
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif
			if (m_fastBlur_ARB > 0)
			{
				glDeleteProgramsARB(1, &m_fastBlur_ARB);
				m_fastBlur_ARB = 0;
			}
			break;
		
		default:
			break;
	}
#endif
	
	// OpenGL Shader Manager is shut down.
	m_init = false;
}


#ifdef HAVE_GLEW
/**
 * GLExtsInUse(): Get a list of the OpenGL extensions in use.
 * @return List of OpenGL extensions in use.
 */
QStringList GLShaderManager::GLExtsInUse(void)
{
	QStringList exts;
	
	if (GLEW_ARB_fragment_program || GLEW_VERSION_2_0)
		exts.append(QLatin1String("GL_ARB_fragment_program"));
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
	else if (GLEW_ATI_text_fragment_shader)
		exts.append(QLatin1String("GL_ATI_text_fragment_shader"));
#endif
	
	// TODO: Other GL extensions.
#if 0
	if (GLEW_ARB_vertex_program)
		exts.append(QLatin1String("GL_ARB_vertex_program"));
	
	if (GLEW_ARB_vertex_shader)
		exts.append(QLatin1String("GL_ARB_vertex_shader"));
	else if (GLEW_EXT_vertex_shader)
		exts.append(QLatin1String("GL_EXT_vertex_shader"));
	
	if (GLEW_ARB_fragment_shader)
		exts.append(QLatin1String("GL_ARB_fragment_shader"));
	if (GLEW_ARB_fragment_shader)
		exts.append(QLatin1String("GL_ARB_fragment_shader"));
	if (GLEW_ARB_shading_language_100)
		exts.append(QLatin1String("GL_ARB_shading_language_100"));
	if (GLEW_ARB_shader_objects)
		exts.append(QLatin1String("GL_ARB_shader_objects"));
#endif
	
	// Return the list of extensions.
	return exts;
}
#endif /* HAVE_GLEW */


/** Set Shader functions. **/


#ifdef HAVE_GLEW
/**
 * setPaused(): Set the Paused shader.
 * @param enabled True to enable; false to disable.
 */
void GLShaderManager::setPaused(bool newEnabled)
{
	// TODO: Check if any other shaders are enabled?
	if (!m_init)
		return;
	if (newEnabled == m_paused_enabled)
		return;
	
	switch (m_paused_type)
	{
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif
		{
			if (m_paused_ARB == 0)
				break;
			
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			const GLenum prgType = (m_paused_type == ST_GL_ARB_FRAGMENT_PROGRAM
						? GL_FRAGMENT_PROGRAM_ARB
						: GL_TEXT_FRAGMENT_SHADER_ATI
						);
#else
			const GLenum prgType = GL_FRAGMENT_PROGRAM_ARB;
#endif
			
			m_paused_enabled = newEnabled;
			if (m_paused_enabled)
			{
				// Enable the shader.
				glEnable(prgType);
				glBindProgramARB(prgType, m_paused_ARB);
			}
			else
			{
				// Disable the shader.
				glDisable(prgType);
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
				if (m_paused_type == ST_GL_ATI_TEXT_FRAGMENT_SHADER)
				{
					// HACK: at least the Mac OS X 10.5 PPC Radeon drivers are broken and
					// without this disable the texture units while the program is still
					// running (10.4 PPC seems to work without this though).
					// Reference: http://git.mplayerhq.hu/?p=mplayer;a=blob;f=libvo/gl_common.c;hb=HEAD
					// NOTE: It doesn't seem to help any...
					glFlush();
				}
#endif
			}
			break;
		}
		
		default:
			break;
	}
}


/**
 * setFastBlur(): Set the Fast Blur shader.
 * @param enabled True to enable; false to disable.
 */
void GLShaderManager::setFastBlur(bool newEnabled)
{
	// TODO: Check if any other shaders are enabled?
	if (!m_init)
		return;
	if (newEnabled == m_paused_enabled)
		return;
	
	if (!m_init)
		return;
	if (m_fastBlur_ARB == 0 || newEnabled == m_fastBlur_ARB)
		return;
	
	switch (m_paused_type)
	{
		case ST_GL_ARB_FRAGMENT_PROGRAM:
			if (m_fastBlur_ARB == 0)
				break;
			
			m_fastBlur_enabled = newEnabled;
			if (m_fastBlur_enabled)
			{
				// Enable the ARB fragment program.
				// TODO: Get texture width for the parameter!
				glEnable(GL_FRAGMENT_PROGRAM_ARB);
				glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, (1/512.0f), 0, 0, 0);
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fastBlur_ARB);
			}
			else
			{
				// Disable the ARB fragment program.
				glDisable(GL_FRAGMENT_PROGRAM_ARB);
			}
			break;
		
		default:
			break;
	}
}
#endif

}
