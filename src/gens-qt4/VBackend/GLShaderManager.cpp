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
 * ms_Paused_ARBfrag_asm(): Paused effect shader. (GL_ARB_fragment_program)
 * Based on grayscale shader from http://arstechnica.com/civis/viewtopic.php?f=19&t=445912
 *
 * Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
 * Source: http://en.wikipedia.org/wiki/YCbCr
 */
const char *GLShaderManager::ms_Paused_ARBfrag_asm =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"PARAM grayscale = {0.299, 0.587, 0.114, 0.0};\n"	// Standard RGB to Grayscale vector.
	"TEMP t0, color;\n"
	"TEX t0, fragment.texcoord[0], texture[0], 2D;\n"	// Get color coordinate.
	"DP3 color, t0, grayscale;\n"				// Calculate grayscale value.
	"ADD_SAT color.z, color.z, color.z;\n"			// Double the blue component.
	"MOV result.color, color;\n"
	"END\n";

/**
 * ms_FastBlur_ARBfrag_asm(): Fast Blur effect shader. (GL_ARB_fragment_program)
 */
const char *GLShaderManager::ms_FastBlur_ARBfrag_asm =
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
	m_paused_ARBfrag = 0;
	m_paused_enabled = false;
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
	// Initialize GLEW.
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		// Error initializing GLEW.
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"Error initializing GLEW: %s", glewGetErrorString(err));
	}
	
	// Check what extensions are supported.
	if (GLEW_ARB_fragment_program)
	{
		// Fragment programs are supported.
		// Load the fragment programs.
		
		// Load the Paused effect fragment program.
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_paused_ARBfrag);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_paused_ARBfrag);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
				   strlen(ms_Paused_ARBfrag_asm), ms_Paused_ARBfrag_asm);
		
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Paused effect FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_paused_ARBfrag > 0)
			{
				glDeleteProgramsARB(1, &m_paused_ARBfrag);
				m_paused_ARBfrag = 0;
			}
		}
		
		// Load the Fast Blur effect fragment program.
		// TODO: Check for errors!
		glGenProgramsARB(1, &m_fastBlur_ARBfrag);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fastBlur_ARBfrag);
		glGetError();	// Clear the error flag.
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
				   strlen(ms_FastBlur_ARBfrag_asm), ms_FastBlur_ARBfrag_asm);
		
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			// An error occured while loading the fragment program.
			// TODO: Remove the extra newline at the end of err_str.
			const char *err_str = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Error creating Fast Blur effect FP: %s", (err_str ? err_str : "(unknown)"));
			
			// Delete the fragment program.
			if (m_fastBlur_ARBfrag > 0)
			{
				glDeleteProgramsARB(1, &m_fastBlur_ARBfrag);
				m_fastBlur_ARBfrag = 0;
			}
		}
	}
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
	if (m_paused_ARBfrag > 0)
	{
		glDeleteProgramsARB(1, &m_paused_ARBfrag);
		m_paused_ARBfrag = 0;
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
	
	if (GLEW_ARB_fragment_program)
		exts.append(QLatin1String("GL_ARB_fragment_program"));
	
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
	if (m_paused_ARBfrag == 0 || newEnabled == m_paused_enabled)
		return;
	
	m_paused_enabled = newEnabled;
	if (m_paused_enabled)
	{
		// Enable the ARB fragment program.
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_paused_ARBfrag);
	}
	else
	{
		// Disable the ARB fragment program.
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}


/**
 * setFastBlur(): Set the Fast Blur shader.
 * @param enabled True to enable; false to disable.
 */
void GLShaderManager::setFastBlur(bool newEnabled)
{
	// TODO: Check if any other shaders are enabled?
	if (m_fastBlur_ARBfrag == 0 || newEnabled == m_fastBlur_ARBfrag)
		return;
	
	m_fastBlur_enabled = newEnabled;
	if (m_fastBlur_enabled)
	{
		// Enable the ARB fragment program.
		// TODO: Get texture width for the parameter!
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, (1/512.0f), 0, 0, 0);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fastBlur_ARBfrag);
	}
	else
	{
		// Disable the ARB fragment program.
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}
#endif

}
