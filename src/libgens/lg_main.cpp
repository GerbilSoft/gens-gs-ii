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
#include "Util/cpuflags.h"
#include "Util/Timing.hpp"

// CPU emulation code.
#include "cpu/M68K.hpp"
#include "cpu/M68K_Mem.hpp"
#include "cpu/Z80.hpp"
#include "cpu/Z80_MD_Mem.hpp"

// System emulation code.
#include "MD/EmuMD.hpp"
#include "MD/VdpIo.hpp"

// Input Device Manager.
#include "GensInput/DevManager.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// C includes.
#include <stdio.h>

// C++ includes.
#include <string>
using std::string;

// Win32 Unicode Translation Layer.
#ifdef _WIN32
#include "Win32/W32U_mini.hpp"
#endif


namespace LibGens
{

static bool ms_IsInit = false;


/**
 * IsRunning(): Determines if LibGens is running.
 * @return True if the LibGens thread is running; false otherwise.
 */
bool IsRunning(void)
{
	return ms_IsInit;
}


/**
 * Init(): Initialize LibGens.
 * @return 0 on success; non-zero on error.
 */
int Init(void)
{
	// TODO: Reference counting?
	if (ms_IsInit)
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
	
	// TODO: Clear LibGens variables.
	
	// Detect CPU flags.
	LibGens_GetCPUFlags();
	
	// Win32 Unicode Translation Layer.
#ifdef _WIN32
	W32U::Init();
#endif
	
	// Initialize LibGens subsystems.
	Timing::Init();
	M68K::Init();
	M68K_Mem::Init();
	Z80::Init();
	Z80_MD_Mem::Init();
	
	EmuMD::Init();
	VdpIo::Init();
	
	DevManager::Init();
	SoundMgr::Init();
	
	ms_IsInit = true;
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
	if (!ms_IsInit)
		return 1;
	
	// TODO: Add CpuFlags::End() or something similar.
	CPU_Flags = 0;
	
	// Shut down LibGens subsystems.
	Timing::End();
	M68K::End();
	M68K_Mem::End();
	Z80::End();
	Z80_MD_Mem::End();
	
	EmuMD::End();
	VdpIo::End();
	
	DevManager::End();
	SoundMgr::End();
	
	// Win32 Unicode Translation Layer.
#ifdef _WIN32
	W32U::End();
#endif
	
	ms_IsInit = false;
	return 0;
}

}
