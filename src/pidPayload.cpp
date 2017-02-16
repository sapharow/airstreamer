#include "pidPayload.h"
#include <libdvbv5/dvb-fe.h>
#include <program.h>
#include "libdvbv5-patch/mpeg_pes.h"

/**
 * If buffer needs to be increased to particular size then it increases to 
 * this size plus margin to reduce memory re-allocs
 */
#define BUFFER_INCREASE_MARGIN	1024

namespace fp {
	namespace cap {

		PIDPayload::PIDPayload(ProgramRef _program, uint32_t _pid, bool splitAcrossPackets)
		: m_PID(_pid)
		, m_Program(_program)
		, m_SplitAcrossPackets(splitAcrossPackets)
		{
			m_PESTable = (dvb_mpeg_pes*)malloc(sizeof(dvb_mpeg_pes) + sizeof(dvb_mpeg_pes_optional));
		}

		PIDPayload::~PIDPayload() {
			free(m_PESTable);
		}

		void PIDPayload::reset() {
			if (m_DataSize >= 3) {
				if ((m_Data[0] == 0) && (m_Data[1] == 0) && (m_Data[2] == 1)) {
					// De-packetise
					ssize_t pesSize = dvb_mpeg_pes_init_patched(dvb_fe_dummy(), m_Data.data(), m_DataSize, (uint8_t*)m_PESTable);
//					printf("Optional fields length = %u\n", m_PESTable->optional->length);
//					printf("stream_id = %u\n", m_PESTable->stream_id);
					size_t nBytesToSkip = pesSize;//sizeof(dvb_mpeg_pes) + m_PESTable->length;
					size_t nBytesToSupply = m_DataSize - nBytesToSkip;
					if (nBytesToSupply && m_Program && m_SplitAcrossPackets) {
						m_Program->supplyStreamPayload(m_Data.data() + nBytesToSkip, nBytesToSupply);
					}

					if (m_PESTable->optional->data_alignment_indicator) {
						if (m_Program) {
						}
					} else {
						printf("Data is NOT aligned. TODO!!!\n");
					}
				} else {
					if (m_SplitAcrossPackets) {
						m_Program->supplyStreamPayload(m_Data.data(), m_DataSize);
					}
				}

			}
			m_DataSize = 0;
			m_PayloadStart = true;
			m_Error = false;
		}

		void PIDPayload::append(uint8_t* src, size_t size, size_t ctr) {
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
			if (!m_SplitAcrossPackets) {
				m_Program->supplyStreamPayload(m_Data.data(), m_DataSize);
			}
		}

		size_t PIDPayload::dataSize() const {
			return m_DataSize;
		}

		uint8_t* PIDPayload::data() {
			return m_Data.data();
		}
	}
}
