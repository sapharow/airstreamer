#pragma once
#include <stream.h>
#include <libdvbv5/pmt.h>

namespace fp {
	namespace cap {

		class PMTHandler : public Stream {
		public:
			typedef std::function<StreamRef(StreamMeta*)> CreateStream;

			/**
			 * When program is spawned this handler is called. If true returned, then program is retained
			 * otherwise it is discarded
			 * @param[in] program Newly created program (non-null)
			 * @return true if program shall be retained, false otherwise
			 */
			typedef std::function<bool(ProgramRef program)> SpawnProgram;

			/**
			 * Create PAT handler
			 * @param[in] pid PID of packet where this PAT is located
			 * @param[in] serviceID Service ID this PAT relates to
			 * @param[in] receivedStreams Callback which fires once PMT received list of streams
			 */
			PMTHandler(uint32_t serviceId, CreateStream cs, SpawnProgram sp);
			~PMTHandler() override;
			void supplyFrame(const uint8_t* data, size_t size, Metadata*) override;

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