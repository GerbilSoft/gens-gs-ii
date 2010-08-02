/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * log_msg.c: Message logging subsystem.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "log_msg.h"
#include <stdarg.h>
#include <stdio.h>

// Critical error handler function pointer.
static log_msg_critical_fn m_critical_fn = NULL;


/**
 * log_msg(): LOG_MSG() function.
 * This function is executed if the corresponding log message
 * is enabled via the LOG_MSG_* defines.
 * @param channel Debug channel.
 * @param level Debug level.
 * @param fn Function name.
 * @param msg Message.
 * @param ... Parameters.
 */
void log_msg(const char *channel, int level, const char *fn, const char *msg, ...)
{
	char out_msg[1024];
	int ret;
	
	// First part of the message.
	ret = snprintf(out_msg, sizeof(out_msg), "%s:%d:%s(): ", channel, level, fn);
	out_msg[sizeof(out_msg)-1] = 0x00;
	if (ret <= 0 || ret >= sizeof(out_msg))
	{
		// Error writing the first part of the message.
		return;
	}
	
	// Second part of the message.
	va_list ap;
	va_start(ap, msg);
	vsnprintf(&out_msg[ret], (sizeof(out_msg)-ret), msg, ap);
	va_end(ap);
	
	// Print the message to stderr.
	fprintf(stderr, "%s\n", out_msg);
	
	// If this is a CRITICAL error, invoke the critical error handler.
	if (level == LOG_MSG_LEVEL_CRITICAL && m_critical_fn != NULL)
		m_critical_fn(channel, out_msg);
}


/**
 * log_msg_register_critical_fn(): Register the critical error handler.
 * @param critical_fn Critical error handler function.
 */
void log_msg_register_critical_fn(log_msg_critical_fn critical_fn)
{
	m_critical_fn = critical_fn;
}
