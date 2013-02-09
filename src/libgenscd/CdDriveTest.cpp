// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

// TODO: CdDriveFactory.
//#ifdef _WIN32
#include "CdDriveSpti.hpp"
//#endif

int main(int argc, char *argv[])
{
#ifndef _WIN32
	printf("Sorry, only Win32 is supported right now.\n");
	return EXIT_FAILURE;
#endif

	if (argc != 2) {
		printf("Syntax: %s D:\n", argv[0]);
		printf("Replace D: with your CD-ROM drive.\n");
		return EXIT_FAILURE;
	}

	// Attempt to open the drive.
	LibGensCD::CdDriveSpti *cdrom = new LibGensCD::CdDriveSpti(argv[1]);
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

	// Get the disc type.
	printf("Current Feature Profile: 0x%04X\n", cdrom->getDiscType());

	delete cdrom;
	return EXIT_SUCCESS;
}
