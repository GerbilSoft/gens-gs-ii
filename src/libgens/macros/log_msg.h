/***************************************************************************
 * Gens: Message Logging.                                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef __LIBGENS_LOG_MSG_H__
#define __LIBGENS_LOG_MSG_H__

#include "common.h"

/**
 * Log levels:
 * LOG_MSG_LEVEL_CRITICAL	== critical error. A message box is shown.
 * LOG_MSG_LEVEL_ERROR		== error.
 * LOG_MSG_LEVEL_WARNING	== warning.
 * LOG_MSG_LEVEL_INFO		== information.
 * LOG_MSG_LEVEL_DEBUG1		== debug level 1.
 * LOG_MSG_LEVEL_DEBUG2		== debug level 2.
 * LOG_MSG_LEVEL_DEBUG3		== debug level 3.
 * LOG_MSG_LEVEL_DEBUG4		== debug level 4.
 */

#define LOG_MSG_LEVEL_CRITICAL	0
#define LOG_MSG_LEVEL_ERROR	1
#define LOG_MSG_LEVEL_WARNING	2
#define LOG_MSG_LEVEL_INFO	3
#define LOG_MSG_LEVEL_DEBUG1	4
#define LOG_MSG_LEVEL_DEBUG2	5
#define LOG_MSG_LEVEL_DEBUG3	6
#define LOG_MSG_LEVEL_DEBUG4	7

#define LOG_MSG_LEVEL_NONE	LOG_MSG_LEVEL_CRITICAL

/**
 * Message channels.
 * The channels are #define'd to the maximum log level.
 * A value of LOG_MSG_LEVEL_NONE is the same as LOG_MSG_LEVEL_CRITICAL,
 * and will only show critical errors.
 */

#define LOG_MSG_CHANNEL_gens	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_stub	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_video	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_audio	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_input	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_z	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_cd	LOG_MSG_LEVEL_INFO

#define LOG_MSG_CHANNEL_mdp	LOG_MSG_LEVEL_INFO

// CPUs.
#define LOG_MSG_CHANNEL_68k	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_z80	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_sh2	LOG_MSG_LEVEL_NONE

// VDP.
#define LOG_MSG_CHANNEL_vdp_io		LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_vdp_m5		LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_vdp_mcd		LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_vdp_32x		LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_vdp_m4		LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_vdp_tms9918	LOG_MSG_LEVEL_INFO

// Sound chip emulation.
#define LOG_MSG_CHANNEL_ym2612	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_psg	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_pcm	LOG_MSG_LEVEL_NONE

// SegaCD emulation.
#define LOG_MSG_CHANNEL_lc89510	LOG_MSG_LEVEL_NONE

// Actual MSG_LOG() code is below.

#ifdef __cplusplus
extern "C" {
#endif

/**
 * log_msg(): Internal message logging function.
 * DO NOT USE OUTSIDE OF log_msg.h!
 * @param channel Debug channel. (ASCII)
 * @param level Debug level.
 * @param msg Message. (UTF-8)
 * @param ... Parameters.
 */
void log_msg(const char *channel, int level, const char *fn, const utf8_str *msg, ...);

/**
 * log_msg_critical_fn(): Function pointer for critical error handler.
 */
typedef void (*log_msg_critical_fn)(const char *channel, const char *msg);

/**
 * log_msg_register_critical_fn(): Register the critical error handler.
 * @param critical_fn Critical error handler function.
 */
void log_msg_register_critical_fn(log_msg_critical_fn critical_fn);

#ifdef __cplusplus
}
#endif

/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB() LOG_MSG(stub, LOG_MSG_LEVEL_WARNING, "STUB function.")

/**
 * LOG_MSG(): Output a debug message.
 * @param channel Debug channel. (ASCII string)
 * @param level Debug level. (integer)
 * @param msg Message. (UTF-8)
 * @param ... Parameters.
 */
#define LOG_MSG(channel, level, msg, ...) \
do { \
	if (LOG_MSG_CHANNEL_ ##channel >= level) \
		log_msg(#channel, level, __func__, msg, ##__VA_ARGS__); \
} while (0)

/**
 * LOG_MSG_ONCE(): Output a debug message one time only.
 * @param channel Debug channel. (ASCII string)
 * @param level Debug level. (integer)
 * @param msg Message. (UTF-8)
 * @param ... Parameters.
 */
#define LOG_MSG_ONCE(channel, level, msg, ...)	\
do { \
	static unsigned char __warned = 0; \
	if (!__warned) \
	{ \
		LOG_MSG(channel, level, msg, ##__VA_ARGS__); \
		__warned = 1; \
	} \
} while (0)

#endif /* __LIBGENS_LOG_MSG_H__ */
