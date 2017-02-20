#pragma once
#include <stream.h>

namespace fp {

	struct VideoStreamMeta : public StreamMeta {
		uint32_t   width;	// Video width
		uint32_t   height;	// Video height
	};

	/**
	 * Stream is class which describes elementary stream
	 * associated with particular program
	 */
	class VideoStream : public Stream {
	public:

		/**
		 * Create Stream class
		 * @param[in] meta Stream meta
		 */
		VideoStream(VideoStreamMeta* meta);

		/**
		 * Get stream width
		 */
		uint32_t width() const;

		/**
		 * Get stream height
		 */
		uint32_t height() const;

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};
}
