/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderPaused.hpp: OpenGL Shader. (Paused Effect)                      *
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

#ifndef __GENS_QT4_VBACKEND_GLSHADERPAUSED_HPP__
#define __GENS_QT4_VBACKEND_GLSHADERPAUSED_HPP__

#include <gens-qt4/config.gens-qt4.h>

#include "GLShader.hpp"

namespace GensQt4 {

class GLShaderPaused : public GLShader
{
	public:
		GLShaderPaused() { }

	private:
		typedef GLShader super;
	private:
		Q_DISABLE_COPY(GLShaderPaused)

	public:
		/**
		 * Initialize the shader.
		 * This must be run from within a valid GL context!
		 */
		void init(void);

	private:
		/** Shaders. **/
#ifdef HAVE_GLEW
		static const char *const ms_GL_ARB_fragment_program;
#ifdef ENABLE_ATI_TEXT_FRAGMENT_SHADER
		static const char *const ms_GL_ATI_text_fragment_shader;
#endif /* ENABLE_ATI_TEXT_FRAGMENT_SHADER */
#endif /* HAVE_GLEW */
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADERPAUSED_HPP__ */
