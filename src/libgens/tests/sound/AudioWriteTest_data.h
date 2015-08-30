/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * AudioWriteTest_data.h: SoundMgr audio write test data.                  *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBGENS_TESTS_EEPROMI2CTEST_DATA_H__
#define __LIBGENS_TESTS_EEPROMI2CTEST_DATA_H__

// C includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * All test data contains 800 samples, which is equivalent
 * to one 60 Hz segment rendering audio at 48,000 Hz.
 */

// Internal buffer: LEFT channel.
extern const int32_t AudioWriteTest_Input_L[800];

// Internal buffer: RIGHT channel.
extern const int32_t AudioWriteTest_Input_R[800];

// Written data: STEREO output.
extern const int16_t AudioWriteTest_Output_Stereo[1600];

// Written data: MONO output, fast mixing.
extern const int16_t AudioWriteTest_Output_Mono_fast[800];

// Written data: MONO output, accurate mixing.
extern const int16_t AudioWriteTest_Output_Mono_accurate[800];

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_TESTS_EEPROMI2CTEST_DATA_H__ */
