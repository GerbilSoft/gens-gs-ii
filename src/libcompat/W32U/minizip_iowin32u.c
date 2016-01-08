/**
 * minizip_iowin32u.c: UTF-8 wrapper for iowin32.c.
 */
#include "minizip_iowin32u.h"

// C includes.
#include <stdlib.h>

#include <zlib.h>
#include "minizip/ioapi.h"
#include "minizip/iowin32.h"

// Win32 Unicode Translation Layer.
#include <windows.h>
#include "W32U_mini.h"

#define __IN_W32U__
#include "W32U_alloca.h"

// open64 function pointers from iowin32.
static open64_file_func pWin32_open64_file_funcA;
static open64_file_func pWin32_open64_file_funcW;

static voidpf ZCALLBACK win32_open64_file_funcUA(voidpf opaque, const void* filename_utf8, int mode)
{
	// NOTE: Extra cast is required because
	// UtoW_filename() and WtoA_filename() are macros.
	const char *filename = (const char*)filename_utf8;
	wchar_t *filenameW;
	char *filenameA;

	// Convert the filename from UTF-8 to UTF-16.
	UtoW_filename(filename);
	if (!filenameW)
		return NULL;

	// Convert the filename from UTF-16 to ANSI.
	WtoA_filename(filename);
	if (!filename)
		return NULL;

	// Call the iowin32.c function.
	return pWin32_open64_file_funcA(opaque, filenameA, mode);
}

static voidpf ZCALLBACK win32_open64_file_funcUW(voidpf opaque, const void* filename_utf8, int mode)
{
	// NOTE: Extra cast is required because
	// UtoW_filename() is a macro.
	const char *filename = (const char*)filename_utf8;
	wchar_t *filenameW;

	// Convert the filename from UTF-8 to UTF-16.
	UtoW_filename(filename);
	if (!filenameW)
		return NULL;

	// Call the iowin32.c function.
	return pWin32_open64_file_funcW(opaque, filenameW, mode);
}

void fill_win32_filefunc64U(zlib_filefunc64_def* pzlib_filefunc_def)
{
	// NOTE: The ANSI and Unicode iowin32 function pointer tables
	// are identical except for the open64 function.

	if (!pWin32_open64_file_funcA) {
		// Get the iowin32 ANSI function pointer.
		fill_win32_filefunc64A(pzlib_filefunc_def);
		pWin32_open64_file_funcA = pzlib_filefunc_def->zopen64_file;
	}

	// Get the iowin32 Unicode function pointer.
	fill_win32_filefunc64W(pzlib_filefunc_def);
	pWin32_open64_file_funcW = pzlib_filefunc_def->zopen64_file;

	// Check if this system supports Unicode.
	if (W32U_IsUnicode()) {
		// System supports Unicode.
		pzlib_filefunc_def->zopen64_file = win32_open64_file_funcUW;
	} else {
		// System does not support Unicode.
		pzlib_filefunc_def->zopen64_file = win32_open64_file_funcUA;
	}
}
