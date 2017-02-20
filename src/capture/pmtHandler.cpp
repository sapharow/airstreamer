#include "pmtHandler.h"
#include <capture/tsSource.h>
#include <capture/program.h>
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/descriptors.h>
#include <exception>

namespace fp {
	namespace cap {

		PMTHandler::PMTHandler(uint32_t serviceId, CreateStream cs, SpawnProgram sp)
		: Stream(nullptr)
		, m_ServiceId(serviceId)
		, m_CreateStream(cs)
		, m_SpawnProgram(sp)
		{ }

		PMTHandler::~PMTHandler() {
			clear();
		}

		void PMTHandler::supplyFrame(const uint8_t* data, size_t size, Metadata*) {
			if (!m_PMT) {
				size_t offset = (data[0] == 0) ? 1 : 0;
				ssize_t pmtSize = dvb_table_pmt_init(dvb_fe_dummy(), data+offset, size-offset, &m_PMT);
				if (pmtSize != -1) {
					std::vector<StreamRef> streams;
					dvb_pmt_stream_foreach(stream, m_PMT) {
						StreamType type = StreamType::Other;
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
									type = StreamType::Audio_AC3;
									break;
								case iso639_language_descriptor:
									if (desc->length >= 3) {
										memcpy((uint8_t*)&language + 1, desc->data, 3);
									}
									break;
								case video_stream_descriptor:
									type = StreamType::Video_H262;
									break;
								default: ;
							}
						}


						switch (stream->type) {
							case stream_video:
								type = StreamType::Video_H261;
								break;
							case stream_video_h262:
								type = StreamType::Video_H262;
								break;
							case stream_audio:
								type = StreamType::Audio_11172_2;
								break;
							case stream_audio_13818_3:
								type = StreamType::Audio_13818_2;
								break;
							default:
								if (type == StreamType::Other) {
									printf("Other 0x%04x\n", stream->type);
									type = StreamType::Other;
									break;
								}
						}

						if (m_CreateStream) {
							std::shared_ptr<StreamMeta> meta;
							switch (type) {
								case StreamType::Video_H261:
								case StreamType::Video_H262:
								case StreamType::Video_H264: {
									auto videoMeta = std::make_shared<VideoStreamMeta>();
									videoMeta->width = 640; // todo
									videoMeta->height = 480; // todo
									meta = videoMeta;
									break;
								}
								case StreamType::Audio_11172_2:
								case StreamType::Audio_13818_2:
								case StreamType::Audio_AC3: {
									auto audioMeta = std::make_shared<AudioStreamMeta>();
									audioMeta->lang = language;
									meta = audioMeta;
									break;
								}
								default:
									meta = std::make_shared<StreamMeta>();
									break;
							}

							meta->id = stream->elementary_pid;
							meta->type = type;
							meta->sync = m_PMT->pcr_pid == stream->elementary_pid;
							auto localStream = m_CreateStream(meta.get());
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