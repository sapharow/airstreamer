#include "pmtHandler.h"
#include <capture/tsSource.h>
#include <capture/program.h>
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/descriptors.h>
#include <exception>

namespace fp {
	namespace cap {

		PMTHandler::PMTHandler(uint32_t serviceId, CreateStream cs, SpawnProgram sp)
		: fp::cap::Stream(0, Type::Other, false, 0)
		, m_ServiceId(serviceId)
		, m_CreateStream(cs)
		, m_SpawnProgram(sp)
		{ }

		PMTHandler::~PMTHandler() {
			clear();
		}

		void PMTHandler::supplyData(const uint8_t* data, size_t size, Metadata*) {
			if (!m_PMT) {
				size_t offset = (data[0] == 0) ? 1 : 0;
				ssize_t pmtSize = dvb_table_pmt_init(dvb_fe_dummy(), data+offset, size-offset, &m_PMT);
				if (pmtSize != -1) {
					std::vector<StreamRef> streams;
					dvb_pmt_stream_foreach(stream, m_PMT) {
						Type type = Type::Other;
						uint32_t fourCC = 0;
						uint32_t language = 0;

						dvb_desc_foreach(desc, stream) {
							switch (desc->type) {
								case registration_descriptor:
									// Format identifier
									if (desc->length >= 4) {
										void* data = desc->data;
										fourCC = *(reinterpret_cast<uint32_t*>(data));
									}
								break;
								case atsc_ac3_audio_descriptor:
									// Audio in AC-3 format
									type = Type::Audio_AC3;
									break;
								case iso639_language_descriptor:
									if (desc->length >= 3) {
										memcpy((uint8_t*)&language + 1, desc->data, 3);
									}
									break;
								case video_stream_descriptor:
									type = Type::Video_H262;
									break;
								default: ;
							}
						}


						switch (stream->type) {
							case stream_video:
								type = Type::Video_H261;
								break;
							case stream_video_h262:
								type = Type::Video_H262;
								break;
							case stream_audio:
								type = Type::Audio_11172_2;
								break;
							case stream_audio_13818_3:
								type = Type::Audio_13818_2;
								break;
							default:
								if (type == Type::Other) {
									printf("Other 0x%04x\n", stream->type);
									type = Type::Other;
									break;
								}
						}

						if (m_CreateStream) {
							auto localStream = m_CreateStream(stream->elementary_pid, type, m_PMT->pcr_pid == stream->elementary_pid, language);
							if (localStream) {
								streams.push_back(localStream);
							}
						}
					}

					if (!streams.empty()) {
						auto program = std::make_shared<Program>((uint32_t)m_PMT->header.id, streams);
						if (m_SpawnProgram) {
							if (m_SpawnProgram(program)) {
								m_Program = program;
							}
						}
					}
				}
			}
		}

		uint32_t PMTHandler::service() const {
			return m_ServiceId;
		}

		void PMTHandler::clear() {
			if (m_PMT) {
				dvb_table_pmt_free(m_PMT);
				m_PMT = nullptr;
			}
		}

	}
}