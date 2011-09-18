/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderManager.hpp: OpenGL Shader Manager.                             *
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

#ifndef __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__
#define __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__

#include <gens-qt4/config.gens-qt4.h>

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

// Qt includes.
#include <QtCore/QStringList>

// Shaders.
#include "GLShaderPaused.hpp"
#include "GLShaderFastBlur.hpp"

namespace GensQt4
{

class GLShaderManager
{
	public:
		GLShaderManager();
		~GLShaderManager();
		
		/**
		 * init(): Initialize the GL Shader Manager.
		 * This must be run from within a valid GL context!
		 */
		void init(void);
		
		/**
		 * end(): Shut down the GL Shader Manager.
		 * This must be run from within a valid GL context!
		 */
		void end(void);
		
		inline bool isInit(void) const
			{ return m_init; }
		
#ifdef HAVE_GLEW
		/**
		 * GLExtsInUse(): Get a list of the OpenGL extensions in use.
		 * @return List of OpenGL extensions in use.
		 */
		static QStringList GLExtsInUse(void);
#endif /* HAVE_GLEW */
		
		/** Query available shaders. **/
#ifdef HAVE_GLEW
		inline bool hasPaused(void) const
			{ return (m_shader_paused.shaderType() != GLShader::ST_NONE); }
		inline bool hasFastBlur(void) const
			{ return (m_shader_fastBlur.shaderType() != GLShader::ST_NONE); }
#else /* !HAVE_GLEW */
		inline bool hasPaused(void) const
			{ return false; }
		inline bool hasFastBlur(void) const
			{ return false; }
#endif /* HAVE_GLEW */
		
		/** Set Shader functions. **/
#ifdef HAVE_GLEW
		void setPaused(bool enabled);
		void setFastBlur(bool enabled);
#else /* !HAVE_GLEW */
		/**
		 * NULL functions that do nothing if GLEW wasn't found at compile time.
		 * These functions are provided for a consistent interface.
		 * TODO: Should these functions be marked as inline?
		 */
		void setPaused(bool enabled)
			{ ((void)enabled); }
		void setFastBlur(bool enabled)
			{ ((void)enabled); }
#endif /* HAVE_GLEW */
	
	protected:
		bool m_init;	// True if the GL Shader Manager is initialized.
		
#ifdef HAVE_GLEW
		/** OpenGL shaders. **/
		GLShaderPaused m_shader_paused;
		GLShaderFastBlur m_shader_fastBlur;
#endif /* HAVE_GLEW */
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__ */
