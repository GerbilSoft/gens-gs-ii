/******************************************************************************
 * libgens: Gens Emulation Library.                                           *
 * lg_main.cpp: Main emulation code.                                          *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2015 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include <libgens/config.libgens.h>

#include "lg_main.hpp"
#include "macros/git.h"
#include "libcompat/cpuflags.h"
#include "Util/Timing.hpp"

// CPU emulation code.
#include "cpu/M68K.hpp"
#include "cpu/M68K_Mem.hpp"
#include "cpu/Z80.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// LibZomg metadata.
#include "libzomg/Metadata.hpp"

// C includes. (C++ namespace)
#include <cstdio>

// C++ includes.
#include <string>
using std::string;

namespace LibGens {

static bool ms_IsInit = false;

// libgens version.
// TODO: Use MDP version macros.
const unsigned int version_mdp = (VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_PATCH);
const char *const version = VERSION_STRING;            // ASCII
const char *const version_desc = "Development Build";	// ASCII

// Version Control System revision. (ASCII) (May be NULL.)
#ifdef GENS_GIT_VERSION
const char *const version_vcs = GENS_GIT_VERSION;
#else
const char *const version_vcs = nullptr;
#endif

/**
 * Determines if LibGens is running.
 * @return True if the LibGens thread is running; false otherwise.
 */
bool IsRunning(void)
{
	return ms_IsInit;
}

/**
 * Initialize LibGens.
 * @return 0 on success; non-zero on error.
 */
int Init(void)
{
	// TODO: Reference counting?
	if (ms_IsInit)
		return 0;

	// Print the Gens/GS startup message.
	fprintf(stderr, "Gens/GS II %s", version);
	if (version_vcs) {
		fprintf(stderr, " (%s)", version_vcs);
	}
	fputc('\n', stderr);

	// Version description.
	if (version_desc) {
		fprintf(stderr, "%s\n", version_desc);
	}

#if !defined(GENS_ENABLE_EMULATION)
	fprintf(stderr, "[NO-EMULATION BUILD; CPU emulation disabled.]\n");
#endif

	fprintf(stderr, "\n"
		"Copyright (c) 1999-2002 by Stéphane Dallongeville.\n"
		"Copyright (c) 2003-2004 by Stéphane Akhoun.\n"
		"Copyright (c) 2008-2015 by David Korth.\n"
		"\n");

	// GNU GPLv2 notice.
	// References:
	// * http://www.gnu.org/licenses/gpl-howto.html
	// * http://web.archive.org/web/20070121002445/http://www.gnu.org/licenses/gpl-howto.html
	fprintf(stderr,
		"This program is free software; you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation; either version 2 of the License, or\n"
		"(at your option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA\n"
		"\n");

	// TODO: Clear LibGens variables.

	// Detect CPU flags.
	LibCompat_GetCPUFlags();

	// Initialize LibGens subsystems.
	M68K::Init();
	M68K_Mem::Init();

	SoundMgr::Init();

	// Initialize LibZomg metadata.
	LibZomg::Metadata::InitProgramMetadata("Gens/GS II",
		version, version_desc, version_vcs);

	fflush(nullptr);	
	ms_IsInit = true;
	return 0;
}


/**
 * Stop LibGens.
 * TODO
 * @return 0 on success; non-zero on error.
 */
int End(void)
{
	// TODO
	if (!ms_IsInit)
		return 1;
	
	// TODO: Add CpuFlags::End() or something similar.
	CPU_Flags = 0;
	
	// Shut down LibGens subsystems.
	M68K::End();
	M68K_Mem::End();
	
	SoundMgr::End();
	
	ms_IsInit = false;
	return 0;
}

}
