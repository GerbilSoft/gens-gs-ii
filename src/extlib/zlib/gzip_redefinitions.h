/**
 * gzip function redefinitions.
 * Required for Gens/GS II because Qt4/Win32 has its own copy of gz*.c.
 */

#ifndef __ZLIB_GZIP_REDEFINITIONS_H__
#define __ZLIB_GZIP_REDEFINITIONS_H__

#define gzopen		gens_gzopen
#define gzdopen		gens_gzdopen
#define gzbuffer	gens_gzbuffer
#define gzsetparams	gens_gzsetparams
#define gzread		gens_gzread
#define gzwrite		gens_gzwrite
#define gzprintf	gens_gzprintf
#define gzputs		gens_gzputs
#define gzgets		gens_gzgets
#define gzputc		gens_gzputc
#define gzgetc		gens_gzgetc
#define gzungetc	gens_gzungetc
#define gzflush		gens_gzflush
#define gzseek		gens_gzseek
#define gzrewind	gens_gzrewind
#define gztell		gens_gztell
#define gzoffset	gens_gzoffset
#define gzeof		gens_gzeof
#define gzdirect	gens_gzdirect
#define gzclose		gens_gzclose
#define gzclose_r	gens_gzclose_r
#define gzclose_w	gens_gzclose_w
#define gzerror		gens_gzerror
#define gzclearerr	gens_gzclearerr
#define gzopen64	gens_gzopen64
#define gzseek64	gens_gzseek64
#define gztell64	gens_gztell64
#define gzoffset64	gens_gzoffset64

#endif /* __ZLIB_GZIP_REDEFINITIONS_H__ */
