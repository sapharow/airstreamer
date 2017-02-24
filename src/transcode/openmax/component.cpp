#include <transcode/openmax/component.h>
#include <transcode/openmax/buffer.h>

namespace fp {
	namespace omx {
		
		Component::Component(const ClientRef& client,
		                     COMPONENT_T* component,
		                     const String& name, 
		                     const std::set<int>& inputPorts,
		                     const std::set<int>& outputPorts)
		: m_Client(client)
		, m_Component(component)
		, m_Name(name)
		, m_InputPorts(inputPorts)
		, m_OutputPorts(outputPorts)
		{ }

		void Component::changeState(OMX_STATETYPE state) {
			if (ilclient_change_component_state(m_Component, state) != 0) {
				throw std::runtime_error("Can not change state for component");
			}
			switch (state) {
				case OMX_StateLoaded:
					printf("\"%s\" switched to Loaded\n", name().c_str());
					break;
				case OMX_StateIdle:
					printf("\"%s\" switched to Idle\n", name().c_str());
					break;
				case OMX_StateExecuting:
					printf("\"%s\" switched to Executing\n", name().c_str());
					break;
				case OMX_StatePause:
					printf("\"%s\" switched to Pause\n", name().c_str());
					break;
				default:
					printf("\"%s\" switched to Other state\n", name().c_str());
					break;
			}
		}

		OMX_STATETYPE Component::state() {
			OMX_STATETYPE state;
			OMX_ERRORTYPE error = OMX_GetState(m_Component, &state);
			return state;
		}

		bool Component::removeEventPortSettingsChanged(int port) {
			return ilclient_remove_event(m_Component, OMX_EventPortSettingsChanged, port, 0, 0, 1) == 0;
		}

		void Component::setParameter(OMX_INDEXTYPE index, const void* parameter) {
			OMX_ERRORTYPE error = OMX_SetParameter(ILC_GET_HANDLE(m_Component), index, (OMX_PTR)parameter);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not set parameter of a component (error %x)", error));
			} 
		}

		void Component::getParameter(OMX_INDEXTYPE index, void* parameter) {
			OMX_ERRORTYPE error = OMX_GetParameter(ILC_GET_HANDLE(m_Component), index, (OMX_PTR)parameter);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not get parameter of a component (error %x)", error));
			} 
		}

		void Component::setConfig(OMX_INDEXTYPE index, const void* parameter) {
			OMX_ERRORTYPE error = OMX_SetConfig(ILC_GET_HANDLE(m_Component), index, (OMX_PTR)parameter);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not set config of a component (error %x)", error));
			}
		}

		void Component::getConfig(OMX_INDEXTYPE index, void* parameter) {
			OMX_ERRORTYPE error = OMX_GetConfig(ILC_GET_HANDLE(m_Component), index, (OMX_PTR)parameter);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not get config of a component (error %x)", error));
			}
		}

		COMPONENT_T* Component::comp() const { return m_Component; }
		const String& Component::name() const { return m_Name; }

		void Component::enablePort(int port) {
			if (m_InputPorts.count(port) || m_OutputPorts.count(port)) {
				if (!m_EnabledPorts.count(port)) {
					ilclient_enable_port(m_Component, port);
					m_EnabledPorts.insert(port);
				}
			} else {
				throw std::invalid_argument(getString("Port %d is not available for \"%s\"", port, name().c_str()));
			}
		}

		void Component::disablePort(int port) {
			if (m_InputPorts.count(port) || m_OutputPorts.count(port)) {
				if (m_EnabledPorts.count(port)) {
					ilclient_disable_port(m_Component, port);
					m_EnabledPorts.erase(port);
				}
			} else {
				throw std::invalid_argument(getString("Port %d is not available for \"%s\"", port, name().c_str()));
			}
		}

		void Component::enablePortBuffers(int port) {
			if (ilclient_enable_port_buffers(m_Component, port, nullptr, nullptr, nullptr) != 0) {
				throw std::runtime_error(getString("Can not enable port %d of \"%s\"", port, name().c_str()));
			}
			printf("Enabled buffers for port %d of \"%s\"\n", port, name().c_str());
		}

		void Component::disablePortBuffers(int port) {
			printf("Disabled buffers for port %d of \"%s\"\n", port, name().c_str());
			ilclient_disable_port_buffers(m_Component, port, nullptr, nullptr, nullptr);
		}

		void Component::disablePortBuffers() {
			printf("Disabled buffers for all ports of \"%s\"\n", name().c_str());
			ilclient_disable_port_buffers(m_Component, OMX_ALL, nullptr, nullptr, nullptr);
		}

		OMX_BUFFERHEADERTYPE* Component::getInputBuffer(int port, bool bBlocking) {
			return ilclient_get_input_buffer(m_Component, port, bBlocking ? 1 : 0);
		}

		OMX_BUFFERHEADERTYPE* Component::getOutputBuffer(int port, bool bBlocking) {
			return ilclient_get_output_buffer(m_Component, port, bBlocking ? 1 : 0);
		}

		void Component::emptyBuffer(OMX_BUFFERHEADERTYPE* buffer) {
			OMX_ERRORTYPE error = OMX_EmptyThisBuffer(ILC_GET_HANDLE(m_Component), buffer);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not empty buffer (error %x)", error));
			} 
		}

		void Component::fillBuffer(OMX_BUFFERHEADERTYPE* buffer) {
			OMX_ERRORTYPE error = OMX_FillThisBuffer(ILC_GET_HANDLE(m_Component), buffer);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not fill buffer (error %x)", error));
			} 
		}

		BufferRef Component::allocateBuffer(uint32_t port, uint32_t size) {
			OMX_BUFFERHEADERTYPE* header = nullptr;
			OMX_ERRORTYPE error = OMX_AllocateBuffer(ILC_GET_HANDLE(m_Component), &header, port, nullptr, size);
			if ( error != OMX_ErrorNone ) {
				throw std::runtime_error(getString("Can not allocate buffer for port %u with size %u (error %x)", port, size, error));
			}
			return std::make_shared<Buffer>(shared_from_this(), port, header, true);
		}

	}
}