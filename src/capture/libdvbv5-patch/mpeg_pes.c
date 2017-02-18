#include "mpeg_pes.h"
#include <memory.h>
#include <libdvbv5/descriptors.h>
#include <libdvbv5/dvb-log.h>
#include <libdvbv5/dvb-fe.h>

ssize_t dvb_mpeg_pes_init_patched(struct dvb_v5_fe_parms *parms, const uint8_t *buf, ssize_t buflen, uint8_t *table)
{
	struct dvb_mpeg_pes *pes = (struct dvb_mpeg_pes *) table;
	const uint8_t *p = buf;

	memcpy(table, p, sizeof(struct dvb_mpeg_pes));
	p += sizeof(struct dvb_mpeg_pes);

	bswap32(pes->bitfield);
	bswap16(pes->length);

	if (pes->sync != 0x000001) {
		return -1;
	}

	if (pes->stream_id == DVB_MPEG_STREAM_PADDING) {
		return -1;
	} else if (pes->stream_id == DVB_MPEG_STREAM_MAP ||
		   pes->stream_id == DVB_MPEG_STREAM_PRIVATE_2 ||
		   pes->stream_id == DVB_MPEG_STREAM_ECM ||
		   pes->stream_id == DVB_MPEG_STREAM_EMM ||
		   pes->stream_id == DVB_MPEG_STREAM_DIRECTORY ||
		   pes->stream_id == DVB_MPEG_STREAM_DSMCC ||
		   pes->stream_id == DVB_MPEG_STREAM_H222E ) {
		dvb_logerr("mpeg pes: unsupported stream type 0x%04x", pes->stream_id);
		return -2;
	} else {
		memcpy(pes->optional, p, sizeof(struct dvb_mpeg_pes_optional) -
					 sizeof(pes->optional->pts) -
					 sizeof(pes->optional->dts));
		p += sizeof(struct dvb_mpeg_pes_optional) -
		     sizeof(pes->optional->pts) -
		     sizeof(pes->optional->dts);
		bswap16(pes->optional->bitfield);
		pes->optional->pts = 0;
		pes->optional->dts = 0;

		const uint8_t* optionalStart = p;
		if (pes->optional->PTS_DTS & 2) {
			struct ts_t pts;
			memcpy(&pts, p, sizeof(pts));
			p += sizeof(pts);
			bswap16(pts.bitfield);
			bswap16(pts.bitfield2);
			if (pts.one != 1 || pts.one1 != 1 || pts.one2 != 1) {
				dvb_logwarn("mpeg pes: invalid pts");
			} else {
				pes->optional->pts |= (uint64_t) pts.bits00;
				pes->optional->pts |= (uint64_t) pts.bits15 << 15;
				pes->optional->pts |= (uint64_t) pts.bits30 << 30;
			}
		}
		if (pes->optional->PTS_DTS & 1) {
			struct ts_t dts;
			memcpy(&dts, p, sizeof(dts));
			p += sizeof(dts);
			bswap16(dts.bitfield);
			bswap16(dts.bitfield2);
			pes->optional->dts |= (uint64_t) dts.bits00;
			pes->optional->dts |= (uint64_t) dts.bits15 << 15;
			pes->optional->dts |= (uint64_t) dts.bits30 << 30;
		}
#if 0
		if (pes->optional->ESCR & 1) {
			// TODO:
			p += 6;
		}
		if (pes->optional->ES_rate & 1) {
			// TODO:
			p += 3;
		}
		if (pes->optional->DSM_trick_mode & 1) {
			// TODO:
			p += 1;
		}
		if (pes->optional->additional_copy_info & 1) {
			// TODO:
			p += 1;
		}
		if (pes->optional->PES_CRC & 1) {
			// TODO:
			p += 2;
		}
		if (pes->optional->PES_extension & 1) {
			dvb_mpeg_pes_extension* ext = (dvb_mpeg_pes_extension*)p;
			p += sizeof(dvb_mpeg_pes_extension);
			if (ext->PES_private_data_flag & 1) {
				// TODO:
				p += 16;
			}
			if (ext->pack_header_field_flag & 1) {
				uint8_t pack_field_length = *p;
				// TODO:
				p += pack_field_length;
			}
			if (ext->program_packet_sequence_counter_flag & 1) {
				// TODO
				p += 2;
			}
			if (ext->p_std_buffer_flag & 1) {
				// TODO
				p += 2;
			}
			if (ext->PES_extension_flag_2 & 1) {
				// TODO
				uint8_t pes_extension_field_length = *p & 0x7f;
				p += pes_extension_field_length;
			}
		}
#endif
		p = optionalStart;
		p += pes->optional->length;
	}
	return p - buf;
}