/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * lg_main.cpp: Main emulation code.                                       *
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

#include "lg_main.hpp"
#include "macros/git.h"
#include "SdlVideo.hpp"

// C includes.
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

// C++ includes.
#include <string>
using std::string;

// VDP Rendering and MD emulation loop.
// TODO: Move to SdlVideo?
#include "MD/VdpRend.hpp"
#include "MD/EmuMD.hpp"

#include <SDL/SDL.h>

#define LG_MSG(str, ...) \
	fprintf(stderr, "LibGens::%s(): " str "\n", __func__, ##__VA_ARGS__)


namespace LibGens
{


static bool m_isInit = false;
static SDL_Thread *ms_thread = NULL;


/**
 * IsRunning(): Determines if LibGens is running.
 * @return True if the LibGens thread is running; false otherwise.
 */
bool IsRunning(void)
{
	return (m_isInit && ms_thread);
}


/**
 * Init(): Initialize LibGens.
 * @return 0 on success; non-zero on error.
 */
int Init(void)
{
	// TODO: Reference counting?
	if (m_isInit)
		return 0;
	
	// Print the Gens/GS startup message.
	fprintf(stderr, "Gens/GS II (Development Build)\n");
#ifdef GENS_GIT_VERSION
	fprintf(stderr, "(" GENS_GIT_VERSION ")\n");
#endif
	fprintf(stderr, "\n"
		"Copyright (c) 1999-2002 by Stéphane Dallongeville.\n"
		"Copyright (c) 2003-2004 by Stéphane Akhoun.\n"
		"Copyright (c) 2008-2010 by David Korth.\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify it\n"
		"under the terms of the GNU General Public License as published by the\n"
		"Free Software Foundation; either version 2 of the License, or (at your\n"
		"option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful, but\n"
		"WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License along\n"
		"with this program; if not, write to the Free Software Foundation, Inc.,\n"
		"51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
		"\n");
	
	// DEBUG: Initialize MD.
	EmuMD::Init_TEST();
	
	// TODO: Clear LibGens variables.
	return 0;
}


/**
 * End(): Stop LibGens.
 * TODO
 * @return 0 on success; non-zero on error.
 */
int End(void)
{
	// TODO
	return 0;
}

}
