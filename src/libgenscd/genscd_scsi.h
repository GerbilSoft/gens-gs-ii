// SCSI structs.
// Based on http://www.codeproject.com/KB/system/mydvdregion.aspx
// Also based on Gens/GS's aspi.h.

#ifndef __LIBGENSCD_SCSI_H__
#define __LIBGENSCD_SCSI_H__

#include <stdint.h>

// Packed struct attribute.
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

// On Windows, pshpack1.h is needed to byte-align structs.
#ifdef _WIN32
#include "pshpack1.h"
#endif

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

#endif /* _WIN32 */

#pragma pack(1)

/**
 * SCSI command definitions.
 * From Gens/GS's aspi.h
 */

//***************************************************************************
//                          %%% TARGET STATUS VALUES %%%
//***************************************************************************
#define STATUS_GOOD     0x00    // Status Good
#define STATUS_CHKCOND  0x02    // Check Condition
#define STATUS_CONDMET  0x04    // Condition Met
#define STATUS_BUSY     0x08    // Busy
#define STATUS_INTERM   0x10    // Intermediate
#define STATUS_INTCDMET 0x14    // Intermediate-condition met
#define STATUS_RESCONF  0x18    // Reservation conflict
#define STATUS_COMTERM  0x22    // Command Terminated
#define STATUS_QFULL    0x28    // Queue full

//***************************************************************************
//                      %%% SCSI MISCELLANEOUS EQUATES %%%
//***************************************************************************
#define MAXLUN          7       // Maximum Logical Unit Id
#define MAXTARG         7       // Maximum Target Id
#define MAX_SCSI_LUNS   64      // Maximum Number of SCSI LUNs
#define MAX_NUM_HA      8       // Maximum Number of SCSI HA's

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
//
//                          %%% SCSI COMMAND OPCODES %%%
//
///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

//***************************************************************************
//               %%% Commands for all Device Types %%%
//***************************************************************************
#define SCSI_CHANGE_DEF 0x40    // Change Definition (Optional)
#define SCSI_COMPARE    0x39    // Compare (O)
#define SCSI_COPY       0x18    // Copy (O)
#define SCSI_COP_VERIFY 0x3A    // Copy and Verify (O)
#define SCSI_INQUIRY    0x12    // Inquiry (MANDATORY)
#define SCSI_LOG_SELECT 0x4C    // Log Select (O)
#define SCSI_LOG_SENSE  0x4D    // Log Sense (O)
#define SCSI_MODE_SEL6  0x15    // Mode Select 6-byte (Device Specific)
#define SCSI_MODE_SEL10 0x55    // Mode Select 10-byte (Device Specific)
#define SCSI_MODE_SEN6  0x1A    // Mode Sense 6-byte (Device Specific)
#define SCSI_MODE_SEN10 0x5A    // Mode Sense 10-byte (Device Specific)
#define SCSI_READ_BUFF  0x3C    // Read Buffer (O)
#define SCSI_REQ_SENSE  0x03    // Request Sense (MANDATORY)
#define SCSI_SEND_DIAG  0x1D    // Send Diagnostic (O)
#define SCSI_TST_U_RDY  0x00    // Test Unit Ready (MANDATORY)
#define SCSI_WRITE_BUFF 0x3B    // Write Buffer (O)

//***************************************************************************
//            %%% Commands Unique to Direct Access Devices %%%
//***************************************************************************
#define SCSI_COMPARE    0x39    // Compare (O)
#define SCSI_FORMAT     0x04    // Format Unit (MANDATORY)
#define SCSI_LCK_UN_CAC 0x36    // Lock Unlock Cache (O)
#define SCSI_PREFETCH   0x34    // Prefetch (O)
#define SCSI_MED_REMOVL 0x1E    // Prevent/Allow medium Removal (O)
#define SCSI_READ6      0x08    // Read 6-byte (MANDATORY)
#define SCSI_READ10     0x28    // Read 10-byte (MANDATORY)
#define SCSI_RD_CAPAC   0x25    // Read Capacity (MANDATORY)
#define SCSI_RD_DEFECT  0x37    // Read Defect Data (O)
#define SCSI_READ_LONG  0x3E    // Read Long (O)
#define SCSI_REASS_BLK  0x07    // Reassign Blocks (O)
#define SCSI_RCV_DIAG   0x1C    // Receive Diagnostic Results (O)
#define SCSI_RELEASE    0x17    // Release Unit (MANDATORY)
#define SCSI_REZERO     0x01    // Rezero Unit (O)
#define SCSI_SRCH_DAT_E 0x31    // Search Data Equal (O)
#define SCSI_SRCH_DAT_H 0x30    // Search Data High (O)
#define SCSI_SRCH_DAT_L 0x32    // Search Data Low (O)
#define SCSI_SEEK6      0x0B    // Seek 6-Byte (O)
#define SCSI_SEEK10     0x2B    // Seek 10-Byte (O)
#define SCSI_SEND_DIAG  0x1D    // Send Diagnostics (MANDATORY)
#define SCSI_SET_LIMIT  0x33    // Set Limits (O)
#define SCSI_START_STP  0x1B    // Start/Stop Unit (O)
#define SCSI_SYNC_CACHE 0x35    // Synchronize Cache (O)
#define SCSI_VERIFY     0x2F    // Verify (O)
#define SCSI_WRITE6     0x0A    // Write 6-Byte (MANDATORY)
#define SCSI_WRITE10    0x2A    // Write 10-Byte (MANDATORY)
#define SCSI_WRT_VERIFY 0x2E    // Write and Verify (O)
#define SCSI_WRITE_LONG 0x3F    // Write Long (O)
#define SCSI_WRITE_SAME 0x41    // Write Same (O)

//***************************************************************************
//          %%% Commands Unique to Sequential Access Devices %%%
//***************************************************************************
#define SCSI_ERASE      0x19    // Erase (MANDATORY)
#define SCSI_LOAD_UN    0x1B    // Load/Unload (O)
#define SCSI_LOCATE     0x2B    // Locate (O)
#define SCSI_RD_BLK_LIM 0x05    // Read Block Limits (MANDATORY)
#define SCSI_READ_POS   0x34    // Read Position (O)
#define SCSI_READ_REV   0x0F    // Read Reverse (O)
#define SCSI_REC_BF_DAT 0x14    // Recover Buffer Data (O)
#define SCSI_RESERVE    0x16    // Reserve Unit (MANDATORY)
#define SCSI_REWIND     0x01    // Rewind (MANDATORY)
#define SCSI_SPACE      0x11    // Space (MANDATORY)
#define SCSI_VERIFY_T   0x13    // Verify (Tape) (O)
#define SCSI_WRT_FILE   0x10    // Write Filemarks (MANDATORY)

//***************************************************************************
//                %%% Commands Unique to Printer Devices %%%
//***************************************************************************
#define SCSI_PRINT      0x0A    // Print (MANDATORY)
#define SCSI_SLEW_PNT   0x0B    // Slew and Print (O)
#define SCSI_STOP_PNT   0x1B    // Stop Print (O)
#define SCSI_SYNC_BUFF  0x10    // Synchronize Buffer (O)

//***************************************************************************
//               %%% Commands Unique to Processor Devices %%%
//***************************************************************************
#define SCSI_RECEIVE    0x08        // Receive (O)
#define SCSI_SEND       0x0A        // Send (O)

//***************************************************************************
//              %%% Commands Unique to Write-Once Devices %%%
//***************************************************************************
#define SCSI_MEDIUM_SCN 0x38    // Medium Scan (O)
#define SCSI_SRCHDATE10 0x31    // Search Data Equal 10-Byte (O)
#define SCSI_SRCHDATE12 0xB1    // Search Data Equal 12-Byte (O)
#define SCSI_SRCHDATH10 0x30    // Search Data High 10-Byte (O)
#define SCSI_SRCHDATH12 0xB0    // Search Data High 12-Byte (O)
#define SCSI_SRCHDATL10 0x32    // Search Data Low 10-Byte (O)
#define SCSI_SRCHDATL12 0xB2    // Search Data Low 12-Byte (O)
#define SCSI_SET_LIM_10 0x33    // Set Limits 10-Byte (O)
#define SCSI_SET_LIM_12 0xB3    // Set Limits 10-Byte (O)
#define SCSI_VERIFY10   0x2F    // Verify 10-Byte (O)
#define SCSI_VERIFY12   0xAF    // Verify 12-Byte (O)
#define SCSI_WRITE12    0xAA    // Write 12-Byte (O)
#define SCSI_WRT_VER10  0x2E    // Write and Verify 10-Byte (O)
#define SCSI_WRT_VER12  0xAE    // Write and Verify 12-Byte (O)

//***************************************************************************
//                %%% Commands Unique to CD-ROM Devices %%%
//***************************************************************************
#define SCSI_PLAYAUD_10 0x45    // Play Audio 10-Byte (O)
#define SCSI_PLAYAUD_12 0xA5    // Play Audio 12-Byte 12-Byte (O)
#define SCSI_PLAYAUDMSF 0x47    // Play Audio MSF (O)
#define SCSI_PLAYA_TKIN 0x48    // Play Audio Track/Index (O)
#define SCSI_PLYTKREL10 0x49    // Play Track Relative 10-Byte (O)
#define SCSI_PAUSE_RESU 0x4B    // Pause/Resume Scan/Play (O)
#define SCSI_STOP_PL_SC 0x4E    // Stop Scan/Play (O)
#define SCSI_PLYTKREL12 0xA9    // Play Track Relative 12-Byte (O)
#define SCSI_READCDCAP  0x25    // Read CD-ROM Capacity (MANDATORY)
#define SCSI_READHEADER 0x44    // Read Header (O)
#define SCSI_SUBCHANNEL 0x42    // Read Subchannel (O)
#define SCSI_READ_TOC   0x43    // Read TOC (O)
#define SCSI_READ_MSF   0xB9    // Read CD MSF format (O)
#define SCSI_SET_SPEED  0xBB    // Set CD speed (O)
#define SCSI_GET_MCH_ST 0xBD    // Mechanic Status (O)
#define SCSI_READ_LBA   0xBE    // Read CD LBA format (O)

//***************************************************************************
//                %%% Commands Unique to Scanner Devices %%%
//***************************************************************************
#define SCSI_GETDBSTAT  0x34    // Get Data Buffer Status (O)
#define SCSI_GETWINDOW  0x25    // Get Window (O)
#define SCSI_OBJECTPOS  0x31    // Object Postion (O)
#define SCSI_SCAN       0x1B    // Scan (O)
#define SCSI_SETWINDOW  0x24    // Set Window (MANDATORY)

//***************************************************************************
//           %%% Commands Unique to Optical Memory Devices %%%
//***************************************************************************
#define SCSI_UpdateBlk  0x3D    // Update Block (O)

//***************************************************************************
//           %%% Commands Unique to Medium Changer Devices %%%
//***************************************************************************
#define SCSI_EXCHMEDIUM 0xA6    // Exchange Medium (O)
#define SCSI_INITELSTAT 0x07    // Initialize Element Status (O)
#define SCSI_POSTOELEM  0x2B    // Position to Element (O)
#define SCSI_REQ_VE_ADD 0xB5    // Request Volume Element Address (O)
#define SCSI_SENDVOLTAG 0xB6    // Send Volume Tag (O)

//***************************************************************************
//            %%% Commands Unique to Communication Devices %%%
//***************************************************************************
#define SCSI_GET_MSG_6  0x08    // Get Message 6-Byte (MANDATORY)
#define SCSI_GET_MSG_10 0x28    // Get Message 10-Byte (O)
#define SCSI_GET_MSG_12 0xA8    // Get Message 12-Byte (O)
#define SCSI_SND_MSG_6  0x0A    // Send Message 6-Byte (MANDATORY)
#define SCSI_SND_MSG_10 0x2A    // Send Message 10-Byte (O)
#define SCSI_SND_MSG_12 0xAA    // Send Message 12-Byte (O)

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
//
//                    %%% END OF SCSI COMMAND OPCODES %%%
//
///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

//***************************************************************************
//                      %%% Request Sense Data Format %%%
//***************************************************************************
typedef struct
{
	uint8_t ErrorCode;		// Error Code (70H or 71H)
	uint8_t SegmentNum;		// Number of current segment descriptor
	uint8_t SenseKey;		// Sense Key(See bit definitions too)
	uint8_t InfoByte0;		// Information MSB
	uint8_t InfoByte1;		// Information MID
	uint8_t InfoByte2;		// Information MID
	uint8_t InfoByte3;		// Information LSB
	uint8_t AddSenLen;		// Additional Sense Length
	uint8_t ComSpecInf0;		// Command Specific Information MSB
	uint8_t ComSpecInf1;		// Command Specific Information MID
	uint8_t ComSpecInf2;		// Command Specific Information MID
	uint8_t ComSpecInf3;		// Command Specific Information LSB
	uint8_t AddSenseCode;		// Additional Sense Code
	uint8_t AddSenQual;		// Additional Sense Code Qualifier
	uint8_t FieldRepUCode;		// Field Replaceable Unit Code
	uint8_t SenKeySpec15;		// Sense Key Specific 15th byte
	uint8_t SenKeySpec16;		// Sense Key Specific 16th byte
	uint8_t SenKeySpec17;		// Sense Key Specific 17th byte
	uint8_t AddSenseBytes;		// Additional Sense Bytes
} SENSE_DATA_FMT;

//***************************************************************************
//                       %%% REQUEST SENSE ERROR CODE %%%
//***************************************************************************
#define SERROR_CURRENT  0x70    // Current Errors
#define SERROR_DEFERED  0x71    // Deferred Errors

//***************************************************************************
//                   %%% REQUEST SENSE BIT DEFINITIONS %%%
//***************************************************************************
#define SENSE_VALID     0x80    // Byte 0 Bit 7
#define SENSE_FILEMRK   0x80    // Byte 2 Bit 7
#define SENSE_EOM       0x40    // Byte 2 Bit 6
#define SENSE_ILI       0x20    // Byte 2 Bit 5

//***************************************************************************
//               %%% REQUEST SENSE SENSE KEY DEFINITIONS %%%
//***************************************************************************
#define KEY_NOSENSE     0x00    // No Sense
#define KEY_RECERROR    0x01    // Recovered Error
#define KEY_NOTREADY    0x02    // Not Ready
#define KEY_MEDIUMERR   0x03    // Medium Error
#define KEY_HARDERROR   0x04    // Hardware Error
#define KEY_ILLGLREQ    0x05    // Illegal Request
#define KEY_UNITATT     0x06    // Unit Attention
#define KEY_DATAPROT    0x07    // Data Protect
#define KEY_BLANKCHK    0x08    // Blank Check
#define KEY_VENDSPEC    0x09    // Vendor Specific
#define KEY_COPYABORT   0x0A    // Copy Abort
#define KEY_EQUAL       0x0C    // Equal (Search)
#define KEY_VOLOVRFLW   0x0D    // Volume Overflow
#define KEY_MISCOMP     0x0E    // Miscompare (Search)
#define KEY_RESERVED    0x0F    // Reserved

//***************************************************************************
//                %%% PERIPHERAL DEVICE TYPE DEFINITIONS %%%
//***************************************************************************
#define DTYPE_DASD      0x00    // Disk Device
#define DTYPE_SEQD      0x01    // Tape Device
#define DTYPE_PRNT      0x02    // Printer
#define DTYPE_PROC      0x03    // Processor
#define DTYPE_WORM      0x04    // Write-once read-multiple
#define DTYPE_CROM      0x05    // CD-ROM device
#define DTYPE_CDROM     0x05    // CD-ROM device
#define DTYPE_SCAN      0x06    // Scanner device
#define DTYPE_OPTI      0x07    // Optical memory device
#define DTYPE_JUKE      0x08    // Medium Changer device
#define DTYPE_COMM      0x09    // Communications device
#define DTYPE_RESL      0x0A    // Reserved (low)
#define DTYPE_RESH      0x1E    // Reserved (high)
#define DTYPE_UNKNOWN   0x1F    // Unknown or no device type

//***************************************************************************
//                %%% ANSI APPROVED VERSION DEFINITIONS %%%
//***************************************************************************
#define ANSI_MAYBE      0x0     // Device may or may not be ANSI approved stand
#define ANSI_SCSI1      0x1     // Device complies to ANSI X3.131-1986 (SCSI-1)
#define ANSI_SCSI2      0x2     // Device complies to SCSI-2
#define ANSI_RESLO      0x3     // Reserved (low)
#define ANSI_RESHI      0x7     // Reserved (high)

/* SCSI Miscellaneous Stuff */
#define SENSE_LEN			14
#define SRB_DIR_SCSI			0x00
#define SRB_POSTING			0x01
#define SRB_ENABLE_RESIDUAL_COUNT	0x04
#define SRB_DIR_IN			0x08
#define SRB_DIR_OUT			0x10

/* ASPI Command Definitions */
#define SC_HA_INQUIRY			0x00
#define SC_GET_DEV_TYPE			0x01
#define SC_EXEC_SCSI_CMD		0x02
#define SC_ABORT_SRB			0x03
#define SC_RESET_DEV			0x04
#define SC_SET_HA_PARMS			0x05
#define SC_GET_DISK_INFO		0x06

/* SRB status codes */
#define SS_PENDING			0x00
#define SS_COMP				0x01
#define SS_ABORTED			0x02
#define SS_ABORT_FAIL			0x03
#define SS_ERR				0x04

#define SS_INVALID_CMD			0x80
#define SS_INVALID_HA			0x81
#define SS_NO_DEVICE			0x82

#define SS_INVALID_SRB			0xE0
#define SS_OLD_MANAGER			0xE1
#define SS_BUFFER_ALIGN			0xE1 // Win32
#define SS_ILLEGAL_MODE			0xE2
#define SS_NO_ASPI			0xE3
#define SS_FAILED_INIT			0xE4
#define SS_ASPI_IS_BUSY			0xE5
#define SS_BUFFER_TO_BIG		0xE6
#define SS_MISMATCHED_COMPONENTS	0xE7 // DLLs/EXE version mismatch
#define SS_NO_ADAPTERS			0xE8
#define SS_INSUFFICIENT_RESOURCES	0xE9
#define SS_ASPI_IS_SHUTDOWN		0xEA
#define SS_BAD_INSTALL			0xEB


/* Host status codes */
#define HASTAT_OK			0x00
#define HASTAT_SEL_TO			0x11
#define HASTAT_DO_DU			0x12
#define HASTAT_BUS_FREE			0x13
#define HASTAT_PHASE_ERR		0x14

#define HASTAT_TIMEOUT			0x09
#define HASTAT_COMMAND_TIMEOUT		0x0B
#define HASTAT_MESSAGE_REJECT		0x0D
#define HASTAT_BUS_RESET		0x0E
#define HASTAT_PARITY_ERROR		0x0F
#define HASTAT_REQUEST_SENSE_FAILED	0x10


/* Additional definitions */
/* SCSI Miscellaneous Stuff */
#define SRB_EVENT_NOTIFY		0x40
#define RESIDUAL_COUNT_SUPPORTED	0x02
#define MAX_SRB_TIMEOUT			1080001u
#define DEFAULT_SRB_TIMEOUT		1080001u

/* These are defined by MS but not adaptec */
#define SRB_DATA_SG_LIST		0x02
#define WM_ASPIPOST			0x4D42


/* ASPI Command Definitions */
#define SC_RESCAN_SCSI_BUS		0x07
#define SC_GETSET_TIMEOUTS		0x08

/* SRB Status.. MS defined */
#define SS_SECURITY_VIOLATION		0xE2 // Replaces SS_INVALID_MODE
/*** END DEFS */

/**
 * END: SCSI command definitions.
 */

// Reference [2]
/**
 * CDB_SCSI: SCSI command data block.
 */
typedef struct PACKED _CDB_SCSI
{
	uint8_t OperationCode;
	uint8_t Reserved1 : 5;
	uint8_t LogicalUnitNumber : 3;
	uint8_t PageCode;
	uint8_t IReserved;
	uint8_t AllocationLength;
	uint8_t Control;
} CDB_SCSI;

/**
 * CDB_SCSI_RD_CAPAC: SCSI CDB for SCSI_RD_CAPAC.
 * Reference: http://en.wikipedia.org/wiki/SCSI_Read_Capacity_Command
 */
typedef struct PACKED _CDB_SCSI_RD_CAPAC
{
	uint8_t OperationCode;
	uint8_t RelAdr : 1;
	uint8_t Reserved1 : 4;
	uint8_t LogicalUnitNumber : 3;
	uint32_t LBA;			// BE32: LBA for use with relative addressing. (if RelAdr == 1)
	uint8_t Reserved2[2];
	uint8_t PMI : 1;
	uint8_t Reserved3 : 7;
	uint8_t Control;
} CDB_SCSI_RD_CAPAC;

/**
 * SCSI_DATA_TST_U_RDY: SCSI_TST_U_RDY returned data.
 * Reference: http://en.wikipedia.org/wiki/SCSI_Test_Unit_Ready_Command
 */
typedef struct PACKED _SCSI_DATA_TST_U_RDY 
{
	uint8_t OperationCode;	// SCSI_TST_U_RDY == 0x00
	uint8_t Reserved1 : 5;
	uint8_t LogicalUnitNumber : 3;
	uint8_t Reserved2;
	uint8_t Reserved3;
	uint8_t Reserved4;
	uint8_t Control;
} SCSI_DATA_TST_U_RDY;

/**
 * SCSI_DATA_REQ_SENSE: SCSI_REQ_SENSE returned data.
 * Reference: http://en.wikipedia.org/wiki/SCSI_Request_Sense_Command
 */
typedef struct PACKED _SCSI_DATA_REQ_SENSE
{
	uint8_t ResponseCode : 7;
	uint8_t Valid : 1;
	uint8_t SegmentNumber;
	uint8_t SenseKey : 4;
	uint8_t Reserved1 : 1;
	uint8_t ILI : 1;
	uint8_t EOM : 1;
	uint8_t Filemark : 1;
	uint8_t Information;
	uint8_t AdditionalSenseLength;
	uint8_t CmdSpecificInfo[4];
	uint8_t AdditionalSenseCode;
	uint8_t AdditionalSenseCodeQualifier;
	uint8_t FieldReplaceableUnitCode;
	uint8_t SenseKeySpecific1 : 7;
	uint8_t SKSV : 1;
	uint8_t SenseKeySpecific2;
	uint8_t SenseKeySpecific3;
	uint8_t AdditionalSenseBytes[4];
} SCSI_DATA_REQ_SENSE;

/**
 * SCSI_INQUIRY_STD_DATA: Returned data from SCSI_INQUIRY.
 */
typedef struct PACKED _SCSI_INQUIRY_STD_DATA
{
	uint8_t peripheral_device_type : 5;
	uint8_t peripheral_qualifier: 3;
	uint8_t rsvrd : 7;
	uint8_t rmb: 1;
	uint8_t version;
	uint8_t RESPONSE_DATA_FORMAT;	// 7 = AERC, 6 = Obsolete, 5 = NormACA, 4 = HiSup 3-0 = Response data format.
					// If ANSI Version = 0, this is ATAPI and bits 7-4 = ATAPI version.
	uint8_t	ADDITIONAL_LENGTH;	// Number of additional bytes available in inquiry data
	uint8_t	SCCSReserved;		// SCC-2 device flag and reserved fields
	uint8_t	flags1;			// First byte of support flags
	uint8_t	flags2;			// Second byte of support flags (Byte 7)
	char	vendor_id[8];
	char	product_id[16];
	char	product_revision_level[4];
} SCSI_INQUIRY_STD_DATA;

/**
 * SCSI_DATA_RD_CAPAC: SCSI_RD_CAPAC returned data.
 * Reference: http://en.wikipedia.org/wiki/SCSI_Read_Capacity_Command
 */
typedef struct PACKED _SCSI_DATA_RD_CAPAC
{
	uint32_t LBA;		// BE32: Maximum LBA
	uint32_t BlockLength;	// BE32: Block length (in bytes)
} SCSI_DATA_RD_CAPAC;

/**
 * CDB_READ_TOC: SCSI CDB for SCSI_READ_TOC.
 */
typedef struct PACKED _CDB_SCSI_READ_TOC
{
	uint8_t OperationCode;	// SCSI_READ_TOC == 0x43
	uint8_t MSF;		// Set to 0 for LBA; set to 2 for MSF.
	uint8_t Format;
	uint8_t Reserved[3];
	uint8_t TrackSessionNumber;	// First track/session number to read.
	uint16_t AllocationLength;	// BE16
	uint8_t Control;
} CDB_SCSI_READ_TOC;

#define SCSI_READ_TOC_FORMAT_LBA 0x00
#define SCSI_READ_TOC_FORMAT_MSF 0x02

/**
 * CD-ROM Table of Contents: Track entry.
 */
typedef struct PACKED _SCSI_CDROM_TOC_TRACK
{
	uint8_t rsvd1;
	uint8_t ControlADR;	// Track type.
	uint8_t TrackNumber;
	uint8_t rsvd2;
	uint32_t StartAddress;	// BE32
} SCSI_CDROM_TOC_TRACK;

/**
 * CD-ROM Table of Contents.
 */
typedef struct PACKED _SCSI_CDROM_TOC
{
	uint16_t DataLen;	// BE16
	uint8_t FirstTrackNumber;
	uint8_t LastTrackNumber;
	SCSI_CDROM_TOC_TRACK Tracks[100];
} SCSI_CDROM_TOC;

// Data tracks are identified by bit 2 (0x04) being set in ControlADR.
#define IS_DATA_TRACK(ControlADR) (!!((ControlADR) & 0x04))
#define IS_AUDIO_TRACK(ControlADR) (!((ControlADR) & 0x04))

/****************************************************************/

/** SCSI error code macros. (From udev) **/
#define ERRCODE(s)	((((s)[2] & 0x0F) << 16) | ((s)[12] << 8) | ((s)[13]))
#define SK(errcode)	(((errcode) >> 16) & 0xF)
#define ASC(errcode)	(((errcode) >> 8) & 0xFF)
#define ASCQ(errcode)	((errcode) & 0xFF)

/** MMC commands. (Based on udev) **/
#define MMC_GET_CONFIGURATION		0x46
#define MMC_READ_DISC_INFORMATION	0x51

/**
 * CDB for the MMC GET CONFIGURATION command.
 */
typedef struct PACKED _CDB_MMC_GET_CONFIGURATION
{
	uint8_t OperationCode;		// MMC_GET_CONFIGURATION (0x46)
	uint8_t RT;
	uint16_t StartingFeatureNumber;	// BE16
	uint8_t Reserved[3];
	uint16_t AllocationLength;	// BE16
	uint8_t Control;
} CDB_MMC_GET_CONFIGURAITON;

/**
 * Response from the MMC GET CONFIGURATION command.
 * Header only; Feature descriptors are variable-length.
 */
typedef struct PACKED _SCSI_MMC_GET_CONFIGURATION_HEADER_DATA
{
	uint32_t DataLength;		// BE32
	uint8_t Reserved[2];
	uint16_t CurrentProfile;	// BE16
} SCSI_MMC_GET_CONFIGURATION_HEADER_DATA;

/**
 * CDB for the MMC READ DISC INFORMATION command.
 */
typedef struct PACKED _CDB_MMC_READ_DISC_INFORMATION
{
	uint8_t OperationCode;		// MMC_GET_CONFIGURATION (0x46)
	uint8_t DataType;
	uint8_t Reserved[5];
	uint16_t AllocationLength;	// BE16
	uint8_t Control;
} CDB_MMC_READ_DISC_INFORMATION;

// Data types for CDB_MMC_READ_DISC_INFORMATION.
#define MMC_READ_DISC_INFORMATION_DATATYPE_STANDARD	0x00
#define MMC_READ_DISC_INFORMATION_DATATYPE_TRACK	0x01
#define MMC_READ_DISC_INFORMATION_DATATYPE_POW		0x02

/**
 * Response from the MMC_READ_DISC_INFORMATION command.
 */
typedef struct PACKED _SCSI_MMC_READ_DISC_INFORMATION_DATA
{
	uint16_t DiscInfoLength;		// BE16
	uint8_t DiscStatusFlags;
	uint8_t FirstTrackNumber;
	uint8_t NumSessionsLSB;
	uint8_t FirstTrackNumberInLastSessionLSB;
	uint8_t LastTrackNumberInLastSessionLSB;
	uint8_t ValidFlags;
	uint8_t DiscType;
	uint8_t NumSessionsMSB;
	uint8_t FirstTrackNumberInLastSessionMSB;
	uint8_t LastTrackNumberInLastSessionMSB;
	uint32_t DiscIdentification;		// BE32
	uint32_t LastSessionLeadInStartLBA;	// BE32
	uint32_t LastPossibleLeadOutStartLBA;	// BE32
	uint32_t DiscBarCode;			// BE32
	uint8_t DiscApplicationCode;

	// We don't need the OPC tables.
	/*
	uint8_t NumOPCTables;
	uint8_t OPCTableEntries[];
	*/
} SCSI_MMC_READ_DISC_INFORMATION_DATA;

/****************************************************************/

// On Windows, pshpack1.h is needed to byte-pack structs.
// poppack.h turns off pshpack1.h, since byte-packing is only needed for the ASPI structs.
#ifdef _WIN32
#include "poppack.h"
#endif

#pragma pack()

#endif /* __LIBGENSCD_SCSI_H__ */
