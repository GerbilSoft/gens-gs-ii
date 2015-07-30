/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * W32U_argv.h: UTF-8 conversion for argv[].                               *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBW32U_W32U_ARGV_H__
#define __LIBW32U_W32U_ARGV_H__

#ifndef _WIN32
#error W32U_argv.h should only be included on Win32!
#endif

/**
 * NOTE: W32U_argv.h doesn't have an include restriction,
 * since it's intended to only be run in main().
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert the Windows command line to UTF-8.
 * @param p_argc	[out] Pointer to new argc.
 * @param p_argv	[out] Pointer to new argv.
 * @param p_envp	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
int W32U_GetArgvU(int *p_argc, char **p_argv[], char **p_envp[]);

#ifdef __cplusplus
}
#endif

#endif /* __LIBW32U_W32U_ARGV_H__ */
