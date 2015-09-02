/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLShaderFastBlur.hpp: OpenGL Shader. (Fast Blur)                        *
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

#ifndef __GENS_SDL_GLSHADERFASTBLUR_HPP__
#define __GENS_SDL_GLSHADERFASTBLUR_HPP__

#include "GLShader.hpp"

namespace GensSdl {

class GLShaderFastBlur : public GLShader
{
	public:
		/**
		 * Initialize the shader.
		 * This must be run from within a valid GL context!
		 * @return 0 on success; non-zero on error.
		 */
		virtual int init(void) final;

		/**
		 * Enable the shader.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int enable(void) final;
};

}

#endif /* __GENS_SDL_GLSHADERFASTBLUR_HPP__ */
