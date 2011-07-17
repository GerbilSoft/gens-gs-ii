/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VGA_charset.h: VGA character set. (ASCII only)                          *
 *                                                                         *
 * Based on the UniVGA X11 font.                                           *
 ***************************************************************************/

#ifndef __LIBGENS_MD_VGA_CHARSET_H__
#define __LIBGENS_MD_VGA_CHARSET_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t VGA_charset_ASCII[0x60][16];

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_MD_VGA_CHARSET_H__ */
