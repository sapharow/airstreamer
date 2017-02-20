#pragma once
#include <types.h>

namespace fp {
	enum class StreamType {
		Video_H261,
		Video_H262,
		Video_H264,
		Audio_11172_2,
		Audio_13818_2,
		Audio_AC3,
		Other
	};

	struct StreamMeta {
		uint32_t   id;		// Stream ID
		StreamType type;	// Stream type
		bool       sync;	// Syncronisation by this stream
		virtual ~StreamMeta() = default;
	};

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

		/**
		 * Create Stream class
		 * @param[in] meta Stream meta (nullable)
		 */
		Stream(StreamMeta* meta);
		virtual ~Stream() = default;

		/**
		 * Retrieve stream ID
		 */
		uint32_t id() const;

		/**
		 * Retrieve stream type
		 */
		StreamType type() const;

		/**
		 * Return true if synchronisation is performed by this stream
		 */
		bool sync() const;

		/**
		 * Supply stream with frame data
		 * @param[in] data Data to supply
		 * @param[in] size Size of data
		 */
		virtual void supplyFrame(const uint8_t* data, size_t size, Metadata* = nullptr) = 0;

	private:
		uint32_t m_ID = 0;
		StreamType m_Type = StreamType::Other;
		bool m_Sync = false;
	};
}
