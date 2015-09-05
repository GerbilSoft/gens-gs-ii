/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShader.cpp: OpenGL Shader. (Base Class)                               *
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

#include "GLShader.hpp"

namespace GensQt4 {

GLShader::GLShader()
{
	// Clear all the variables.
	m_shaderType = ST_NONE;

#ifdef HAVE_GLEW
	// OpenGL shader variables.
	m_ARB_program = 0;
#endif /* HAVE_GLEW */
}

GLShader::~GLShader()
{
	end();
}

/**
 * Shut down the shader.
 * This must be run from within a valid GL context!
 */
void GLShader::end(void)
{
	switch (m_shaderType) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif
			// Delete the ARB program.
			if (m_ARB_program > 0)
				glDeleteProgramsARB(1, &m_ARB_program);
			break;

		case ST_GL_ATI_FRAGMENT_SHADER:
			if (m_ATI_fragment_shader > 0)
				glDeleteFragmentShaderATI(m_ATI_fragment_shader);
			break;

		default:
			break;
	}
}

/**
 * Enable the shader.
 */
void GLShader::enable(void)
{
	switch (shaderType()) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
		{
			if (m_ARB_program == 0)
				break;

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			const GLenum prgType = (shaderType() == ST_GL_ARB_FRAGMENT_PROGRAM
						? GL_FRAGMENT_PROGRAM_ARB
						: GL_TEXT_FRAGMENT_SHADER_ATI
						);
#else /* !ENABLE_ATI_TEXT_FRAGMENT_SHADER */
			const GLenum prgType = GL_FRAGMENT_PROGRAM_ARB;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

			// Enable the shader.
			glEnable(prgType);
			glBindProgramARB(prgType, m_ARB_program);
			break;
		}

		case ST_GL_ATI_FRAGMENT_SHADER:
			if (m_ATI_fragment_shader == 0)
				break;

			glEnable(GL_FRAGMENT_SHADER_ATI);
			glBindFragmentShaderATI(m_ATI_fragment_shader);
			break;

		default:
			break;
	}
}

/**
 * Disable the shader.
 */
void GLShader::disable(void)
{
	switch (shaderType()) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
		{
			if (m_ARB_program <= 0)
				break;

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			const GLenum prgType = (shaderType() == ST_GL_ARB_FRAGMENT_PROGRAM
						? GL_FRAGMENT_PROGRAM_ARB
						: GL_TEXT_FRAGMENT_SHADER_ATI
						);
#else /* !ENABLE_ATI_TEXT_FRAGMENT_SHADER */
			const GLenum prgType = GL_FRAGMENT_PROGRAM_ARB;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

			// Disable the shader.
			glDisable(prgType);
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
			if (shaderType() == ST_GL_ATI_TEXT_FRAGMENT_SHADER) {
				// HACK: at least the Mac OS X 10.5 PPC Radeon drivers are broken and
				// without this disable the texture units while the program is still
				// running (10.4 PPC seems to work without this though).
				// Reference: http://git.mplayerhq.hu/?p=mplayer;a=blob;f=libvo/gl_common.c;hb=HEAD
				// NOTE: It doesn't seem to help any...
				glFlush();
			}
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
			break;
		}

		case ST_GL_ATI_FRAGMENT_SHADER:
			if (m_ATI_fragment_shader == 0)
				break;

			glDisable(GL_FRAGMENT_SHADER_ATI);
			break;

		default:
			break;
	}
}

}
