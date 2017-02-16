#include <dvbSource.h>
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/dvb-demux.h>
#include <unistd.h>

#define VERBOSE_LEVEL 2
#define BUFFER_SIZE   8192
#define GRAB_FREQ     642000000
#define BANDWIDTH     8000000

namespace fp {
	namespace cap {

		DVBSource::DVBSource(uint32_t adapter, uint32_t frontend, uint32_t demuxer) 
		: m_AdapterDevice(adapter)
		, m_DemuxerDevice(demuxer)
		{
			m_Frontend = dvb_fe_open(m_AdapterDevice, frontend, VERBOSE_LEVEL, 0);
			if (!m_Frontend) {
				throw std::runtime_error("DVB adapter can not be opened");
			}

			// Setup DVB
			dvb_set_compat_delivery_system(m_Frontend, SYS_DVBT);
			dvb_fe_store_parm(m_Frontend, DTV_FREQUENCY,    GRAB_FREQ);
			dvb_fe_store_parm(m_Frontend, DTV_BANDWIDTH_HZ, BANDWIDTH);
			dvb_fe_store_parm(m_Frontend, DTV_GUARD_INTERVAL, GUARD_INTERVAL_1_8);
			dvb_fe_store_parm(m_Frontend, DTV_TRANSMISSION_MODE, TRANSMISSION_MODE_8K);
			dvb_fe_set_parms(m_Frontend);
			// TODO: FEC_5/6
		}

		DVBSource::~DVBSource() {
			stop();
			dvb_fe_close(m_Frontend);
		}

		size_t DVBSource::readDataInto(uint8_t* buffer, size_t size) {
			if (m_DemuxIO != -1) {
				ssize_t res = read(m_DemuxIO, buffer, size);
				if (res == -1) {
					throw std::runtime_error("Error reading from demuxer");
				}
				return res;
			} else {
				throw std::runtime_error("Can not read from non-initialised demuxer");
			}
			return 0;
		}

		void DVBSource::doStart() {
			m_DemuxIO = dvb_dmx_open(m_AdapterDevice, m_DemuxerDevice);
			if (m_DemuxIO != -1) {
				// Get all available PIDs
				//dvb_set_section_filter(m_DemuxIO, 0x2000, 0, nullptr, nullptr, nullptr, DMX_CHECK_CRC);
				//dvb_set_pesfilter(m_DemuxIO, 0x2000, DMX_PES_VIDEO | DMX_PES_AUDIO | DMX_PES_PCR, DMX_OUT_TSDEMUX_TAP, BUFFER_SIZE);
			} else {
				throw std::runtime_error("DVB demuxer can not be opened");
			}
		}
		
		void DVBSource::doStop() {
			if (m_DemuxIO != -1) {
				dvb_dmx_stop(m_DemuxIO);
				dvb_dmx_close(m_DemuxIO);
				m_DemuxIO = -1;
			}
		}

	}
}