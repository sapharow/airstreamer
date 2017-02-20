#include <stream.h>

namespace fp {

	Stream::Stream(StreamMeta* meta) 
	{ 
		if (meta) {
			m_ID = meta->id;
			m_Type = meta->type;
			m_Sync = meta->sync;
		}
	}

	uint32_t Stream::id() const {
		return m_ID;
	}

	StreamType Stream::type() const {
		return m_Type;
	}

	bool Stream::sync() const {
		return m_Sync;
	}

}


