/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * compat_getopt.h: getopt.h compatibility header.                         *
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

#ifndef __LIBCOMPAT_COMPAT_GETOPT_H__
#define __LIBCOMPAT_COMPAT_GETOPT_H__

#include <libcompat/config.libcompat.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

/**
 * NOTE: vlc_getopt only has a replacement for vlc_getopt_long().
 * It also doesn't support optional arguments.
 * TODO: Port getopt() and getopt_long() from glibc or gnulib?
 */

#ifndef HAVE_GETOPT_LONG
#include "libcompat/vlc_getopt/vlc_getopt.h"
// Wrappers to make vlc_getopt_long() act like GNU getopt_long().
extern vlc_getopt_t vlc_getopt_state;
#define getopt_long(argc, argv, optstring, longopts, longindex) \
        vlc_getopt_long(argc, argv, optstring, longopts, longindex, &vlc_getopt_state)
#define option vlc_option
#define optarg vlc_getopt_state.arg
#define optind vlc_getopt_state.ind
#endif

#endif /* __LIBCOMPAT_COMPAT_GETOPT_H__ */
