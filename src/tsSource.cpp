#include <tsSource.h>
#include "pidPayload.h"
#include <vector>
#include <set>
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/mpeg_ts.h>
#include <libdvbv5/pat.h>
#include "libdvbv5-patch/mpeg_pes.h"
#include <libdvbv5/descriptors.h>
#include <memory.h>
#include <program.h>
#include <cassert>

/**
 * Size of buffer (bytes) to read from source implementation
 */
#define READ_BUFFER 8192

namespace fp {
	namespace cap {

		class PATHandler : public fp::cap::Program {
		public:
			PATHandler(uint32_t pid)
			: fp::cap::Program(pid)
			{ }

			~PATHandler() override {
				clear();
			}

			void supplyStreamPayload(const uint8_t* data, size_t size) override {
				// PAT table
				if (!m_PAT) {
					ssize_t patSize = dvb_table_pat_init(dvb_fe_dummy(), data, size, &m_PAT);
					if (patSize != -1) {
						dvb_pat_program_foreach(program, m_PAT) {
							if (program->service_id == 0) {
								printf("Network ID = 0x%02x\n", program->pid);
							} else {
								printf("Program map PID 0x%02x > Program number 0x%02x\n", program->pid, program->service_id);
							}
						}
					}
				}
			}
		private:
			void clear() {
				if (m_PAT) {
					dvb_table_pat_free(m_PAT);
					m_PAT = nullptr;
				}
			}
			dvb_table_pat* m_PAT = nullptr;
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
			std::vector<PIDPayload*> pidPayload(8192);

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
				uint8_t* bufferReadPtr;
				for (bufferReadPtr = buffer.data(); bufferWritePtr - bufferReadPtr >= 188; bufferReadPtr += 188) {
					// Buffer contains another TS packet to analyse
					ssize_t tableSize;
					if (bufferReadPtr[0] == DVB_MPEG_TS) {
						tableSize = dvb_mpeg_ts_init(dvb_fe_dummy(), bufferReadPtr, bufferWritePtr - bufferReadPtr, (uint8_t*)tsPacket, &tableSize);
						if (tableSize <= 188) {

							if (!pidPayload[tsPacket->pid]) {
								ProgramRef program;
								// Allocate payload info
								std::lock_guard<std::recursive_mutex> lock(thiz->m_Mutex);

								// Create custom programs handler
								switch (tsPacket->pid) {
									case 0x0000:
										// Program association table
										program = std::make_shared<PATHandler>((uint32_t)tsPacket->pid);
										break;

									case 0x1fff:
										// Do nothing for null packet
										continue;
									break;

									case 0x1ffb:
										// Used by DigiCipher 2/ATSC MGT metadata
										continue;
									break;
								}
								if (!program) {
									auto pp = thiz->m_ProgramProvider;
									if (pp) {
										program = pp(tsPacket->pid);
									}
								}
								pidPayload[tsPacket->pid] = new PIDPayload(program, tsPacket->pid);
							}

							if (tsPacket->payload) {
								if (tsPacket->payload_start) {
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
//									printf("pid: %u, PCR\n", tsPacket->pid);
								}
							}
						}

					}
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

			printf("pidPayload.size() = %u\n", (unsigned)pidPayload.size());

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