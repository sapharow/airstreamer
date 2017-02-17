#include "patHandler.h"
#include <libdvbv5/dvb-fe.h>

namespace fp {
	namespace cap {

		PATHandler::PATHandler(ReceivedServices receivedServices)
		: fp::cap::Stream(0, Type::Other, false)
		, m_ReceivedServices(receivedServices)
		{ }

		PATHandler::~PATHandler() {
			clear();
		}

		void PATHandler::supplyData(const uint8_t* data, size_t size, Metadata*) {
			// PAT table
			if (!m_PAT) {
				size_t offset = (data[0] == 0) ? 1 : 0;
				ssize_t patSize = dvb_table_pat_init(dvb_fe_dummy(), data+offset, size-offset, &m_PAT);
				if (patSize != -1) {
					printf("Transport stream ID = 0x%02u\n", m_PAT->header.id);
					dvb_pat_program_foreach(program, m_PAT) {
						if (program->service_id != 0) {
							m_Services.emplace((uint32_t)program->pid, (uint32_t)program->service_id);
						}
					}
					if (m_ReceivedServices) {
						m_ReceivedServices(m_Services);
					}
				}
			}
		}

		const std::unordered_map<uint32_t, uint32_t>& PATHandler::services() const {
			return m_Services;
		}

		void PATHandler::clear() {
			if (m_PAT) {
				dvb_table_pat_free(m_PAT);
				m_PAT = nullptr;
			}
		}

	}
}