/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShader.hpp: OpenGL Shader. (Base Class)                               *
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

#ifndef __GENS_QT4_VBACKEND_GLSHADER_HPP__
#define __GENS_QT4_VBACKEND_GLSHADER_HPP__

#include <config.h>

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

// Disabled for now due to problems with Mac OS X 10.5.7 PPC.
// (Radeon 9000 series video card)
//#define ENABLE_ATI_TEXT_FRAGMENT_SHADER

namespace GensQt4
{

class GLShader
{
	public:
		GLShader();
		virtual ~GLShader();
		
		/**
		 * init(): Initialize the shader.
		 * This must be run from within a valid GL context!
		 */
		virtual void init(void) = 0;
		
		/**
		 * end(): Shut down the shader.
		 * This must be run from within a valid GL context!
		 */
		virtual void end(void);
		
		/** TODO: Separate enable() / disable(), or a single setEnabled()? **/
		
		/**
		 * enable(): Enable the shader.
		 */
		virtual void enable(void);
		
		/**
		 * disable(): Disable the shader.
		 */
		void disable(void);
		
		enum GLShaderType
		{
			ST_NONE				= 0,
			ST_GL_ARB_FRAGMENT_PROGRAM,
			ST_GL_ATI_FRAGMENT_SHADER,
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			ST_GL_ATI_TEXT_FRAGMENT_SHADER,
#endif
			
			ST_MAX
		};
		
		/**
		 * shaderType(): Determine the type of shader.
		 * @return Shader type.
		 */
		GLShaderType shaderType(void) const
			{ return m_shaderType; }
	
	protected:
		/** Shader type. **/
		void setShaderType(GLShaderType newShaderType)
			{ m_shaderType = newShaderType; }
		
#ifdef HAVE_GLEW
		/** OpenGL shader variables. **/
		// (GLEW is required.)
		
		/**
		 * m_ARB_program: ID of either GL_ARB_vertex_program or GL_ARB_fragment_program.
		 */
		GLuint m_ARB_program;
		
		/**
		 * m_ATI_fragment_shader: ID of GL_ATI_fragment_shader.
		 */
		GLuint m_ATI_fragment_shader;
#endif /* HAVE_GLEW */
	
	private:
		/** Shader type. **/
		GLShaderType m_shaderType;
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADER_HPP__ */
