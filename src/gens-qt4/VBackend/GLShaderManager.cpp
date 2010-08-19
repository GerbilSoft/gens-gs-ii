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
 */
const char *GLShaderManager::ms_Paused_ARBfrag_asm =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"PARAM grayscale = {0.30, 0.59, 0.11, 0.0};\n"		// Standard RGB to Grayscale algorithm.
	"TEMP t0, color;\n"
	"TEX t0, fragment.texcoord[0], texture[0], 2D;\n"	// Get color coordinate.
	"DP3 color, t0, grayscale;\n"				// Calculate grayscale value.
	"ADD_SAT color.z, color.z, color.z;\n"			// Double the blue component.
	"MOV result.color, color;\n"
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
		
		GLenum err = glGetError();
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

}
