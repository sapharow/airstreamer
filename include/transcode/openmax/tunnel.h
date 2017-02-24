#pragma once
#include <transcode/openmax/components.h>

namespace fp {
	namespace omx {
		
		class Tunnel : public Components {
		public:
			Tunnel();
			void set(const ComponentRef& source, uint32_t sourcePort, const ComponentRef& dest, uint32_t destPort, uint32_t timeout = 0);
			virtual ~Tunnel();
		private:
			std::vector<TUNNEL_T> m_Tunnels;
		};
	}
}