#pragma once
#include <functional>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		class Program {
		public:
			/**
			 * Create program with given PID
			 */
			Program(uint32_t pid);
			virtual ~Program() = default;

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
			virtual void supplyStreamPayload(const uint8_t* data, size_t size) = 0;
		private:
			uint32_t m_PID;
		};

	}

}