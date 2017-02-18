#include <transcode/transcoder.h>

namespace fp {
	namespace trans {

		Transcoder::Transcoder(const StreamRef& output) 
		: m_Output(output)
		{
			if (!m_Output) {
				throw std::invalid_argument("Output stream must not be nullptr");
			}
		}

		void Transcoder::supplyFrame(const uint8_t* data, size_t size) {
			// TODO
		}
		
	}
}