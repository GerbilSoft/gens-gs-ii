/**
 * iowin32u.h: UTF-8 wrapper for iowin32.h.
 */

#ifndef __MINIZIP_IOWIN32U_H__
#define __MINIZIP_IOWIN32U_H__

/* _MZ_OF() */
#include "mz_of.h"

#ifdef __cplusplus
extern "C" {
#endif

void fill_win32_filefunc64U _MZ_OF((zlib_filefunc64_def* pzlib_filefunc_def));

#ifdef __cplusplus
}
#endif

#endif /* __MINIZIP_IOWIN32U_H__ */
