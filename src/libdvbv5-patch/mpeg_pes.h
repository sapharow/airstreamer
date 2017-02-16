#ifndef _MPEG_PES_PATCHED_H
#define _MPEG_PES_PATCHED_H
#include <stdint.h>
#include <unistd.h> /* ssize_t */
#include <libdvbv5/mpeg_pes.h>

struct dvb_mpeg_pes_extension {
	union {
		uint8_t bitfield;
		struct {
			uint8_t PES_extension_flag2:1;
			uint8_t reserved:3;
			uint8_t p_std_buffer_flag:1;
			uint8_t program_packet_sequence_counter_flag:1;
			uint8_t pack_header_field_flag:1;
			uint8_t PES_private_data_flag:1;
		} __attribute__((packed));
	} __attribute__((packed));
} __attribute__((packed));

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a struct dvb_mpeg_pes from buffer
 * @ingroup dvb_table
 *
 * @param parms		struct dvb_v5_fe_parms for log functions
 * @param buf		Buffer
 * @param buflen	Length of buffer
 * @param table		Pointer to allocated struct dvb_mpeg_pes
 *
 * @return		Length of data in table
 *
 * This function copies the length of struct dvb_mpeg_pes
 * to table and fixes endianness. The pointer table has to be
 * allocated on stack or dynamically.
 */
ssize_t dvb_mpeg_pes_init_patched(struct dvb_v5_fe_parms *parms, const uint8_t *buf, ssize_t buflen, uint8_t *table);

#ifdef __cplusplus
}
#endif

#endif