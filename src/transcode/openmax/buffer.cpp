#include <transcode/openmax/buffer.h>
#include <transcode/openmax/component.h>

namespace fp {
	namespace omx {

		Buffer::Buffer(const ComponentRef& component, uint32_t port, OMX_BUFFERHEADERTYPE* header, bool bOwnBuffer) 
		: m_Component(component)
		, m_Port(port)
		, m_Header(header)
		, m_OwnBuffer(bOwnBuffer)
		{ }


		Buffer::~Buffer() {
			if (m_OwnBuffer) {
				OMX_FreeBuffer(m_Component->comp(), m_Port, m_Header);
			}
		}
	}
}