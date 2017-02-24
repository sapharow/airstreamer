#include <transcode/openmax/tunnel.h>

namespace fp {
	namespace omx {

		Tunnel::Tunnel() {
			TUNNEL_T Tunnel = { nullptr, 0, nullptr, 0 };
			m_Tunnels.push_back(Tunnel);
		}

		void Tunnel::set(const ComponentRef& source, uint32_t sourcePort, const ComponentRef& dest, uint32_t destPort, uint32_t timeout) {
			if (!source) 
				throw std::invalid_argument("Source OMX component can not be null");
			if (!dest) 
				throw std::invalid_argument("Destination OMX component can not be null");

			printf("Setting up tunnel between \"%s\" and \"%s\"...\n", source->name().c_str(), dest->name().c_str());

			// Setup tunnel
			TUNNEL_T Tunnel;
			set_tunnel(&Tunnel, source->comp(), sourcePort, dest->comp(), destPort);
			int res = ilclient_setup_tunnel(&Tunnel, 0, timeout);
			switch (res) {
				case 0: {
					addComponent(source);
					addComponent(dest);

					auto it = m_Tunnels.begin();
					std::advance(it, m_Tunnels.size() - 1);
					m_Tunnels.insert(it, Tunnel);
	
					printf("Created tunnel between \"%s\" and \"%s\"\n", source->name().c_str(), dest->name().c_str());
					break;
				}
				case -1: throw std::runtime_error("A timeout waiting for the parameter changed");
				case -2: throw std::runtime_error("An error was returned instead of parameter changed");
				case -3: throw std::runtime_error("No streams are available from this port");
				case -4: throw std::runtime_error("Requested stream is not available from this port");
				case -5: throw std::runtime_error("The data format was not acceptable to the sink");
				default: throw std::runtime_error(getString("Unknown error (%d)", res));
			}
		}

		Tunnel::~Tunnel() {
			for (size_t i=0; i<m_Tunnels.size(); i++) {
				ilclient_disable_tunnel(&m_Tunnels[i]);
			}
			for (auto i : m_Components) {
				i.second->disablePortBuffers();
			}
			ilclient_teardown_tunnels(m_Tunnels.data());
		}

	}
}