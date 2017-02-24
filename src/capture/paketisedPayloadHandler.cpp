#include "paketisedPayloadHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <stream.h>
#include "libdvbv5-patch/mpeg_pes.h"

/**
 * If buffer needs to be increased to particular size then it increases to 
 * this size plus margin to reduce memory re-allocs
 */
#define BUFFER_INCREASE_MARGIN	1024

namespace fp {
	namespace cap {

		PaketisedPayloadHandler::PaketisedPayloadHandler(StreamRef stream)
		: m_Stream(stream)
		{
		}

		void PaketisedPayloadHandler::supplyStream(const StreamRef& stream) {
			if (!m_DataSize) {
				return;
			}
			stream->supplyFrame(m_Data.data(), m_DataSize);
		}

		void PaketisedPayloadHandler::reset() {
			// Determine if content is PES
			if (m_Stream) {
				try {
					supplyStream(m_Stream);
				} catch (std::exception& e) {
					// Stream won't accept data
					printf("Stream won't accept data (%s)\n", e.what());
				}
			}

			m_DataSize = 0;
			m_PayloadStart = true;
			m_Error = false;
		}

		void PaketisedPayloadHandler::append(const uint8_t* src, size_t size, size_t ctr, bool incrementCtr) {
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
					if (incrementCtr) {
						m_Counter++;
					}
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

		size_t PaketisedPayloadHandler::dataSize() const {
			return m_DataSize;
		}

		const uint8_t* PaketisedPayloadHandler::data() const {
			return m_Data.data();
		}
	}
}
