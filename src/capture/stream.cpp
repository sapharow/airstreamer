#include <capture/stream.h>

namespace fp {
	namespace cap {

		Stream::Stream(uint32_t id, Type type, bool sync, uint32_t lang) 
		: m_ID(id)
		, m_Type(type)
		, m_Sync(sync)
		, m_Lang(lang)
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

		uint32_t Stream::lang() const {
			return m_Lang;
		}

	}
}


