#include "programPayloadHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <programReceiver.h>
#include "libdvbv5-patch/mpeg_pes.h"

/**
 * If buffer needs to be increased to particular size then it increases to 
 * this size plus margin to reduce memory re-allocs
 */
#define BUFFER_INCREASE_MARGIN	1024

namespace fp {
	namespace cap {

		ProgramPayloadHandler::ProgramPayloadHandler(ProgramReceiverRef _program, uint32_t _pid)
		: m_PID(_pid)
		, m_ProgramReceiver(_program)
		{
			m_PESTable = (dvb_mpeg_pes*)malloc(sizeof(dvb_mpeg_pes) + sizeof(dvb_mpeg_pes_optional));
		}

		ProgramPayloadHandler::~ProgramPayloadHandler() {
			free(m_PESTable);
		}

		void ProgramPayloadHandler::handlePayload() {
			if (m_DataSize >= 3) {
				if ((m_Data[0] == 0) && (m_Data[1] == 0) && (m_Data[2] == 1)) {
					// Content is PES
					const ssize_t pesSize = dvb_mpeg_pes_init_patched(dvb_fe_dummy(), m_Data.data(), m_DataSize, (uint8_t*)m_PESTable);
					if (pesSize != -1) {
						// PES table is correct
						size_t nBytesToSupply = m_DataSize - pesSize;
						if (nBytesToSupply && m_ProgramReceiver) {
							ProgramReceiver::Stream stream;
							switch (m_PESTable->stream_id) {
								case DVB_MPEG_PES_AUDIO:
									stream = ProgramReceiver::Stream::Audio;
									break;
								case DVB_MPEG_PES_VIDEO:
									stream = ProgramReceiver::Stream::Video;
									break;
								case DVB_MPEG_STREAM_MAP:
									stream = ProgramReceiver::Stream::Video;
									break;
								case DVB_MPEG_STREAM_PADDING:
									return;
									break;
								case DVB_MPEG_STREAM_PRIVATE_2:
									stream = ProgramReceiver::Stream::Private2;
									break;
								case DVB_MPEG_STREAM_ECM:
									stream = ProgramReceiver::Stream::ECM;
									break;
								case DVB_MPEG_STREAM_EMM:
									stream = ProgramReceiver::Stream::EMM;
									break;
								case DVB_MPEG_STREAM_DIRECTORY:
									stream = ProgramReceiver::Stream::Directory;
									break;
								case DVB_MPEG_STREAM_DSMCC:
									stream = ProgramReceiver::Stream::DSMCC;
									break;
								case DVB_MPEG_STREAM_H222E:
									stream = ProgramReceiver::Stream::H222E;
									break;
								default:
									stream = ProgramReceiver::Stream::Unknown;
							}
							bool bPTS = (m_PESTable->optional->PTS_DTS & 2);
							bool bDTS = (m_PESTable->optional->PTS_DTS & 1);

							if (m_PESTable->optional->data_alignment_indicator) {
								if (m_ProgramReceiver) {
								}
							} else {
								printf("Data is NOT aligned. TODO!!!\n");
							}
							m_ProgramReceiver->supplyES(stream, 
							                            m_PESTable->stream_id, 
							                            (m_PESTable->optional->PTS_DTS & 0x2) ? &m_PESTable->optional->pts : nullptr,
							                            (m_PESTable->optional->PTS_DTS & 0x1) ? &m_PESTable->optional->dts : nullptr,
							                            m_Data.data() + pesSize, 
							                            nBytesToSupply);
						}
					}
//					printf("Optional fields length = %u\n", m_PESTable->optional->length);
//					printf("stream_id = %u\n", m_PESTable->stream_id);
					return;
				}
			}

			// Content is not PES
			m_ProgramReceiver->supplyPayload(m_Data.data(), m_DataSize);
		}

		void ProgramPayloadHandler::reset() {
			// Determine if content is PES
			handlePayload();

			m_DataSize = 0;
			m_PayloadStart = true;
			m_Error = false;
		}

		void ProgramPayloadHandler::append(const uint8_t* src, size_t size, size_t ctr) {
			if (m_PayloadStart) {
				m_PayloadStart = false;
				m_Counter = ctr;
			} else {
				// Check counter
				if (m_Error) {
					return;
				} else {
					if (m_Counter == ctr) {
						// Duplicate - ignore
						return;
					}
					m_Counter++;
					m_Counter &= 0xf;
					if (m_Counter != ctr) {
						// Counter mismatch
						printf("Counter mismatch. Expected %u, given %u\n", (unsigned)m_Counter, (unsigned)ctr);
						m_Error = true;
						return;
					}
				}
			}

			if (m_Data.size() < (m_DataSize + size)) {
				m_Data.resize(m_DataSize + size + BUFFER_INCREASE_MARGIN);
			}
			memcpy(m_Data.data() + m_DataSize, src, size);
			m_DataSize += size;
		}

		size_t ProgramPayloadHandler::dataSize() const {
			return m_DataSize;
		}

		uint8_t* ProgramPayloadHandler::data() {
			return m_Data.data();
		}
	}
}
