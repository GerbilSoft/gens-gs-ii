/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_main.hpp: Main UI code.                                            *
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

#include "gqt4_main.hpp"
#include "libgens/lg_main.hpp"

#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	// Initialize LibGens.
	int ret = LibGens::Init(NULL, "Gens/GS II");
	if (ret != 0)
		return ret;
	
	// Prompt for a color.
	char buf[1024];
	int n, r, g, b;
	uint32_t color;
	while (LibGens::IsRunning() && !feof(stdin))
	{
		printf("Enter an RGB color value as three numbers, e.g. 255 255 255.\n");
		fgets(buf, sizeof(buf), stdin);
		if (feof(stdin))
			break;
		
		// Parse the values.
		n = sscanf(buf, "%d %d %d", &r, &g, &b);
		if (n != 3 || r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
		{
			printf("Invalid input data. Try again.\n");
			continue;
		}
		
		// Send the message.
		color = (r | (g << 8) | (b << 16));
		LibGens::qToLG->push(LibGens::MtQueue::MTQ_LG_SETBGCOLOR, (void*)color);
		printf("\n");
	}
	
	// Shut down LibGens.
	LibGens::End();
	
	// Finished.
	return 0;
}
