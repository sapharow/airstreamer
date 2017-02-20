#include <videoStream.h>

namespace fp {

	VideoStream::VideoStream(VideoStreamMeta* meta) 
	: Stream(meta)
	{ 
		if (meta) {
			m_Width = meta->width;
			m_Height = meta->height;
		}
	}

	uint32_t VideoStream::width() const {
		return m_Width;
	}

	uint32_t VideoStream::height() const {
		return m_Height;
	}

}


