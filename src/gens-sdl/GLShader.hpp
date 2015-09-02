/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLShader.hpp: OpenGL Shader. (Base Class)                               *
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

#ifndef __GENS_SDL_GLSHADER_HPP__
#define __GENS_SDL_GLSHADER_HPP__

// OpenGL. (GLEW)
#include <GL/glew.h>

// Disabled for now due to problems with Mac OS X 10.5.7 PPC.
// (Radeon 9000 series video card)
//#define ENABLE_ATI_TEXT_FRAGMENT_SHADER

namespace GensSdl {

class GLShader
{
	public:
		GLShader();
		virtual ~GLShader();

		/**
		 * Initialize the shader.
		 * This must be run from within a valid GL context!
		 * @return 0 on success; non-zero on error.
		 */
		virtual int init(void) = 0;

		/**
		 * Shut down the shader.
		 * This must be run from within a valid GL context!
		 * @return 0 on success; non-zero on error.
		 */
		virtual int end(void);

		/**
		 * Enable the shader.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int enable(void);

		/**
		 * Disable the shader.
		 * @return 0 on success; non-zero on error.
		 */
		int disable(void);

		/**
		 * GL Shader type.
		 * Indicates the type of shader in use.
		 */
		enum GLShaderType {
			ST_NONE				= 0,
			ST_GL_ARB_FRAGMENT_PROGRAM,
			ST_GL_ATI_FRAGMENT_SHADER,
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			ST_GL_ATI_TEXT_FRAGMENT_SHADER,
#endif

			ST_MAX
		};

		/**
		 * Get the GL shader type.
		 * @return Shader type.
		 */
		inline GLShaderType shaderType(void) const;

		/**
		 * Is the shader usable?
		 * @return True if usable; false if not.
		 */
		inline bool isUsable(void) const;

	private:
		// Shader type.
		GLShaderType m_shaderType;

	protected:
		/**
		 * Set the shader type.
		 * @param shaderType New shader type.
		 */
		inline void setShaderType(GLShaderType shaderType);

		// Shader name.
		// Type of object depends on m_shaderType.
		// TODO: Make private with accessors, like m_shaderType?
		GLuint m_shaderName;
};

/**
 * Get the GL shader type.
 * @return Shader type.
 */
inline GLShader::GLShaderType GLShader::shaderType(void) const
	{ return m_shaderType; }

/**
 * Set the GL shader type.
 * @param shaderType New shader type.
 */
inline void GLShader::setShaderType(GLShaderType shaderType)
{
	// TODO: If m_shaderName != 0, deallocate it.
	m_shaderType = shaderType;
}

/**
 * Is the shader usable?
 * @return True if usable; false if not.
 */
inline bool GLShader::isUsable(void) const
{
	// NOTE: Only checking m_shaderType.
	// TODO: Also check m_shaderName != 0?
	return (m_shaderType != ST_NONE);
}

}

#endif /* __GENS_SDL_GLSHADER_HPP__ */
