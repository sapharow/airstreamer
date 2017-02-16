#include <program.h>

namespace fp {
	namespace cap {
		Program::Program(uint32_t pid)
		: m_PID(pid)
		{ }

		uint32_t Program::pid() const {
			return m_PID;
		}


	}
}