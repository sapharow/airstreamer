#include <stream.h>

namespace fp {
	namespace cap {

		Stream::Stream(uint32_t id, Type type, bool sync) 
		: m_ID(id)
		, m_Type(type)
		, m_Sync(sync)
		{ }

		uint32_t Stream::id() const {
			return m_ID;
		}

		Stream::Type Stream::type() const {
			return m_Type;
		}

		bool Stream::sync() const {
			return m_Sync;
		}

	}
}


