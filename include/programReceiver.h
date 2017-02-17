#pragma once
#include <functional>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * ProgramReceiver is handler for program payload gathered from multiple transport packets
		 */
		class ProgramReceiver {
		public:
			enum class Stream {
				Audio,
				Video,
				Map,
				Private2,
				ECM,
				EMM,
				Directory,
				DSMCC,
				H222E
			};
			/**
			 * Create program with given PID
			 */
			ProgramReceiver(uint32_t pid);
			virtual ~ProgramReceiver() = default;

			/**
			 * Retrieve PID
			 */
			uint32_t pid() const;

			/**
			 * Once source receives whole stream payload it supplies it to program
			 * via this function
			 * @param[in] data Payload data
			 * @param[in] size Payload size
			 */
			virtual void supplyPayload(const uint8_t* data, size_t size) = 0;

			/**
			 * If payload is identified as Elementary stream it is being supplied to this function
			 * @param[in] stream Stream type
			 * @param[in] data Payload data
			 * @param[in] size Payload size
			 */
			virtual void supplyES(Stream stream, uint32_t streamId, const uint8_t* data, size_t size) {};

		private:
			uint32_t m_PID;
		};

	}

}