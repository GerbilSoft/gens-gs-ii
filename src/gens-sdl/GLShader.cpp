/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
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

namespace GensSdl {

GLShader::GLShader()
	: m_shaderType(ST_NONE)
	, m_shaderName(0)
{ }

GLShader::~GLShader()
{
	end();
}


/**
 * Shut down the shader.
 * This must be run from within a valid GL context!
 * @return 0 on success; non-zero on error.
 */
int GLShader::end(void)
{
	if (m_shaderName == 0) {
		// No shader.
		// TODO: POSIX error code?
		return -1;
	}

	int ret = 0;
	switch (m_shaderType) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
			glDeleteProgramsARB(1, &m_shaderName);
			break;

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
			// GL_ATI_text_fragment_shader uses the same
			// infrastructure as ARB programs.
			glDeleteProgramsARB(1, &m_shaderName);
			break;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

		case ST_GL_ATI_FRAGMENT_SHADER:
			glDeleteFragmentShaderATI(m_shaderName);
			break;

		default:
			ret = -1;
			break;
	}

	// Shader has been deleted.
	m_shaderType = ST_NONE;
	m_shaderName = 0;
	return ret;
}

/**
 * Enable the shader.
 * @return 0 on success; non-zero on error.
 */
int GLShader::enable(void)
{
	if (m_shaderName == 0) {
		// No shader has been compiled.
		// TODO: POSIX error code?
		return -1;
	}

	int ret = 0;
	switch (m_shaderType) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_shaderName);
			break;

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
			// GL_ATI_text_fragment_shader uses the same
			// infrastructure as ARB programs.
			glEnable(GL_TEXT_FRAGMENT_SHADER_ATI);
			glBindProgramARB(GL_TEXT_FRAGMENT_SHADER_ATI, m_shaderName);
			break;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

		case ST_GL_ATI_FRAGMENT_SHADER:
			glEnable(GL_FRAGMENT_SHADER_ATI);
			glBindFragmentShaderATI(m_shaderName);
			break;

		default:
			// TODO: POSIX error code?
			ret = -1;
			break;
	}

	return ret;
}


/**
 * Disable the shader.
 * @return 0 on success; non-zero on error.
 */
int GLShader::disable(void)
{
	if (m_shaderName == 0) {
		// No shader has been compiled.
		// TODO: POSIX error code?
		return -1;
	}

	int ret = 0;
	switch (m_shaderType) {
		case ST_GL_ARB_FRAGMENT_PROGRAM:
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
			break;

#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		case ST_GL_ATI_TEXT_FRAGMENT_SHADER:
			glDisable(GL_TEXT_FRAGMENT_SHADER_ATI);

			if (m_shaderType == ST_GL_ATI_TEXT_FRAGMENT_SHADER) {
				// HACK: at least the Mac OS X 10.5 PPC Radeon drivers are broken and
				// without this disable the texture units while the program is still
				// running (10.4 PPC seems to work without this though).
				// Reference: http://git.mplayerhq.hu/?p=mplayer;a=blob;f=libvo/gl_common.c;hb=HEAD
				// NOTE: It doesn't seem to help any...
				glFlush();
			}
			break;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */

		case ST_GL_ATI_FRAGMENT_SHADER:
			glDisable(GL_FRAGMENT_SHADER_ATI);
			break;

		default:
			// TODO: POSIX error code?
			ret = -1;
			break;
	}

	return ret;
}

}
