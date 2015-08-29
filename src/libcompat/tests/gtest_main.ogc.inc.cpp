/***************************************************************************
 * libcompat/tests: Compatibility Library. (Test Suite)                    *
 * gtest_main.ogc.inc.cpp: main() function for test suites.                *
 * libogc version; used for Nintendo Wii and Nintendo GameCube.            *
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

// NOTE: You must have a static int test_main() function that has the
// actual test case code. This file merely initializes gtest for certain
// embedded systems.

#if !defined(HW_RVL) && !defined(HW_DOL)
#error gtest_main.ogc.inc.cpp is for libogc only.
#endif

// libogc
#include <gccore.h>
#include <wiiuse/wpad.h>
static GXRModeObj *rmode = nullptr;
static void *xfb = nullptr;

/**
 * Wait for the next full frame.
 * In interlaced mode, this waits for both fields to be drawn.
 */
static void video_waitForFrame(void)
{
	VIDEO_WaitVSync();
	if (rmode->viTVMode&VI_NON_INTERLACE) {
		VIDEO_WaitVSync();
	} else {
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();
	}
}

/**
 * Initialize libogc's VIDEO and CON subsystems.
 */
static void init_libogc(void)
{
	// Initialize video using the preferred video mode.
	// This corresponds to the settings in the Wii System Menu.
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(nullptr);

	// Workaround for overscan issues. (from DOP-Mii)
	// NOTE: This effectively decreases visible area from 80x30 to 80x28 on NTSC!
	if (rmode->viTVMode == VI_NTSC || CONF_GetEuRGB60() || CONF_GetProgressiveScan())
                GX_AdjustForOverscan(rmode, rmode, 0, (u16)(rmode->viWidth * 0.026));

	VIDEO_Configure(rmode);

	// Initialize the display.
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();

	// Wait for the next frame before we initialize the console.
	video_waitForFrame();

	// Initialize the console. (Required for printf().)
	CON_InitEx(rmode,		// video mode
		   0,			// xstart
		   0,			// ystart
		   rmode->fbWidth,	// xres
		   rmode->xfbHeight	// yres
		 );
}

int main(int argc, char *argv[])
{
	// Initialize libogc's subsystems.
	init_libogc();

	// Run the test suite.
	int ret = test_main(argc, argv);

	// Wait for the final frame to be rendered before
	// returning to the loader. This ensures that
	// test information isn't cut off.
	// TODO: Add a timeout and/or wait for a button press?
	video_waitForFrame();
	return ret;
}
