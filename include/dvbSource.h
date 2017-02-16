#pragma once
#include <tsSource.h>
#include <libdvbv5/dvb-fe.h>

namespace fp {
	namespace cap {

		/**
		 * DVB device to capture from
		 */
		class DVBSource : public TSSource {
		public:
			/**
			 * Create DVB source
			 * @param[in] adapter Adapter device number
			 * @param[in] demuxer Demuxer device number
			 * @param[in] frontend Frontend device number
			 */
			DVBSource(uint32_t adapter, uint32_t frontend, uint32_t demuxer);
			~DVBSource() override;

		protected:
			size_t readDataInto(uint8_t* buffer, size_t size) override;
			void doStart() override;
			void doStop() override;

		private:
			dvb_v5_fe_parms* m_Frontend = nullptr;
			int m_DemuxIO = -1;
			uint32_t m_AdapterDevice = -1;
			uint32_t m_DemuxerDevice = -1;
		};


	}
}