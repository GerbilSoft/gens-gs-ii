/*****************************************************************************************
 * libgens: Gens Emulation Library.                                                      *
 * fw_32x.h: Reverse-engineered Sega 32X firmware.                                       *
 *                                                                                       *
 * Copyright (c) 2005 by Joseph Norman.                                                  *
 *                                                                                       *
  * Feel free to use this code, recompile the code, redistribute the unmodified code,    *
 * modify it with your own name on it and redistribute it as yours if you                *
 * so wish to do so without getting caught looking stupid, but you may not sell it for   *
 * cash monies, or for in exchange of hot prostitutes, nor include it with any other     *
 * redistributable software packages without consent from DevSter. This code is IS AS,   *
 * which is latin for jibber jabber, to DevSter and the holder of this code, means       *
 * there are no other further attatchments, absolutely no guarantees in it "working",    *
 * comes with no lifetime waranty, et al, and you will gain nothing more than to play    *
 * your super cool Sega Genesis 32X (names reserved to their rightful owners) without    *
 * having to resort to using the actual copyrighted bios files. Let it further be noted  *
 * that the use of the word "code" in this exclaimer refers to both the source code, and *
 * the pre-compiled code that was distributed.                                           *
 *****************************************************************************************/

#ifndef __LIBGENS_DATA_32X_FW_32X_H__
#define __LIBGENS_DATA_32X_FW_32X_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * fw_32x_genesis[]: Genesis-side firmware for 32X.
 * Mapped at:
 * - M68K: 0x400000 - 0x47FFFF (mirrored every 256 bytes)
 * - M68K: 0x480000 - 0x4800FF
 */
extern const uint8_t fw_32x_genesis[256];

/**
 * fw_32x_msh2[]: Master SH2 firmware for 32X.
 * Mapped at: MSH2: 0x00000000 - 0x000007FF
 * 
 * NOTE: MSH2 firmware is 2048 bytes, but this version
 * is only 712 bytes. It will need to be padded on load.
 */
extern const uint8_t fw_32x_msh2[712];

/**
 * fw_32x_msh2[]: Slave SH2 firmware for 32X.
 * Mapped at: SSH2: 0x00000000 - 0x000003FF
 * 
 * NOTE: SSH2 firmware is 1024 bytes, but this version
 * is only 532 bytes. It will need to be padded on load.
 */
extern const uint8_t fw_32x_ssh2[532];

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_DATA_32X_FW_32X_H__ */
