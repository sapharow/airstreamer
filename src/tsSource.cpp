#include <tsSource.h>
#include "paketisedPayloadHandler.h"
#include "paketisedESPayloadHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/mpeg_ts.h>
#include <libdvbv5/pmt.h>
#include "patHandler.h"
#include "pmtHandler.h"
#include <program.h>
#include <cassert>

/**
 * Size of buffer (bytes) to read from source implementation
 */
#define READ_BUFFER 8192

namespace fp {
	namespace cap {

		TSSource::TSSource() 
		{ }

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
			std::vector<PaketisedPayloadHandler*> pidPayload(8192);

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

								// Create custom programs handler
								if (tsPacket->pid == 0x0000) {
									// Found program association table (PAT)
									auto patHandler = std::make_shared<PATHandler>([&pidPayload, thiz](const PATHandler::Services& services) {
										// Services created, create payload handlers for PMTs
										for (auto service : services) {
											// Create PMT handlers for received PIDs
											if (!pidPayload[service.first]) {
												auto pmtHandler = std::make_shared<PMTHandler>(service.second, 
												                                               [&pidPayload, thiz](uint32_t id, Stream::Type type, bool sync){
												                                               	auto localStream = thiz->createStream(id, type, sync);
												                                               	if (!pidPayload[id]) {
												                                               		pidPayload[id] = new PaketisedESPayloadHandler(localStream);
												                                               	}
												                                               	return localStream;
												                                               },
												                                               [thiz](const ProgramRef& program) {
												                                               	thiz->programSpawned(program);
												                                               });
												pidPayload[service.first] = new PaketisedPayloadHandler(pmtHandler);
											}
										}
									});
									pidPayload[tsPacket->pid] = new PaketisedPayloadHandler(patHandler);
								}
							}

							// Process packets according to provided handlers
							if (pidPayload[tsPacket->pid]) {
								if (tsPacket->payload) {
									if (tsPacket->payload_start) {
										// Restart payload collection
										pidPayload[tsPacket->pid]->reset();
									}
									if (tsPacket->adaptation_field && tsPacket->adaption->PCR) {
//										tsPacket->adaption->data
									}
									pidPayload[tsPacket->pid]->append(bufferReadPtr + tableSize, 
									                                  188 - tableSize, 
									                                  tsPacket->continuity_counter,
									                                  tsPacket->payload & 1
									                                  );
								} else {
									printf("no payload (pid=%u)\n", tsPacket->pid);
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

			for (size_t i=0; i<pidPayload.size(); i++) {
				if (pidPayload[i]) {
					delete(pidPayload[i]);
					pidPayload[i] = nullptr;
				}
			}
			free(tsPacket);
		}

	}
}