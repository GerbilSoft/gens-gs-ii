/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderManager.cpp: OpenGL Shader Manager.                             *
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

#include "GLShaderManager.hpp"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

// C includes.
#include <string.h>

namespace GensQt4
{

GLShaderManager::GLShaderManager()
{
	m_init = false;
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
	
	// Initialize shaders.
	m_shader_paused.init();
	m_shader_fastBlur.init();
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
	// Shut down the shaders.
	m_shader_paused.end();
	m_shader_fastBlur.end();
#endif /* HAVE_GLEW */
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
	else if (GLEW_ATI_fragment_shader)
		exts.append(QLatin1String("GL_ATI_fragment_shader"));
	
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
	if (newEnabled)
		m_shader_paused.enable();
	else
		m_shader_paused.disable();
}


/**
 * setFastBlur(): Set the Fast Blur shader.
 * @param enabled True to enable; false to disable.
 */
void GLShaderManager::setFastBlur(bool newEnabled)
{
	// TODO: Check if any other shaders are enabled?
	if (newEnabled)
		m_shader_fastBlur.enable();
	else
		m_shader_fastBlur.disable();
}
#endif

}
