#include <audioStream.h>

namespace fp {

	AudioStream::AudioStream(AudioStreamMeta* meta) 
	: Stream(meta)
	{
		if (meta) {
			m_Lang = meta->lang;
		}
	}

	uint32_t AudioStream::lang() const {
		return m_Lang;
	}

}


