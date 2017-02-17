#include <program.h>

namespace fp {
	namespace cap {

		Program::Program(uint32_t id, const Streams& streams) 
		: m_ID(id)
		, m_Streams(streams)
		{ }

		const Program::Streams& Program::streams() const {
			return m_Streams;
		}

		uint32_t Program::id() const {
			return m_ID;
		}

	}
}

