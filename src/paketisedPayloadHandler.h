#pragma once
#include <types.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * Handler of paketised payload
		 */
		class PaketisedPayloadHandler {
		public:
			/**
			 * Create PID payload handler class
			 * @param[in] stream Stream which will receive concatenated payload upon reception
			 */
			PaketisedPayloadHandler(StreamRef stream);
			virtual ~PaketisedPayloadHandler() = default;

			/**
			 * Reset payload writer to start new payload. Once reset, collected payload is being
			 * supplied to stream then resets.
			 */
			void reset();

			/**
			 * Append data to payload
			 */
			void append(const uint8_t* src, size_t size, size_t ctr, bool incrementCtr);

			/**
			 * Retrieve data size
			 */
			size_t dataSize() const;

			/**
			 * Retrieve data
			 */
			const uint8_t* data() const;

		protected:
			/**
			 * Default implementation supplies stream with data as-is:
			 *   stream->supplyData(data(), dataSize());
			 *
			 * @param[in] stream Stream to supply with data (nonnull)
			 */
			virtual void supplyStream(const StreamRef& stream);

		private:

			size_t m_Counter = 0;
			bool   m_Error = true;
			bool   m_PayloadStart = false;
			StreamRef m_Stream;
			std::vector<uint8_t> m_Data;
			size_t m_DataSize = 0;
		};

	}
}