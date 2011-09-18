/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLShaderFastBlur.hpp: OpenGL Shader. (Fast Blur)                        *
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

#ifndef __GENS_QT4_VBACKEND_GLSHADERFASTBLUR_HPP__
#define __GENS_QT4_VBACKEND_GLSHADERFASTBLUR_HPP__

#include <gens-qt4/config.gens-qt4.h>

#include "GLShader.hpp"

namespace GensQt4
{

class GLShaderFastBlur : public GLShader
{
	public:
		/**
		 * init(): Initialize the shader.
		 * This must be run from within a valid GL context!
		 */
		void init(void);
		
		/**
		 * enable(): Enable the shader.
		 */
		void enable(void);
	
	private:
		/** Shaders. **/
#ifdef HAVE_GLEW
		static const char *ms_GL_ARB_fragment_program;
#endif /* HAVE_GLEW */
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADERFASTBLUR_HPP__ */
