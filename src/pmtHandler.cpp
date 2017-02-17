#include "pmtHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <program.h>

namespace fp {
	namespace cap {

		PMTHandler::PMTHandler(uint32_t serviceId, StreamProvider sp, ReceivedProgram receivedProgram)
		: fp::cap::Stream(0, Type::Other, false)
		, m_ServiceId(serviceId)
		, m_SP(sp)
		, m_ReceivedProgram(receivedProgram)
		{ }

		PMTHandler::~PMTHandler() {
			clear();
		}

		void PMTHandler::supplyData(const uint8_t* data, size_t size) {
			if (!m_PMT) {
				size_t offset = (data[0] == 0) ? 1 : 0;
				ssize_t pmtSize = dvb_table_pmt_init(dvb_fe_dummy(), data+offset, size-offset, &m_PMT);
				if (pmtSize != -1) {
					if (m_SP) {
						std::vector<StreamRef> streams;
						dvb_pmt_stream_foreach(stream, m_PMT) {
							Type type = Type::Other;
							switch (stream->type) {
								case 0x01:
									type = Type::Video_11172_2;
									break;
								case 0x02:
									type = Type::Video_13818_2;
									break;
								case 0x03:
									type = Type::Audio_11172_2;
									break;
								case 0x04:
									type = Type::Audio_13818_2;
									break;
								default:
									type = Type::Other;
									break;
							}
							auto localStream = m_SP(stream->elementary_pid, type, m_PMT->pcr_pid == stream->elementary_pid);
							if (localStream) {
								streams.push_back(localStream);
							}
						}

						m_Program = std::make_shared<Program>((uint32_t)m_PMT->header.id, streams);
						if (m_ReceivedProgram) {
							m_ReceivedProgram(m_Program);
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