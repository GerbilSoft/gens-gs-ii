/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FpsManager.cpp: FPS Manager class.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "FpsManager.hpp"

// ARRAY_SIZE(x)
#include "libgens/macros/common.h"

namespace GensQt4 {

FpsManager::FpsManager(QObject *parent)
	: super(parent)
	, m_fpsAvg(0.0)
	, m_fpsPtr(0)
{
	// Reset the FPS array.
	for (int i = 0; i < ARRAY_SIZE(m_fps); i++) {
		m_fps[i] = -1.0;
	}
}

/**
 * Reset the FPS manager.
 */
void FpsManager::reset(void)
{
	// Clear the FPS average and reset the pointer.
	m_fpsAvg = 0.0;
	m_fpsPtr = 0;

	// Reset the FPS array.
	for (int i = 0; i < ARRAY_SIZE(m_fps); i++) {
		m_fps[i] = -1.0;
	}

	// Average FPS has been updated.
	emit updated(m_fpsAvg);
}

/**
 * Push an FPS value.
 * @param fps FPS value.
 */
void FpsManager::push(double fps)
{
	m_fpsPtr = (m_fpsPtr + 1) % ARRAY_SIZE(m_fps);
	m_fps[m_fpsPtr] = fps;

	// Calculate the new average.
	int count = 0;
	double sum = 0;
	for (int i = 0; i < ARRAY_SIZE(m_fps); i++) {
		if (m_fps[i] >= 0.0) {
			sum += m_fps[i];
			count++;
		}
	}

	if (count <= 0) {
		m_fpsAvg = 0.0;
	} else {
		m_fpsAvg = (sum / (double)count);
	}

	// Average FPS has been updated.
	emit updated(m_fpsAvg);
}

}
