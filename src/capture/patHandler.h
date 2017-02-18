#pragma once
#include <stream.h>
#include <libdvbv5/pat.h>
#include <unordered_map>

namespace fp {
	namespace cap {

		class PATHandler : public Stream {
		public:
			typedef std::unordered_map<uint32_t, uint32_t> Services;
			typedef std::function<void(const Services&)> ReceivedServices;

			/**
			 * Create PAT handler
			 * @param[in] receivedServices Callback which fires once PAT received list of services
			 */
			PATHandler(ReceivedServices receivedServices);
			~PATHandler() override;
			void supplyFrame(const uint8_t* data, size_t size, Metadata*) override;

			/**
			 * Get PMT_PID -> ServiceID
			 */
			const std::unordered_map<uint32_t, uint32_t>& services() const;

		private:
			void clear();
			dvb_table_pat* m_PAT = nullptr;

			// PMT_PID -> ServiceID
			std::unordered_map<uint32_t, uint32_t> m_Services;
			ReceivedServices m_ReceivedServices;
		};

	}
}