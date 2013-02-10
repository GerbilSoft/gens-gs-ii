// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

// TODO: CdDriveFactory.
#ifdef _WIN32
#include "CdDriveSpti.hpp"
#elif defined(__linux__)
#include "CdDriveLinux.hpp"
#endif

int main(int argc, char *argv[])
{
#if !defined(_WIN32) && !defined(__linux__)
	printf("Sorry, only Win32 and Linux are supported right now.\n");
	return EXIT_FAILURE;
#else
	if (argc != 2) {
#if defined(_WIN32)
		printf("Syntax: %s D:\n", argv[0]);
		printf("Replace D: with your CD-ROM drive.\n");
#elif defined(__linux__)
		printf("Syntax: %s /dev/sr0\n", argv[0]);
		printf("Replace /dev/sr0 with your CD-ROM drive.\n");
#endif
		return EXIT_FAILURE;
	}

	// Attempt to open the drive.
#if defined(_WIN32)
	LibGensCD::CdDriveSpti *cdrom = new LibGensCD::CdDriveSpti(argv[1]);
#elif defined(__linux__)
	LibGensCD::CdDriveLinux *cdrom = new LibGensCD::CdDriveLinux(argv[1]);
#endif
	if (!cdrom->isOpen()) {
		printf("Error opening %s.\n", argv[1]);
		printf("(TODO: Get the error code!)\n");
		return EXIT_FAILURE;
	}

	// Inquiry the drive.
	cdrom->inquiry();
	if (!cdrom->isInquirySuccessful()) {
		printf("INQUIRY failed.\n");
		printf("(TODO: Get the error code!)\n");
		return EXIT_FAILURE;
	}

	// Print the drive information.
	printf("Device information:\n");
	printf("Vendor:   %s\n", cdrom->dev_vendor().c_str());
	printf("Model:    %s\n", cdrom->dev_model().c_str());
	printf("Firmware: %s\n", cdrom->dev_firmware().c_str());
	printf("\n");

	// Get the disc and drive type.
	printf("Current Disc Type:  0x%08X\n", cdrom->getDiscType());
	printf("Current Drive Type: 0x%08X\n", cdrom->getDriveType());

	delete cdrom;
	return EXIT_SUCCESS;
#endif
}
