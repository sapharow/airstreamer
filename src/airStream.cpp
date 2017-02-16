#include <airStream.h>
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/dvb-demux.h>
#include <libdvbv5/mpeg_pes.h>
#include <libdvbv5/mpeg_ts.h>
#include <libdvbv5/pat.h>
#include <libdvbv5/sdt.h>
#include <libdvbv5/eit.h>
#include <libdvbv5/header.h>
#include <exception>
#include <vector>
#include <set>
#include <map>
#include <unistd.h>
#include <cassert>
#include <algorithm>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0
#define VERBOSE_LEVEL 2
#define GRAB_FREQ     642000000
#define BANDWIDTH     8000000

#define GUARD_INTERVAL_1_32   0
#define GUARD_INTERVAL_1_16   1
#define GUARD_INTERVAL_1_8    2
#define GUARD_INTERVAL_1_4    3
#define GUARD_INTERVAL_AUTO   4
#define GUARD_INTERVAL_1_128  5
#define GUARD_INTERVAL_19_128 6
#define GUARD_INTERVAL_PN420  8
#define GUARD_INTERVAL_PN595  9
#define GUARD_INTERVAL_PN945  10

namespace fp {

	struct AirStream::Private {
		Private()
		: dvbFE(nullptr)
		, dvbDMX(-1)
		{
			dvbFE = dvb_fe_open(DVB_ADAPTER, DVB_FRONTEND, VERBOSE_LEVEL, 0);
			if (!dvbFE) {
				throw std::runtime_error("DVB adapter can not be opened");
			}

			dvbDMX = dvb_dmx_open(DVB_ADAPTER, DVB_DEMUX);
			if (dvbDMX == -1) {
				dvb_fe_close(dvbFE);
				throw std::runtime_error("DVB demuxer can not be opened");
			}

			buffer.resize(1024);

			// Setup DVB
			dvb_set_compat_delivery_system(dvbFE, SYS_DVBT);
			dvb_fe_store_parm(dvbFE, DTV_FREQUENCY,    GRAB_FREQ);
			dvb_fe_store_parm(dvbFE, DTV_BANDWIDTH_HZ, BANDWIDTH);
			dvb_fe_store_parm(dvbFE, DTV_GUARD_INTERVAL, GUARD_INTERVAL_1_8);
			dvb_fe_store_parm(dvbFE, DTV_TRANSMISSION_MODE, TRANSMISSION_MODE_8K);
			dvb_fe_set_parms(dvbFE);
			// TODO: FEC_5/6

			// Get all available PIDs
			dvb_set_section_filter(dvbDMX, 0x2000, 
				0, nullptr, nullptr, nullptr, DMX_CHECK_CRC);

			dvb_set_pesfilter(dvbDMX, 0x2000, DMX_PES_VIDEO, DMX_OUT_TSDEMUX_TAP, buffer.size());

			outF = fopen("test.ts", "wb");

			uint8_t* bufferPtr = buffer.data();

			ts_meta = (dvb_mpeg_ts*)malloc(sizeof(dvb_mpeg_ts) + sizeof(dvb_mpeg_ts_adaption));

			bytesCount = 0;
			while (bytesCount < 10000000) {

				// Try to fill buffer with 1+ TS packets
				size_t nPackets = 0;
				while (1) {
					size_t nMaxBytesToRead = buffer.data() + buffer.size() - bufferPtr;
					const ssize_t nRead = read(dvbDMX, bufferPtr, nMaxBytesToRead);
					if (nRead > 0) {
						assert((nRead % 188) == 0);
						nPackets += nRead / 188;
						bufferPtr += nRead;
						break;
					}
				}

				// Buffer has >=188 bytes
				// Consume it
				bufferPtr = buffer.data();
				while (nPackets--) {
					uint32_t first = *(uint32_t*)bufferPtr;
					if ((first & 0xff) == 0x47) {
						processTs(bufferPtr);
					}
					bufferPtr += 188;
				}

//				printf("bytesCount=%u\n", (unsigned)bytesCount);
			}
			fclose(outF);
		}

		~Private() {
			dvb_fe_close(dvbFE);
			dvb_dmx_close(dvbDMX);
		}

		void processTs(uint8_t* data) {
			ssize_t tableSize;
			ssize_t tsSize = dvb_mpeg_ts_init (dvbFE, data, 188, (uint8_t*)ts_meta, &tableSize);
			if (tsSize) {
				if ((ts_meta->scrambling == 0) /*&& (ts_meta->tei == 0)*/) {
//					if (ts_meta->pid == 0xcc)
					if (ts_meta->payload)
						dvb_mpeg_ts_print(dvbFE, ts_meta);

					switch (ts_meta->pid) {
						case 0x0000: {
							// Found PAT
							dvb_table_pat* patTable = nullptr;
							ssize_t tableSize = dvb_table_pat_init(dvbFE, data + tsSize, 188 - tsSize, &patTable);
							if (tableSize == -1) {
								printf("Error in PAT\n");
							} else {
								pat.clear();
								dvb_pat_program_foreach(i, patTable) {
									const uint32_t pid = i->pid;
									const uint32_t sid = i->service_id;
									pat.emplace(pid, sid);
								}
								dvb_table_pat_free(patTable);
							}
							break;
						}
						case 0x0001:
							// CAT
							// TODO
							break;
						case 0x0002:
							// TSDT
							break;
						case 0x0003:
							// IPMP
							break;
						case 0x0010:
							// NIT, ST
							break;
						case 0x0011: {
							// TODO:
							/*
							// SDT/BAT
							dvb_table_header* table = (dvb_table_header*)(data + tsSize);
							dvb_table_header_print(dvbFE, table);
							fwrite(data, 188, 1, outF);
							fclose(outF);
							dvb_mpeg_ts_print(dvbFE, ts_meta);
							for (size_t i=0; i<188; i++) {
								printf("0x%02x ", data[i]);
							}
							printf("\n");
							dvb_table_sdt *sdtTable = nullptr;
							ssize_t tableSize = dvb_table_sdt_init(dvbFE, data + tsSize + 1, 188 - tsSize - 1, &sdtTable);
							if (tableSize == -1) {
								printf("Error in SDT\n");
							} else {
								dvb_table_sdt_print(dvbFE, sdtTable);
								dvb_table_sdt_free(sdtTable);
							}
							exit(1);
							*/
							break;
						}
							// TODO
							break;
						case 0x0012: {
							// EIT, ST, CIT
							/*
							dvb_mpeg_ts_print(dvbFE, ts_meta);
							for (size_t i=0; i<188; i++) {
								printf("0x%02x ", data[i]);
							}
							printf("\n");
							*/
/*
							dvb_table_eit* table = nullptr;
							ssize_t tableSize = dvb_table_eit_init(dvbFE, data + tsSize + 1, 188 - tsSize - 1, &table);
							if (tableSize == -1) {
								printf("Error in EIT table\n");
							} else {
//								dvb_table_eit_print(dvbFE, table);
								dvb_table_eit_free(table);
							}
							*/
							break;
						}
						case 0x0013:
							// RST, ST
							break;
						case 0x0014:
							// TDT, TOT, ST
							break;
						case 0x0015:
							// Network synchronisation
							break;
						case 0x0016:
							// RNT
							break;
						case 0x0017:
						case 0x0018:
						case 0x0019:
						case 0x001A:
						case 0x001B:
							// Reserved
							break;
						case 0x001C:
							// Inband signalling
							break;
						case 0x001D:
							// Measurement
							break;
						case 0x001E:
							// DIT
							break;
						case 0x001F:
							// SIT
							break;
						case 0x1FFF:
							// Null packet
							break;
						default:
							break;

					}
//					dvb_mpeg_ts_print(dvbFE, ts_meta);
					bytesCount += 188;

/*
					// Skip to index
					for (size_t i=0; i<pid_idx; i++) {
						printf("      ");
					}

					printf("%5d", ts_meta->pid);
					printf("\n");
*/
//					dvb_mpeg_ts_print(dvbFE, &ts_meta);
//					printf("tableSize=%ld, tsSize=%ld\n", tableSize, tsSize);
				}
			}
		}

		dvb_v5_fe_parms* dvbFE;
		int dvbDMX;
		std::vector<uint8_t> buffer;
		size_t bytesCount;
		FILE* outF = nullptr;

		dvb_mpeg_ts* ts_meta = nullptr;

		// Association PID > ServiceID
		std::map<uint32_t, uint32_t> pat;
	};

	AirStream::AirStream() 
	: m_Private(std::make_shared<Private>())
	{

	}

}