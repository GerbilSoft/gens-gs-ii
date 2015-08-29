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

// These variables are set if the
// corresponding button is pressed.
static bool power_pressed = false;
static bool reset_pressed = false;

// Set if waiting for the user to press HOME.
static bool waiting_for_home = false;

/**
 * POWER button callback.
 */
static void __Sys_PowerCallback(void)
{
	// FIXME: Handle POWER while running the tests.
	// Currently, the test suite can't be interrupted.
	if (waiting_for_home) {
		power_pressed = true;
	}
}

/**
 * RESET button callback.
 */
static void __Sys_ResetCallback(void)
{
	// FIXME: Handle RESET while running the tests.
	// Currently, the test suite can't be interrupted.
	if (waiting_for_home) {
		reset_pressed = true;
	}
}

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
 * Initialize libogc's subsystems.
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

	// Initialize Wii remotes.
	WPAD_Init();
}

/**
 * Shut down libogc's subsystems.
 */
static void end_libogc(void)
{
	WPAD_SetPowerButtonCallback(NULL);
	WPAD_SetBatteryDeadCallback(NULL);

	// Make sure the Wii remotes are closed properly.
	while (WPAD_GetStatus() == WPAD_STATE_ENABLING) { }
	if (WPAD_GetStatus() == WPAD_STATE_ENABLED) {
		WPAD_Flush(WPAD_CHAN_ALL);
		WPAD_Shutdown();
	}
	WPAD_Shutdown();
}

/**
 * Wait for the HOME button to be pressed.
 */
static void wait_for_home(void)
{
	printf("\033[37;0m\nPress the HOME button to exit.");
	video_waitForFrame();

	waiting_for_home = true;
	bool in_loop = true;
	while (in_loop) {
		// Update the Wii Remote status.
		WPAD_ScanPads();

		// Check if HOME was pressed on any controller.
		for (int i = 0; i < 4; i++) {
			if (WPAD_ButtonsDown(i) & WPAD_BUTTON_HOME) {
				// HOME button pressed.
				in_loop = false;
				break;
			}
		}

		// Check if POWER or RESET was pressed.
		if (power_pressed) {
			printf("\nPOWER button pressed; shutting down...");
			video_waitForFrame();
			SYS_ResetSystem(SYS_POWEROFF, 0, 0);
		} else if (reset_pressed) {
			printf("\nRESET button pressed; restarting...");
			SYS_ResetSystem(SYS_RESTART, 0, 0);
		}
	}

	printf("\nReturning to loader...");
	video_waitForFrame();
}

int main(int argc, char *argv[])
{
	// Register power/reset callbacks.
	SYS_SetPowerCallback(__Sys_PowerCallback);
	SYS_SetResetCallback(__Sys_ResetCallback);

	// Initialize libogc's subsystems.
	init_libogc();

	// Run the test suite.
	int ret = test_main(argc, argv);
	wait_for_home();

	// Shut down libogc's subsystems.
	end_libogc();

	// Wait for the final frame to be rendered before
	// returning to the loader. This ensures that
	// test information isn't cut off.
	// TODO: Add a timeout and/or wait for a button press?
	video_waitForFrame();
	return ret;
}
