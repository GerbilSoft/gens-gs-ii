// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

#include "CdDrive.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2) {
#if defined(_WIN32)
		printf("Syntax: %s D:\n", argv[0]);
		printf("Replace D: with your CD-ROM drive.\n");
#else
		printf("Syntax: %s /dev/sr0\n", argv[0]);
		printf("Replace /dev/sr0 with your CD-ROM drive.\n");
#endif
		return EXIT_FAILURE;
	}

	// Attempt to open the drive.
	LibGensCD::CdDrive *cdrom = new LibGensCD::CdDrive(argv[1]);
	if (!cdrom->isOpen()) {
		printf("Error opening %s.\n", argv[1]);
		printf("(TODO: Get the error code!)\n");
		return EXIT_FAILURE;
	}

	// Check if the drive inquiry failed. (TODO)

	// Print the drive information.
	printf("Device information:\n");
	printf("Vendor:   %s\n", cdrom->dev_vendor().c_str());
	printf("Model:    %s\n", cdrom->dev_model().c_str());
	printf("Firmware: %s\n", cdrom->dev_firmware().c_str());
	printf("\n");

	// Get the disc and drive type.
	printf("Current Disc Type:  0x%08X\n", cdrom->getDiscType());
	printf("Current Drive Type: 0x%08X\n", cdrom->getDriveType());
	printf("\n");

	delete cdrom;
	return EXIT_SUCCESS;
}
