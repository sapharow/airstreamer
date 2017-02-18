#include <transcode/transcoder.h>
#include <stream.h>

namespace fp {
	namespace trans {

		Transcoder::Transcoder(Stream::Type inputType, const StreamRef& output) 
		: m_Output(output)
		, m_InputType(inputType)
		{
			if (!m_Output) {
				throw std::invalid_argument("Output stream must not be nullptr");
			}
		}

		void Transcoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata) {
			// Null transcode
			m_Output->supplyFrame(data, size, metadata);
		}
	
		const StreamRef& Transcoder::output() const {
			return m_Output;
		}

		Stream::Type Transcoder::inputType() const {
			return m_InputType;
		}
	
	}
}