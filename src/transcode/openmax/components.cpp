#include <transcode/openmax/components.h>

namespace fp {
	namespace omx {
		
		Components::Components() {
			m_List.push_back(nullptr);
		}

		void Components::addComponent(const ComponentRef& component) {
			if (!component) 
				throw std::invalid_argument("OMX component can not be null");

			if (!m_Components.count(component->comp())) {
				m_Components.emplace(component->comp(), component);
				auto it = m_List.begin();
				std::advance(it, m_List.size() - 1);
				m_List.insert(it, component->comp());
			}

		}

		void Components::stateTransition(OMX_STATETYPE state) {
			ilclient_state_transition(m_List.data(), OMX_StateIdle);
		}

		Components::~Components() {
			ilclient_state_transition(m_List.data(), OMX_StateIdle);
			ilclient_state_transition(m_List.data(), OMX_StateLoaded);
			ilclient_cleanup_components(m_List.data());
		}

	}
}