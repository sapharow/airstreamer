#include "paketisedESPayloadHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <stream.h>
#include "libdvbv5-patch/mpeg_pes.h"

namespace fp {
	namespace cap {

		PaketisedESPayloadHandler::PaketisedESPayloadHandler(StreamRef stream)
		: PaketisedPayloadHandler(stream)
		{
			m_PESTable = (dvb_mpeg_pes*)malloc(sizeof(dvb_mpeg_pes) + sizeof(dvb_mpeg_pes_optional));
		}

		PaketisedESPayloadHandler::~PaketisedESPayloadHandler() {
			free(m_PESTable);
		}

		void PaketisedESPayloadHandler::supplyStream(const StreamRef& stream) {
			if (dataSize() >= 3) {
				if ((data()[0] == 0) && (data()[1] == 0) && (data()[2] == 1)) {
					// Content is PES
					const ssize_t pesSize = dvb_mpeg_pes_init_patched(dvb_fe_dummy(), data(), dataSize(), (uint8_t*)m_PESTable);
					if (pesSize != -1) {
						// PES table is correct
						size_t nBytesToSupply = dataSize() - pesSize;
						if (nBytesToSupply) {
							/*
							Stream::Type type;
							switch (m_PESTable->stream_id) {
								case DVB_MPEG_PES_AUDIO:
									type = Stream::Type::Audio;
									break;
								case DVB_MPEG_PES_VIDEO:
									type = Stream::Type::Video;
									break;
								case DVB_MPEG_STREAM_MAP:
									type = Stream::Type::Video;
									break;
								case DVB_MPEG_STREAM_PADDING:
									return;
									break;
								case DVB_MPEG_STREAM_PRIVATE_2:
									type = Stream::Type::Private2;
									break;
								case DVB_MPEG_STREAM_ECM:
									type = Stream::Type::ECM;
									break;
								case DVB_MPEG_STREAM_EMM:
									type = Stream::Type::EMM;
									break;
								case DVB_MPEG_STREAM_DIRECTORY:
									type = Stream::Type::Directory;
									break;
								case DVB_MPEG_STREAM_DSMCC:
									type = Stream::Type::DSMCC;
									break;
								case DVB_MPEG_STREAM_H222E:
									type = Stream::Type::H222E;
									break;
								default:
									type = Stream::Type::Unknown;
							}
							*/
							stream->supplyData(
							                     /*
							                            (m_PESTable->optional->PTS_DTS & 0x2) ? &m_PESTable->optional->pts : nullptr,
							                            (m_PESTable->optional->PTS_DTS & 0x1) ? &m_PESTable->optional->dts : nullptr,
							                            */
							                            data() + pesSize, 
							                            nBytesToSupply);
						}
						return;
					}
//					printf("Optional fields length = %u\n", m_PESTable->optional->length);
//					printf("stream_id = %u\n", m_PESTable->stream_id);
				}
			}
			PaketisedPayloadHandler::supplyStream(stream);
		}

	}
}
