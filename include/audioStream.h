#pragma once
#include <stream.h>

namespace fp {

	struct AudioStreamMeta : public StreamMeta {
		uint32_t   lang;	// Stream language
	};

	/**
	 * AudioStream is class which describes audio elementary stream
	 * associated with particular program
	 */
	class AudioStream : public Stream {
	public:
		/**
		 * Create Stream class
		 * @param[in] meta Stream meta
		 */
		AudioStream(AudioStreamMeta* meta);

		/**
		 * Stream language as 3-bytes code
		 * 0 means no language assigned
		 */
		uint32_t lang() const;

	private:
		uint32_t m_Lang = 0;
	};
}
