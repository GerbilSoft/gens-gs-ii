/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * AudioWriteTest_generator.c: SoundMgr audio write test data generator.   *
 * Generated using AudioWriteTest_generator.c.                             *
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static void createRandomSamples(int32_t *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		int32_t sample = (rand() & 0x7FFF) + (rand() & 0x7FFF) - 0x8000;
		if ((rand() & 0xF) == 0) {
			// Bias the value to be oversaturated.
			if (sample < 0) {
				sample -= 0x8000;
			} else {
				sample += 0x8000;
			}
		}
		if ((rand() & 0x1F) == 0) {
			// Bias the value to be even more oversaturated.
			if (sample < 0) {
				sample -= 0x10000;
			} else {
				sample += 0x10000;
			}
		}
		if ((rand() & 0x3F) == 0) {
			// Bias the value to be even *more* oversaturated.
			if (sample < 0) {
				sample -= 0x20000;
			} else {
				sample += 0x20000;
			}
		}
		buf[i] = sample;
	}
}

static inline int16_t clamp(int32_t sample)
{
	if (sample < -0x8000)
		return -0x8000;
	else if (sample > 0x7FFF)
		return 0x7FFF;
	return (int16_t)sample;
}

static inline int16_t clamp64(int64_t sample)
{
	if (sample < -0x8000)
		return -0x8000;
	else if (sample > 0x7FFF)
		return 0x7FFF;
	return (int16_t)sample;
}

/**
 * Write an int32_t buffer to a C source file.
 * @param f Output file.
 * @param name Name of the buffer.
 * @param buf Buffer.
 * @param size Number of entries in the buffer. (NOT bytes!)
 */
static void writeBuffer_i32(FILE *f, const char *name, const int32_t *buf, int size)
{
	int i;
	fprintf(f, "const int32_t %s[%d] = {\n\t", name, size);
	for (i = 0; i < size; i++, buf++) {
		if (i > 0 && (i % 4 == 0)) {
			fprintf(f, "\n\t");
		}
		if (*buf < 0) {
			fprintf(f, "-0x%08X", -*buf);
		} else {
			fprintf(f, " 0x%08X", *buf);
		}
		if (i != (size-1)) {
			fprintf(f, ",");
			if (i % 4 != 3) {
				fprintf(f, " ");
			}
		}
	}
	fprintf(f, "\n};\n");
}

/**
 * Write an int16_t buffer to a C source file.
 * @param f Output file.
 * @param name Name of the buffer.
 * @param buf Buffer.
 * @param size Number of entries in the buffer. (NOT bytes!)
 */
static void writeBuffer_i16(FILE *f, const char *name, const int16_t *buf, int size)
{
	int i;
	fprintf(f, "const int16_t %s[%d] = {\n\t", name, size);
	for (i = 0; i < size; i++, buf++) {
		if (i > 0 && (i % 8 == 0)) {
			fprintf(f, "\n\t");
		}
		if (*buf < 0) {
			fprintf(f, "-0x%04X", -*buf);
		} else {
			fprintf(f, " 0x%04X", *buf);
		}
		if (i != (size-1)) {
			fprintf(f, ",");
			if (i % 8 != 7) {
				fprintf(f, " ");
			}
		}
	}
	fprintf(f, "\n};\n");
}

int main(void)
{
	// Seed the RNG.
	srand((unsigned int)time(NULL));

	// Create 800-sample Left and Right buffers.
	// (48,000 Hz @ 60 Hz)
	int32_t bufL[800];
	int32_t bufR[800];
	// Destination buffers.
	int16_t destStereo[800*2];	// Stereo
	int16_t destMono_fast[800];	// Mono (fast)
	int16_t destMono_accurate[800];	// Mono (accurate for edge cases)

	// Start with edge cases.
	bufL[ 0] = 0;		//  0x0000
	bufL[ 1] = 0x3FFF;	// +0x3FFF
	bufL[ 2] = 0x4000;	// +0x4000
	bufL[ 3] = 0x7FFF;	// +0x7FFF
	bufL[ 4] = 0x8000;	// +0x7FFF [saturated]
	bufL[ 5] = 0xBFFF;	// +0x7FFF [saturated]
	bufL[ 6] = 0xC000;	// +0x7FFF [saturated]
	bufL[ 7] = 0xFFFF;	// +0x7FFF [saturated]
	bufL[ 8] = 0x10000;	// +0x7FFF [saturated]
	bufL[ 9] = 0x110000;	// +0x7FFF [saturated]
	bufL[10] = 0x1110000;	// +0x7FFF [saturated]
	bufL[11] = 0x11110000;	// +0x7FFF [saturated]
	bufL[12] = 0x11112222;	// +0x7FFF [saturated]
	bufL[13] = 0x3FFFFFFF;	// +0x7FFF [saturated]
	// NOTE: These edge cases likely will never happen,
	// and *will* cause overflow when calculating
	// monaural output by adding, then dividing.
	// Instead, divide, then add, then OR the LSBs.
	bufL[14] = 0x40000000;	// +0x7FFF [saturated]
	bufL[15] = 0x7FFFFFFF;	// +0x7FFF [saturated]
	bufL[16] = -0x3FFF;	// -0x3FFF
	bufL[17] = -0x4000;	// -0x4000
	bufL[18] = -0x7FFF;	// -0x7FFF
	bufL[19] = -0x8000;	// -0x8000
	bufL[20] = -0xBFFF;	// -0x8000 [saturated]
	bufL[21] = -0xC000;	// -0x8000 [saturated]
	bufL[22] = -0xFFFF;	// -0x8000 [saturated]
	bufL[23] = -0x10000;	// -0x8000 [saturated]
	bufL[24] = -0x110000;	// -0x8000 [saturated]
	bufL[25] = -0x1110000;	// -0x8000 [saturated]
	bufL[26] = -0x11110000;	// -0x8000 [saturated]
	bufL[27] = -0x11112222;	// -0x8000 [saturated]
	bufL[28] = -0x3FFFFFFF;	// -0x8000 [saturated]
	bufL[29] = -0x40000000;	// -0x8000 [saturated]
	bufL[30] = -0x7FFFFFFF;	// -0x8000 [saturated]
	bufL[31] = -0x80000000;	// -0x8000 [saturated]

	// Copy to bufR.
	memcpy(bufR, bufL, 32*sizeof(bufL[0]));

	// Create random data for the remaining values.
	createRandomSamples(&bufL[32], 800-32);
	createRandomSamples(&bufR[32], 800-32);

	// Clamp the stereo buffers and calculate the monaural buffer.
	int i;
	for (i = 0; i < 800; i++) {
		destStereo[i*2] = clamp(bufL[i]);
		destStereo[i*2+1] = clamp(bufR[i]);

		// Faster. (non-saturated add; can screw up with values >= 2^31)
		// NOTE: We're probably always going to use the fast algorithm,
		// since values >= 2^31 are highly unlikely. Also, we're probably
		// rendering in Stereo in most cases anyway.
		int32_t mono_fast = (bufL[i] + bufR[i]) >> 1;
		destMono_fast[i] = clamp(mono_fast);

		// Higher accuracy for edge cases.
		/* NOTE: This doesn't work when the samples are negative...
		int32_t mono_accurate = (bufL[i] >> 1) + (bufR[i] >> 1);
		mono_accurate |= ((bufL[i] & 1) | (bufR[i] & 1));
		destM_accurate[i] = clamp(mono_accurate);
		*/

		// Higher accuracy for edge cases.
		// Use an int64_t to avoid overflow.
		// We're not going to actually use this in the emulator;
		// instead, we'll use either the "fast" algorithm, which
		// won't overflow because we'll never hit >= 2^31, or
		// MMX/SSE2, which supports saturated addition.
		int64_t mono_accurate = ((int64_t)bufL[i] + (int64_t)bufR[i]) >> 1;
		destMono_accurate[i] = clamp64(mono_accurate);

		if (mono_fast != mono_accurate) {
			printf("L == %08X, R == %08X, fast == %04X, accurate == %04X\n",
				bufL[i], bufR[i], mono_fast, (int32_t)mono_accurate);
		}
	}

	// TODO: Edge cases for test data that isn't a multiple of 8 or 16 bytes?

	// Write the buffers.
	FILE *f = fopen("AudioWriteTest_data.c", "w");
	fprintf(f,
		"/***************************************************************************\n"
		" * libgens/tests: Gens Emulation Library. (Test Suite)                     *\n"
		" * AudioWriteTest_data.c: SoundMgr audio write test data.                  *\n"
		" * Generated using AudioWriteTest_generator.c.                             *\n"
		" *                                                                         *\n"
		" * Copyright (c) 2015 by David Korth.                                      *\n"
		" *                                                                         *\n"
		" * This program is free software; you can redistribute it and/or modify it *\n"
		" * under the terms of the GNU General Public License as published by the   *\n"
		" * Free Software Foundation; either version 2 of the License, or (at your  *\n"
		" * option) any later version.                                              *\n"
		" *                                                                         *\n"
		" * This program is distributed in the hope that it will be useful, but     *\n"
		" * WITHOUT ANY WARRANTY; without even the implied warranty of              *\n"
		" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *\n"
		" * GNU General Public License for more details.                            *\n"
		" *                                                                         *\n"
		" * You should have received a copy of the GNU General Public License along *\n"
		" * with this program; if not, write to the Free Software Foundation, Inc., *\n"
		" * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *\n"
		" ***************************************************************************/\n"
		"\n"
		"#include \"AudioWriteTest_data.h\"\n"
		"\n");

	// NOTE: Manually adding linebreaks between structs.
	writeBuffer_i32(f, "AudioWriteTest_Input_L", bufL, ARRAY_SIZE(bufL));
	fprintf(f, "\n");
	writeBuffer_i32(f, "AudioWriteTest_Input_R", bufR, ARRAY_SIZE(bufR));
	fprintf(f, "\n");
	writeBuffer_i16(f, "AudioWriteTest_Output_Stereo", destStereo, ARRAY_SIZE(destStereo));
	fprintf(f, "\n");
	writeBuffer_i16(f, "AudioWriteTest_Output_Mono_fast", destMono_fast, ARRAY_SIZE(destMono_fast));
	fprintf(f, "\n");
	writeBuffer_i16(f, "AudioWriteTest_Output_Mono_accurate", destMono_accurate, ARRAY_SIZE(destMono_accurate));
}
