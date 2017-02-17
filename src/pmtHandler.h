#pragma once
#include <stream.h>
#include <libdvbv5/pmt.h>

namespace fp {
	namespace cap {

		class PMTHandler : public fp::cap::Stream {
		public:
			typedef std::function<StreamRef(uint32_t, Stream::Type, bool)> CreateStream;
			typedef std::function<void(ProgramRef)> SpawnProgram;

			/**
			 * Create PAT handler
			 * @param[in] pid PID of packet where this PAT is located
			 * @param[in] serviceID Service ID this PAT relates to
			 * @param[in] receivedStreams Callback which fires once PMT received list of streams
			 */
			PMTHandler(uint32_t serviceId, CreateStream cs, SpawnProgram sp);
			~PMTHandler() override;
			void supplyData(const uint8_t* data, size_t size, Metadata*) override;

			/**
			 * Get associated serviceID
			 */
			uint32_t service() const;

		private:
			void clear();
			dvb_table_pmt* m_PMT = nullptr;
			uint32_t m_ServiceId;
			ProgramRef m_Program;
			CreateStream m_CreateStream;
			SpawnProgram m_SpawnProgram;
		};

	}
}