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
		
		inline bool isInit(void) const { return m_init; }
		
		/** Query available shaders. **/
#ifdef HAVE_GLEW
		inline bool hasPaused(void) const { return (m_paused_ARBfrag != 0); }
		inline bool hasFastBlur(void) const { return (m_fastBlur_ARBfrag != 0); }
#else
		inline bool hasPaused(void) const { return false; }
		inline bool hasFastBlur(void) const { return false; }
#endif
		
		/** Set Shader functions. **/
#ifdef HAVE_GLEW
		void setPaused(bool enabled);
		void setFastBlur(bool enabled);
#else
		/**
		 * NULL functions that do nothing if GLEW wasn't found at compile time.
		 * These functions are provided for a consistent interface.
		 * TODO: Should these functions be marked as inline?
		 */
		void setPaused(bool enabled) { ((void)enabled); }
		void setFastBlur(bool enabled) { ((void)enabled); }
#endif
	
	protected:
		bool m_init;	// True if the GL Shader Manager is initialized.
		
		/** OpenGL shaders. **/
		
#ifdef HAVE_GLEW
		// Paused effect: ARB fragment program.
		GLuint m_paused_ARBfrag;
		bool m_paused_enabled;
		static const char *ms_Paused_ARBfrag_asm;
		
		// Fast Blur: ARB fragment program.
		GLuint m_fastBlur_ARBfrag;
		bool m_fastBlur_enabled;
		static const char *ms_FastBlur_ARBfrag_asm;
#endif
};

}

#endif /* __GENS_QT4_VBACKEND_GLSHADERMANAGER_HPP__ */
