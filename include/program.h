#pragma once
#include <types.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * Program is class which handles all elementary streams which
		 * associated with particular service
		 */
		class Program {
		public:
			typedef std::vector<StreamRef> Streams;

			/**
			 * Create program with streams
			 * @param[in] id Program ID
			 * @param[in] streams Attached streams
			 */
			Program(uint32_t id, const Streams& streams);

			/**
			 * Retrieve list of attached streams
			 */
			const Streams& streams() const;

			/**
			 * Retrieve program ID
			 */
			uint32_t id() const;

		private:
			uint32_t m_ID;
			Streams m_Streams;

		};

	}
}