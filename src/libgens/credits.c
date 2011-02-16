/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * credits.c: Gens/GS II credits.                                          *
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

#include "credits.h"

// C includes.
#include <string.h>

const GensGS_credits_t GensGS_credits[] =
{
	{"Main Programmer",		"David Korth"},
	{"-", NULL},
	
	{"Original Gens Programmer",	"Stéphane Dallongeville"},
	{"Gens/BeOS Port",		"Caz"},
	{"Gens/Linux Port",		"Stéphane Akhoun"},
	{"-", NULL},
	
	{"Gens/Linux Contributors",	"Lester Barrows (aryarya)"},
	{NULL,				"El Pelos (wah wah 69)"},
	{"-", NULL},
	
	{"Gens/GS Contributors",	NULL},
	{"-", NULL},
	
	{"Software Testing",		"AamirM"},
	{NULL,				"Baffo32"},
	{NULL,				"Biafra Republic"},
	{NULL,				"Corner-Face-Jacks"},
	{NULL,				"CyberKitsune"},
	{NULL,				"Delta"},
	{NULL,				"djohe"},
	{NULL,				"Glitch"},
	{NULL,				"mister_k81"},
	{NULL,				"NeKit"},
	{NULL,				"Nicolas Bondoux"},
	{NULL,				"segaloco"},
	{NULL,				"tetsuo"},
	{NULL,				"SOTI"},
	{NULL,				"SonicAD"},
	{NULL,				"superGear"},
	{NULL,				"Tets"},
	{NULL,				"TiZ"},
	{NULL,				"Zombie Ryushu"},
	{"-", NULL},
	
	{"Mac OS X Testing",		"Pietro Gagliardi (andlabs)"},
	{NULL,				"Puto"},
	{NULL,				"sonicblur"},
	{"-", NULL},
	
	{"MD Emulation Assistance",	"AamirM"},
	{NULL,				"Jorge"},
	{NULL,				"TmEE"},
	{"-", NULL},
	
	{"Test ROMs",			"Jorge"},
	{NULL,				"Sik"},
	{"-", NULL},
	
	{"32X PWM Improvements",	"Chilly Willy"},
	{NULL,				"Joseph Fenton"},
	{"-", NULL},
	
	{"GLSL Assistance",		"Damizean"},
	{"-", NULL},
	
	{"Gens/GS Application Icon",	"Marc Gordon (Cinossu)"},
	{"-", NULL},
	
	{"Game Genie Icon",		"Maximal|Firestorm"},
	{"-", NULL},
	
	{"SGens Icon",			"Sik"},
	{"-", NULL},
	
	{"VDP Layer Options Icon",	"SkyLights"},
	{"-", NULL},
	
	{"Game Gear PSG Stereo Info",	"nineko"},
	{"-", NULL},
	
	{"Haiku Port (in progress)",	"Phil Costin"},
	{"-", NULL},
	
	{"Controller Icons",		"theocas"},
	{"-", NULL},
	
	{"WahCade Integration",		"Zombie Ryushu"},
	{"-", NULL},
	
	{"VGM Logging Code",		"ValleyBell"},
	{"-", NULL},
	
	{"Credits ROM",			"Sik"},
	{NULL, NULL}
};
