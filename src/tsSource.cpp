#include <tsSource.h>
#include <vector>
#include <set>
#include <libdvbv5/mpeg_ts.h>
#include <libdvbv5/mpeg_pes.h>
#include <libdvbv5/descriptors.h>
#include <memory.h>
#include <program.h>
#include <cassert>

/**
 * Size of buffer (bytes) to read from source implementation
 */
#define READ_BUFFER 8192

/**
 * If buffer needs to be increased to particular size then it increases to 
 * this size plus margin to reduce memory re-allocs
 */
#define BUFFER_INCREASE_MARGIN	1024

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

static ssize_t dvb_mpeg_pes_init2(struct dvb_v5_fe_parms *parms, const uint8_t *buf, ssize_t buflen, uint8_t *table)
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
//				dvb_logwarn("mpeg pes: invalid pts");
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

namespace fp {
	namespace cap {

		struct TSSource::Payload {
			Payload(ProgramRef _program, uint32_t _pid)
			: pid(_pid)
			, program(_program)
			{
				pesTable = (dvb_mpeg_pes*)malloc(sizeof(dvb_mpeg_pes) + sizeof(dvb_mpeg_pes_optional));
			}

			~Payload() {
				free(pesTable);
			}

			/**
			 * Reset payload writer to start new payload
			 * @param[in] ctr New counter value
			 */
			void reset() {
				if (dataSize >= 3) {
					if ((data[0] == 0) && (data[1] == 0) && (data[2] == 1)) {
						// De-packetise
						ssize_t pesSize = dvb_mpeg_pes_init2(nullptr, data.data(), dataSize, (uint8_t*)pesTable);

//						printf("Optional fields length = %u\n", pesTable->optional->length);
						printf("stream_id = %u\n", pesTable->stream_id);
						size_t nBytesToSkip = pesSize;//sizeof(dvb_mpeg_pes) + pesTable->length;
						size_t nBytesToSupply = dataSize - nBytesToSkip;
						if (nBytesToSupply && program) {
							program->supplyStreamPayload(data.data() + nBytesToSkip, nBytesToSupply);
						}

						if (pesTable->optional->data_alignment_indicator) {
							if (program) {
							}
						} else {
							printf("Data is NOT aligned. TODO!!!\n");
						}

	//					printf("pesSize = %u; dataSize = %u; pesLength=%u\n", pesSize, dataSize, pesTable->length);
					} else {
						/*
						for (size_t i=0; i<10; i++) {
							printf("0x%02x ", data[i]);
						}
						printf("\n");
						*/
//						printf("PID=%u\n", pid);
					}

				}
				dataSize = 0;
				payloadStart = true;
				error = false;
			}

			/**
			 * Append data to payload
			 */
			void append(uint8_t* src, size_t size, size_t ctr) {
				if (payloadStart) {
					payloadStart = false;
					counter = ctr;
				} else {
					// Check counter
					if (error) {
						return;
					} else {
						if (counter == ctr) {
							// Duplicate - ignore
							return;
						}
						counter++;
						counter &= 0xf;
						if (counter != ctr) {
							// Counter mismatch
							printf("Counter mismatch. Expected %u, given %u\n", (unsigned)counter, (unsigned)ctr);
							error = true;
							return;
						}
					}
				}

				if (data.size() < (dataSize + size)) {
					data.resize(dataSize + size + BUFFER_INCREASE_MARGIN);
				}
				memcpy(data.data() + dataSize, src, size);
				dataSize += size;
			}

			std::vector<uint8_t> data;
			size_t dataSize = 0;

		private:
			size_t counter = 0;
			bool   error = true;
			bool   payloadStart = false;
			uint32_t pid;
			dvb_mpeg_pes* pesTable;
			ProgramRef program;
		};

		TSSource::TSSource() 
		{
		}

		TSSource::~TSSource() {
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			if (m_Started) {
				m_Started = false;
				m_Thread->join();
			}
		}

		void TSSource::start() {
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			if (!m_Started) {
				doStart();
				// Start reading thread
				m_Started = true;
				m_Thread = std::make_shared<std::thread>(mainLoop, this);
			}
		}

		void TSSource::stop() {
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			if (m_Started) {
				doStop();
				// Stop reading thread
				m_Started = false;
				m_Thread->join();
			}
		}

		bool TSSource::running() {
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			return m_Started && !m_ThreadFinished;
		}

		void TSSource::mainLoop(TSSource* thiz) {
			// Allocate buffer
			std::vector<Payload*> pidPayload(8192);

			std::vector<uint8_t> buffer(READ_BUFFER);
			thiz->m_ThreadFinished = false;
			dvb_mpeg_ts* tsPacket = (dvb_mpeg_ts*)malloc(sizeof(dvb_mpeg_ts) + sizeof(dvb_mpeg_ts_adaption));
			
			uint8_t* bufferWritePtr = buffer.data();
			const uint8_t* bufferEnd = buffer.data() + buffer.size();
			while (thiz->m_Started) {
				size_t nBytesRead = thiz->readDataInto(bufferWritePtr, bufferEnd - bufferWritePtr);
				if (nBytesRead == 0) {
					// Exit thread
					break;
				}
				bufferWritePtr += nBytesRead;

				// Go through whole buffer
				uint8_t* bufferReadPtr = buffer.data();
				while (bufferWritePtr - bufferReadPtr >= 188) {
					// Buffer contains another TS packet to analyse
					// TODO: Workout bufferReadPtr[0..187]
					ssize_t tableSize;
					if (bufferReadPtr[0] == DVB_MPEG_TS) {
						tableSize = dvb_mpeg_ts_init(nullptr, bufferReadPtr, bufferWritePtr - bufferReadPtr, (uint8_t*)tsPacket, &tableSize);
						if (tableSize <= 188) {
							switch (tsPacket->pid) {
									case 0x1fff:
										// Do nothing for null packet
									break;
									case 0x1ffb:
										// Used by DigiCipher 2/ATSC MGT metadata
									break;

									default: {
										if (!pidPayload[tsPacket->pid]) {
											// Allocate payload info
											ProgramRef program;
											std::lock_guard<std::recursive_mutex> lock(thiz->m_Mutex);
											auto pp = thiz->m_ProgramProvider;
											if (pp) {
												program = pp(tsPacket->pid);
											}
											pidPayload[tsPacket->pid] = new Payload(program, tsPacket->pid);
										}

										if (tsPacket->payload) {
											if (tsPacket->payload_start) {
												// Send payload to receiver
												// TODO:

												// Restart payload collection
												pidPayload[tsPacket->pid]->reset();
											}
											assert(tableSize <= 188);
											pidPayload[tsPacket->pid]->append(bufferReadPtr + tableSize, 188 - tableSize, tsPacket->continuity_counter);
										} else {
											printf("no payload (pid=%u)\n", tsPacket->pid);
										}

										if (tsPacket->adaptation_field) {
											if (tsPacket->adaption->PCR) {
					//							printf("pid: %u, PCR\n", tsPacket->pid);
											}
										}
									}
								}
						}

					}

					bufferReadPtr += 188;
				}

				// Check if buffer contains remaining data
				if (bufferReadPtr != bufferWritePtr) {
					memmove(buffer.data(), bufferReadPtr, bufferWritePtr - bufferReadPtr);
					bufferWritePtr = buffer.data() + (bufferWritePtr - bufferReadPtr);
				} else {
					bufferWritePtr = buffer.data();
				}
				bufferReadPtr = buffer.data();
			}
			thiz->m_ThreadFinished = true;

			size_t nPIDs = 0;
			for (size_t i=0; i < pidPayload.size(); i++) {
				if (pidPayload[i]) {
					printf("  pid=%u\n", (unsigned)i);
					nPIDs++;
				}
			}
			printf("pidPayload.size() = %u\n", (unsigned)pidPayload.size());
			printf("pids.size=%u\n", (unsigned)nPIDs);

			for (size_t i=0; i<pidPayload.size(); i++) {
				if (pidPayload[i]) {
					delete(pidPayload[i]);
					pidPayload[i] = nullptr;
				}
			}
			free(tsPacket);
		}

		void TSSource::setProgramProvider(ProgramProvider pp) {
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			m_ProgramProvider = pp;
		}

	}
}