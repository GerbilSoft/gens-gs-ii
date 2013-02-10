#ifndef __LIBGENSCD_GENSCD_SCSI_WIN32_H__
#define __LIBGENSCD_GENSCD_SCSI_WIN32_H__

/*

Some structures are borrowed from other header files and some structures are constructed by lallous from the specification docs

References
--------------
[1] mmc5r03a.pdf from t10.org
[2] MS Windows DDK headers

*/

#ifdef _WIN32

// If this was not defined we should also assume that some structs are not defined as well
#ifndef IOCTL_SCSI_PASS_THROUGH_DIRECT

#define IOCTL_SCSI_PASS_THROUGH_DIRECT 0x4D014
#define SCSI_IOCTL_DATA_OUT          0
#define SCSI_IOCTL_DATA_IN           1
#define SCSI_IOCTL_DATA_UNSPECIFIED  2

// Reference [2]
typedef struct PACKED _SCSI_PASS_THROUGH_DIRECT 
{
	uint16_t Length;
	uint8_t ScsiStatus;
	uint8_t PathId;
	uint8_t TargetId;
	uint8_t Lun;
	uint8_t CdbLength;
	uint8_t SenseInfoLength;
	uint8_t DataIn;
	uint32_t DataTransferLength;
	uint32_t TimeOutValue;
	void *DataBuffer;
	uint32_t SenseInfoOffset;
	uint8_t Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

#endif /* IOCTL_SCSI_PASS_THROUGH_DIRECT */

#endif /* __LIBGENSCD_GENSCD_SCSI_WIN32_H__ */
