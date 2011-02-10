/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderManager.hpp: OpenGL Shader Manager.                             *
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

#ifndef __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__
#define __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__

#include <config.h>

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

// Qt includes.
#include <QtCore/QStringList>

// Disabled for now due to problems with Mac OS X 10.5.7 PPC.
// (Radeon 9000 series video card)
//#define ENABLE_ATI_TEXT_FRAGMENT_SHADER

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
			{ return (m_paused_type != ST_NONE); }
		inline bool hasFastBlur(void) const
			{ return (m_fastBlur_ARB != 0); }
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
		void setPaused(bool enabled) { ((void)enabled); }
		void setFastBlur(bool enabled) { ((void)enabled); }
#endif /* HAVE_GLEW */
	
	protected:
		bool m_init;	// True if the GL Shader Manager is initialized.
		
		enum ShaderType
		{
			ST_NONE				= 0,
			ST_GL_ARB_FRAGMENT_PROGRAM	= 1,
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			ST_GL_ATI_TEXT_FRAGMENT_SHADER	= 2,
#endif
			
			ST_MAX
		};
		
		/** OpenGL shaders. **/
		
#ifdef HAVE_GLEW
		// Paused effect.
		bool m_paused_enabled;
		ShaderType m_paused_type;
		GLuint m_paused_ARB;
		static const char *ms_Paused_ARB_fragment_program_src;
#if ENABLE_ATI_TEXT_FRAGMENT_SHADER
		static const char *ms_Paused_ATI_text_fragment_shader_src;
#endif
		
		// Fast Blur effect.
		bool m_fastBlur_enabled;
		ShaderType m_fastBlur_type;
		GLuint m_fastBlur_ARB;
		static const char *ms_FastBlur_ARB_fragment_program_src;
#endif /* HAVE_GLEW */
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__ */
