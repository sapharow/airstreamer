#pragma once
#include <types.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * Stream is class which describes elementary stream
		 * associated with particular program
		 */
		class Stream {
		public:
			/**
			 * Stream metadata which optionally can be supplied 
			 */
			struct Metadata {
				uint64_t* dts;	// Decode timestamp
				uint64_t* pts;	// Presentation timestamp
			};

			enum class Type {
				Video_H261,
				Video_H262,
				Audio_11172_2,
				Audio_13818_2,
				Audio_AC3,
				Other
			};

			/**
			 * Create Stream class
			 * @param[in] id Program ID
			 * @param[in] type Stream type
			 * @param[in] sync If set to true then synchronisation is performed by this stream
			 * @param[in] lang Language code
			 */
			Stream(uint32_t id, Type type, bool sync, uint32_t lang);
			virtual ~Stream() = default;

			/**
			 * Retrieve stream ID
			 */
			uint32_t id() const;

			/**
			 * Retrieve stream type
			 */
			Type type() const;

			/**
			 * Return true if synchronisation is performed by this stream
			 */
			bool sync() const;

			/**
			 * Stream language as 3-bytes code
			 * 0 means no language assigned
			 */
			uint32_t lang() const;

			/**
			 * Supply stream with data
			 * @param[in] data Data to supply
			 * @param[in] size Size of data
			 */
			virtual void supplyData(const uint8_t* data, size_t size, Metadata* = nullptr) = 0;

		private:
			uint32_t m_ID;
			Type m_Type;
			bool m_Sync;
			uint32_t m_Lang;
		};
	}
}
