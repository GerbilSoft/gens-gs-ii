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

// open64 function pointers from iowin32.
static open64_file_func pWin32_open64_file_funcA;
static open64_file_func pWin32_open64_file_funcW;

voidpf ZCALLBACK win32_open64_file_funcUA(voidpf opaque, const void* filename, int mode)
{
	int cchWcs, cbMbs;
	wchar_t *wcs;
	char *mbs;
	voidpf ret;

	// Convert the UTF-8 filename to UTF-16.
	cchWcs = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;

	wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, wcs, cchWcs);

	// Convert the UTF-16 filename to ANSI.
	cbMbs = WideCharToMultiByte(CP_ACP, 0, wcs, cchWcs, NULL, 0, NULL, NULL);
	if (cbMbs <= 0) {
		free(wcs);
		return NULL;
	}

	mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(CP_ACP, 0, wcs, cchWcs, mbs, cbMbs, NULL, NULL);
	free(wcs);

	// Call the iowin32.c function.
	ret = pWin32_open64_file_funcA(opaque, mbs, mode);
	free(mbs);
	return ret;
}

voidpf ZCALLBACK win32_open64_file_funcUW(voidpf opaque, const void* filename, int mode)
{
	int cchWcs;
	wchar_t *wcs;
	voidpf ret;

	// Convert the UTF-8 filename to UTF-16.
	cchWcs = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;

	wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, wcs, cchWcs);

	// Call the iowin32.c function.
	ret = pWin32_open64_file_funcW(opaque, (const void*)wcs, mode);
	free(wcs);
	return ret;
}

void fill_win32_filefunc64U(zlib_filefunc64_def* pzlib_filefunc_def)
{
	// TODO: Verify that the system supports UTF-8.
	// Win9x: Windows 98 and later supports UTF-8. (NOT WINDOWS 95!)
	// WinNT: Windows NT 4.0 and later supports UTF-8.
	// Not sure about NT 3.x.

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
