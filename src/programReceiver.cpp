#include <programReceiver.h>

namespace fp {
	namespace cap {

		ProgramReceiver::ProgramReceiver(uint32_t pid)
		: m_PID(pid)
		{ 
			printf("Found new PID = 0x%04x\n", pid);
		}

		uint32_t ProgramReceiver::pid() const {
			return m_PID;
		}


	}
}