#pragma once
#include <transcode/openmax/component.h>

namespace fp {
	namespace omx {

		class Components {
		public:
			Components();
			void addComponent(const ComponentRef& component);
			void stateTransition(OMX_STATETYPE state);
			virtual ~Components();
		
		protected:
			typedef std::map<COMPONENT_T*, ComponentRef> Map;
			Map m_Components;

		private:
			std::vector<COMPONENT_T*> m_List;
		};
	}
}