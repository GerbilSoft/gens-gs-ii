/**
 * minizip_iowin32u.h: UTF-8 wrapper for iowin32.h.
 */

#ifndef __LIBCOMPAT_W32U_MINIZIP_IOWIN32U_H__
#define __LIBCOMPAT_W32U_MINIZIP_IOWIN32U_H__

#include "minizip/ioapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a MiniZip 64-bit file function definition
 * using the UTF-8 conversion functions.
 *
 * @param pzlib_filefunc_def [out]
 */
void fill_win32_filefunc64U(zlib_filefunc64_def* pzlib_filefunc_def);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCOMPAT_W32U_MINIZIP_IOWIN32U_H__ */
