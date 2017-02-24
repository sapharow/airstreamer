#pragma once

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "bcm_host.h"
#include "ilclient.h"

#ifdef __cplusplus
}
#endif

namespace fp {
	namespace omx {

		class Buffer {
		public:
			Buffer(const ComponentRef& component, uint32_t port, OMX_BUFFERHEADERTYPE* header, bool bOwnBuffer);
			virtual ~Buffer();

			OMX_BUFFERHEADERTYPE* header() const { return m_Header; }
		private:
			ComponentRef m_Component;
			uint32_t m_Port;
			OMX_BUFFERHEADERTYPE* m_Header;
			bool m_OwnBuffer;
		};

	}
}
