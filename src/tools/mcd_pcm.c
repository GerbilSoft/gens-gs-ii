/***************************************************************************
 * mcd_pcm: Sega CD PCM utility.                                           *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>

// popt
#include <popt.h>

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#include "libcompat/W32U/W32U_argv.h"
#endif

// strdup() requires __USE_BSD when compiling without GNU extensions.
#define __USE_BSD 1
#include <string.h>

#define MCD_PCM_VERSION 0x00010000U

/* Make sure the structs are packed. */
#if defined(__GNUC__)
#if !defined(PACKED)
#define PACKED __attribute__ ((packed))
#endif
#else
#define PACKED
#endif

// Directory separator.
#ifdef _WIN32
#define DIRSEP_CHR '\\'
#else
#define DIRSEP_CHR '/'
#endif

// Default sample rate.
// TODO: What frequency should be used?
// We're going to assume (12.5 MHz / 384) == 32,552.1 Hz.
static const uint32_t def_sample_rate = 12500000 / 384;

/* WAV header struct. */
/* Description from http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
/* All values are little-endian, except for character fields. */
#pragma pack(1)
typedef struct PACKED _wav_header_t {
	/* RIFF chunk descriptor. */
	struct PACKED {
		char		ChunkID[4];	/* Contains "RIFF". */
		uint32_t	ChunkSize;	/* Size of the whole WAV, minus 8. */
		char		Format[4];	/* Contains "WAVE". */
	} riff;

	/* "fmt " sub-chunk. */
	struct PACKED {
		char		SubchunkID[4];	/* Contains "fmt ". */
		uint32_t	SubchunkSize;	/* Size of the subchunk, minus 8. */
		uint16_t	AudioFormat;	/* PCM == 1 */
		uint16_t	NumChannels;
		uint32_t	SampleRate;
		uint32_t	ByteRate;
		uint16_t	BlockAlign;
		uint16_t	BitsPerSample;
	} fmt;
	
	struct PACKED {
		char		SubchunkID[4];	/* Contains "data". */
		uint32_t	SubchunkSize;	/* Size of the whole WAV, minus 8, minus size of riff and fmt. */
	} data;
} wav_header_t;
#pragma pack()

// Endianness defines ported from libsdl.
// TODO: Figure out how to do this in CMake.
#define PCM_LIL_ENDIAN 1234
#define PCM_BIG_ENDIAN 4321
#ifndef PCM_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__ppc64__) || \
    defined(__powerpc__) || defined(__powerpc64__) || \
    defined(__POWERPC__) || defined(__POWERPC64__) || \
    defined(_M_PPC) || \
    defined(__armeb__) || defined(__ARMEB__) || \
    defined(__SPARC__)
#define PCM_BYTEORDER PCM_BIG_ENDIAN
#else
#define PCM_BYTEORDER PCM_LIL_ENDIAN
#endif
#endif

#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		(((x) & 0x0000FF00UL) << 8) | \
		(((x) & 0x00FF0000UL) >> 8))

#if PCM_BYTEORDER == PCM_LIL_ENDIAN
	#define be16_to_cpu(x)  __swab16(x)
	#define be32_to_cpu(x)  __swab32(x)
	#define le16_to_cpu(x)  (x)
	#define le32_to_cpu(x)  (x)

	#define cpu_to_be16(x)  __swab16(x)
	#define cpu_to_be32(x)  __swab32(x)
	#define cpu_to_le16(x)  (x)
	#define cpu_to_le32(x)  (x)
#else /* PCM_BYTEORDER == PCM_BIG_ENDIAN */
	#define be16_to_cpu(x)  (x)
	#define be32_to_cpu(x)  (x)
	#define le16_to_cpu(x)  __swab16(x)
	#define le32_to_cpu(x)  __swab32(x)

	#define cpu_to_be16(x)  (x)
	#define cpu_to_be32(x)  (x)
	#define cpu_to_le16(x)  __swab16(x)
	#define cpu_to_le32(x)  __swab32(x)
#endif

static void print_prg_info(void)
{
	fprintf(stderr, "mcd_pcm: Sega CD PCM Utility. (Version ");
	
	if (MCD_PCM_VERSION & 0xFFFF) {
		fprintf(stderr, "%d.%d.%d",
			((MCD_PCM_VERSION >> 24) & 0xFF),
			((MCD_PCM_VERSION >> 16) & 0xFF),
			(MCD_PCM_VERSION & 0xFFFF));
	} else {
		fprintf(stderr, "%d.%d",
			((MCD_PCM_VERSION >> 24) & 0xFF),
			((MCD_PCM_VERSION >> 16) & 0xFF));
	}

	fprintf(stderr, ")\n"
		"Copyright (c) 2011 by David Korth.\n");
}

static void print_gpl(void)
{
	fprintf(stderr,
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
}

static void print_help(const poptContext con)
{
	print_prg_info();
	fputc('\n', stderr);
	// NOTE: poptPrintHelp() only prints the filename portion of argv[0].
	poptPrintHelp(con, stderr, 0);

	fprintf(stderr,
		"\n"
		"Starting and ending positions may be specified in hexadecimal if\n"
		"prefixed with \"0x\".\n"
		"\n"
		"Common sample rates include 16276, 24414, and 32552 (Sega CD max).\n"
		"\n");
}

/**
 * Process PCM data.
 * @param f_pcm			[in] PCM file. (input)
 * @param f_wav			[in] WAV file. (output)
 * @param start_pos		[in] Starting position within the PCM file.
 * @param max_length		[in] Maximum length to decode. (negative == until EOF or 0xFF)
 * @param sample_rate		[in] Sample rate to encode in the WAV header.
 * @param samples_processed	[out] If not NULL, will contain number of samples processed.
 * @return Error code:
 * - 0 on success.
 * - 1 if EOF is hit before the PCM end flag.
 * - 2 if max_length is hit before the PCM end flag.
 * - other on error
 */
static int process_pcm(FILE *f_pcm, FILE *f_wav,
		       int64_t start_pos, int max_length,
		       uint32_t sample_rate,
		       uint32_t *samples_processed)
{
	// WAV header.
	wav_header_t wav_header;
	// Chunk and Subchunk IDs.
	static const char chunk_RIFF[4] = {'R', 'I', 'F', 'F'};
	static const char chunk_WAVE[4] = {'W', 'A', 'V', 'E'};
	static const char chunk_fmt[4]  = {'f', 'm', 't', ' '};
	static const char chunk_data[4] = {'d', 'a', 't', 'a'};

	// PCM and WAV buffers.
	uint8_t buf_pcm[4096];
	uint8_t buf_wav[4096];
	int ret;			// fread() return value.
	int i;				// `for` loop counter.
	int found_end_flag = 0;		// Set if the PCM end flag 0xFF is found.
	int end_flag_code = 1;		// End flag error code.
	int wav_length = 0;		// WAV data length.

	// Create the WAV header.
	// This will be written after the PCM data is converted.
	memset(&wav_header, 0, sizeof(wav_header));

	// RIFF
	memcpy(wav_header.riff.ChunkID, chunk_RIFF, sizeof(chunk_RIFF));
	// ChunkSize will be filled in after the conversion is completed.
	memcpy(wav_header.riff.Format, chunk_WAVE, sizeof(chunk_WAVE));

	// fmt
	memcpy(wav_header.fmt.SubchunkID, chunk_fmt, sizeof(chunk_fmt));
	wav_header.fmt.SubchunkSize	= cpu_to_le32(sizeof(wav_header.fmt) - 8);
	wav_header.fmt.AudioFormat	= cpu_to_le16(1);		// PCM
	wav_header.fmt.NumChannels	= cpu_to_le16(1);
	wav_header.fmt.SampleRate	= cpu_to_le32(sample_rate);
	wav_header.fmt.BitsPerSample	= cpu_to_le16(8);
	wav_header.fmt.BlockAlign	= cpu_to_le16(1);		// Each sample is 8 bits.
	wav_header.fmt.ByteRate		= cpu_to_le32(sample_rate);	// sample_rate bytes per sample.

	// data
	memcpy(wav_header.data.SubchunkID, chunk_data, sizeof(chunk_data));
	// SubchunkSize will be filled in after the conversion is completed.

	// Negative max length == all the PCM.
	if (max_length < 0) {
		max_length = INT_MAX;
	}

	// Seek to the starting position within the PCM file.
	fseeko(f_pcm, start_pos, SEEK_SET);
	// Seek to immediately after the WAV header.
	fseeko(f_wav, (int64_t)sizeof(wav_header), SEEK_SET);

	// Read 4 KB at a time.
	while (!feof(f_pcm) && !found_end_flag) {
		ret = fread(buf_pcm, 1, sizeof(buf_pcm), f_pcm);
		if ((wav_length + ret) > max_length || (wav_length + ret) < 0) {
			// Either we hit the maximum length,
			// or overflow occurred.
			ret = (max_length - wav_length);
			found_end_flag = 1;
			end_flag_code = 2;
		}

		for (i = 0; i < ret; i++) {
			// Sega CD PCM uses sign-magnitude format.
			if (buf_pcm[i] == 0xFF) {
				// End of PCM.
				ret = i;
				found_end_flag = 1;
				end_flag_code = 0;
				break;
			}

			// Convert sign-magnitude to unsigned 8-bit.
			if (buf_pcm[i] & 0x80) {
				// Sign bit is set: negative sample.
				// Unsigned 8-bit range: 0x00 - 0x7F
				buf_wav[i] = (0x80 - (buf_pcm[i] & ~0x80));
			} else {
				// Sign bit is not set: positive sample.
				// Unsigned 8-bit range: 0x80 - 0xFF
				buf_wav[i] = (0x80 + buf_pcm[i]);
			}
		}

		// Write the WAV data.
		fwrite(buf_wav, 1, ret, f_wav);

		// Increase the wav_length counter.
		wav_length += ret;
	}

	// Write the WAV header.
	fseeko(f_wav, 0, SEEK_SET);
	wav_header.data.SubchunkSize = cpu_to_le32(wav_length);
	wav_header.riff.ChunkSize    = cpu_to_le32(wav_length +
						   sizeof(wav_header.data) +
						   sizeof(wav_header.fmt) +
						   sizeof(wav_header.riff) - 8);
	fwrite(&wav_header, 1, sizeof(wav_header), f_wav);

	// If samples_processed is not NULL, return the number of samples processed.
	if (samples_processed) {
		*samples_processed = wav_length;
	}

	// Return the end flag error code.
	return end_flag_code;
}

int main(int argc, char *argv[])
{
	// Options.
	int64_t start_pos = 0;
	int max_length = -1;
	uint32_t sample_rate = def_sample_rate;
	char *out_filename = NULL;
	const char *pcm_filename = NULL;

	// Opened files.
	FILE *f_pcm = NULL;
	FILE *f_wav = NULL;

	// process_pcm() variables.
	uint32_t samples_processed = 0;
	int ret = -1;
	int mins = 0, secs = 0, csecs = 0;

	// popt: help options table.
	struct poptOption helpOptionsTable[] = {
		{"help", '?', POPT_ARG_NONE, NULL, '?', "Show this help message", NULL},
		{"usage", 0, POPT_ARG_NONE, NULL, 'u', "Display brief usage message", NULL},
		{"version", 'V', POPT_ARG_NONE, NULL, 'V', "Display version information", NULL},
		POPT_TABLEEND
	};

	// popt: main options table.
	struct poptOption optionsTable[] = {
		{"start",  's', POPT_ARG_LONGLONG, &start_pos, 0,
			"Starting position in pcm_file.bin, in bytes. (default = 0)", "POS"},
		{"length", 'l', POPT_ARG_INT, &max_length, 0,
			"Maximum length to dump, in bytes. (default = entire file)", "LEN"},
		{"rate",   'r', POPT_ARG_INT, (int*)&sample_rate, 0,
			"Sample rate. (default = 32552 Hz)", "RATE"},
		{"output", 'o', POPT_ARG_STRING, out_filename, 0,
			"Output filename. (default = pcm_file.wav)", "FILENAME"},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
			"Help options:", NULL},
		POPT_TABLEEND
	};
	poptContext optCon;
	int c;

#ifdef _WIN32
        // Convert command line parameters to UTF-8.
        if (W32U_GetArgvU(&argc, &argv, nullptr) != 0) {
                // ERROR!
                return EXIT_FAILURE;
        }
#endif /* _WIN32 */

	// TODO: Add a popt alias context?
	optCon = poptGetContext(NULL, argc, (const char**)argv, optionsTable, 0);
	poptSetOtherOptionHelp(optCon, "<pcm_file>");
	if (argc < 2) {
		poptPrintUsage(optCon, stderr, 0);
		return EXIT_FAILURE;
	}

	// popt: Alias '-h' to '-?'.
	{
		const char *help_argv[2] = {"-?", NULL};
		struct poptAlias help_alias = {NULL, 'h', 1, help_argv};
		poptAddAlias(optCon, help_alias, 0);
	}

	// Process options.
	while ((c = poptGetNextOpt(optCon)) >= 0) {
		switch (c) {
			case 'V':
				print_prg_info();
				fputc('\n', stderr);
				print_gpl();
				return EXIT_SUCCESS;

			case '?':
				print_help(optCon);
				return EXIT_SUCCESS;

			case 'u':
				poptPrintUsage(optCon, stderr, 0);
				return EXIT_SUCCESS;

			default:
				break;
		}
	}

	if (c < -1) {
		// An error occurred during option processing.
		switch (c) {
			case POPT_ERROR_BADOPT:
				// Unrecognized option.
				fprintf(stderr, "%s: unrecognized option '%s'\n"
					"Try `%s --help` for more information.\n",
					argv[0], poptBadOption(optCon, POPT_BADOPTION_NOALIAS), argv[0]);
				break;
			default:
				// Other error.
				fprintf(stderr, "%s: '%s': %s\n"
					"Try `%s --help` for more information.\n",
					argv[0], poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
					poptStrerror(c), argv[0]);
				break;
		}
		return EXIT_FAILURE;
	}

	// Get the input filename.
	pcm_filename = poptGetArg(optCon);
	if (pcm_filename == NULL || poptPeekArg(optCon) != NULL) {
		// Either the input filename wasn't specified,
		// or too many filenames were specified.
		fprintf(stderr, "%s: no filename specified\n"
			"Try `%s --help` for more information.\n",
			argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	// Done parsing arguments.
	poptFreeContext(optCon);

	// Attempt to open the file.
	f_pcm = fopen(pcm_filename, "rb");
	if (!f_pcm) {
		fprintf(stderr, "%s: Error opening '%s': %s\n",
			argv[0], pcm_filename, strerror(errno));
		return EXIT_FAILURE;
	}

	// Open the output file.
	if (out_filename) {
		// Output filename specified.
		f_wav = fopen(out_filename, "wb");
	} else {
		// Output filename not specified.
		// Create it by taking the source filename and changing its extension to ".wav".
		const size_t pcm_filename_len = strlen(pcm_filename);
		const size_t out_filename_len = pcm_filename_len + 16;
		char *src_slash_pos, *dot_pos;

		// Allocate a new buffer for the filename.
		out_filename = (char*)malloc(out_filename_len);

		// Strip the source filename of any directories.
		src_slash_pos = strrchr(pcm_filename, DIRSEP_CHR);
		if (src_slash_pos) {
			strncpy(out_filename, (src_slash_pos + 1), out_filename_len);
		} else {
			strncpy(out_filename, pcm_filename, out_filename_len);
		}

		// Search for a '.' in the filename, starting from the end.
		dot_pos = strrchr(out_filename, '.');
		if (!dot_pos) {
			// No dot found. Append ".wav" to the filename.
			// TODO: strlcat()?
			strcat(out_filename, ".wav");
		} else {
			// Dot found. Search for a directory separator character.
			char *slash_pos = strrchr(out_filename, DIRSEP_CHR);
			if (slash_pos && (slash_pos > dot_pos)) {
				// Slash found, but it's after the dot.
				// Append ".wav" to the filename.
				// TODO: strlcat()?
				strcat(out_filename, ".wav");
			} else {
				// One of two possibilities:
				// - Slash not found.
				// - Slash found, and it's before the dot.
				// Replace the extension with ".wav".
				// TODO: strlcpy()?
				// TODO: Ensure buffer overflows don't happen.
				strcpy(dot_pos, ".wav");
			}
		}

		// Open the WAV file.
		f_wav = fopen(out_filename, "wb");
	}

	// Verify that the WAV file was opened.
	if (!f_wav) {
		// WAV file could not be opened.
		fprintf(stderr, "%s: Error opening '%s': %s\n",
			argv[0], out_filename, strerror(errno));
		fclose(f_pcm);
		free(out_filename);
		return EXIT_FAILURE;
	}

	// Process the file.
	ret = process_pcm(f_pcm, f_wav, start_pos, max_length,
			      sample_rate, &samples_processed);

	// Close the files.
	fclose(f_wav);
	fclose(f_pcm);

	// Print statistics.
	mins = (samples_processed / sample_rate / 60);
	secs = (samples_processed / sample_rate % 60);
	csecs = ((samples_processed * 100) / sample_rate % 100);

	printf("%s: %s converted to WAV:\n"
		"- Sample rate: %d Hz\n"
		"- %d samples processed\n"
		"- Duration: %01d:%02d.%02d\n",
		argv[0], pcm_filename, sample_rate,
		samples_processed, mins, secs, csecs);

	switch (ret) {
		case 0:
			printf("- End of PCM flag (0xFF) reached.\n");
			break;
		case 1:
			printf("- End of file reached.\n");
			break;
		case 2:
			printf("- Maximum length (%d bytes) reached.\n", max_length);
			break;
		default:
			printf("- Unknown return code %d from process_pcm().\n", ret);
			break;
	}

	// TODO: 1 is reserved for EXIT_FAILURE; use other values here?
	return ret;
}
