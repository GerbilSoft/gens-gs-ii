/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * genscd_iso9660.h: ISO-9660 data structure definitions.                  *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

/**
 * References:
 * - http://wiki.osdev.org/ISO_9660
 * - http://www.pismotechnic.com/cfs/iso9660-1999.html
 */

#ifndef __LIBGENSCD_GENSCD_ISO9660_H__
#define __LIBGENSCD_GENSCD_ISO9660_H__

#include <stdint.h>

/* ISO-9660 structs are defined on the byte-level, so we must
 * prevent the compiler from adding alignment padding. */
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

#ifdef _WIN32
#include "pshpack1.h"
#endif

#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ISO-9660 uint16_t stored as LSB (little-endian) and MSB (big-endian).
 */
typedef struct _ISO9660_UINT16_LSB_MSB {
	uint16_t lsb;
	uint16_t msb;
} ISO9660_UINT16_LSB_MSB;

/**
 * ISO-9660 uint32_t stored as LSB (little-endian) and MSB (big-endian).
 */
typedef struct _ISO9660_UINT32_LSB_MSB {
	uint32_t lsb;
	uint32_t msb;
} ISO9660_UINT32_LSB_MSB;

/**
 * ISO-9660 volume descriptor.
 */
typedef struct _ISO9660_VOLUME_DESCRIPTOR
{
	uint8_t vdtype;		/* Volume descriptor type. */
	char magic[5];		/* "CD001" */
	uint8_t version;

	union {
		/* 0x00: El Torito boot record. */
		struct {
			char boot_system_id[32];
			char boot_id[32];
			uint8_t system[1977];
		} boot;

		/* 0x01: Primary Volume Descriptor. */
		struct {
			uint8_t unused1;	/* Always 0x00. */
			char sys_id[32];
			char vol_id[32];
			uint8_t unused2[8];	/* All zeroes. */
			ISO9660_UINT32_LSB_MSB volume_space_size;
			uint8_t unused3[32];
			ISO9660_UINT16_LSB_MSB volume_set_size;
			ISO9660_UINT16_LSB_MSB volume_seq_num;
			ISO9660_UINT16_LSB_MSB logical_block_size;
			ISO9660_UINT32_LSB_MSB path_table_size;
			uint32_t path_table_addr_L;	/* LE32 */
			uint32_t path_table_addr_opt_L;	/* LE32 */
			uint32_t path_table_addr_M;	/* BE32 */
			uint32_t path_table_addr_opt_M;	/* BE32 */
			uint8_t root_dir_entry[34];
			char volume_set_id[128];
			char publisher_id[128];
			char data_preparer_id[128];
			char application_id[128];
			char copyright_file_id[38];
			char abstract_file_id[36];
			char bibliographic_file_id[37];
			uint8_t vol_creation_date_time[17];	/* dec-datetime */
			uint8_t vol_modification_date_time[17];	/* dec-datetime */
			uint8_t vol_expiration_date_time[17];	/* dec-datetime */
			uint8_t vol_effective_date_time[17];	/* dec-datetime */
			uint8_t file_structure_version;		/* Always 0x00 */
			uint8_t unused4;
			uint8_t application_used[512];
			uint8_t reserved[653];
		} pvd;

		/* 0xFF: Volume Descriptor Set Terminator. */
		struct {
			uint8_t reserved[2048-7];
		} term;
	};
} ISO9660_VOLUME_DESCRIPTOR;

/* ISO-9660 magic number. */
#define ISO9660_MAGIC	"CD001"

/* Volume descriptor types. */
#define ISO9660_VDTYPE_BOOT	0x00	/* El Torito boot record */
#define ISO9660_VDTYPE_PVD	0x01	/* Primary Volume Descriptor */
#define ISO9660_VDTYPE_SVG	0x02	/* Supplementary Volume Descriptor, e.g. Joliet */
#define ISO9660_VDTYPE_TERM	0xFF	/* Volume Descriptor Set Terminator */

#ifdef __cplusplus
}
#endif

/* Turn off compiler-specific struct packing. */
#ifdef _WIN32
#include "poppack.h"
#endif

#pragma pack()

#endif /* __LIBGENSCD_GENSCD_ISO9660_H__ */
