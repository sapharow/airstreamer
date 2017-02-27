#include <transcode/videoTranscoder.h>
#include <videoStream.h>

#define MAX_REF_FRAMES 20

namespace fp {
	namespace trans {

		uint32_t DecodedFrame::width() {
			return m_Width;
		}

		void DecodedFrame::setWidth(uint32_t width) {
			m_Width = width;
		}

		uint32_t DecodedFrame::height() {
			return m_Height;
		}

		void DecodedFrame::setHeight(uint32_t height) {
			m_Height = height;
		}

		uint32_t DecodedFrame::timebaseNum() {
			return m_TimebaseNum;
		}

		void DecodedFrame::setTimebaseNum(uint32_t timebaseNum) {
			m_TimebaseNum = timebaseNum;
		} 

		uint32_t DecodedFrame::timebaseDen() {
			return m_TimebaseDen;
		}

		void DecodedFrame::setTimebaseDen(uint32_t timebaseDen) {
			m_TimebaseDen = timebaseDen;
		}

		EncoderContext::EncoderContext(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen) 
		: m_Width(width)
		, m_Height(height)
		, m_Bitrate(bitrate)
		, m_TimebaseNum(timebaseNum)
		, m_TimebaseDen(timebaseDen)
		{ }

		uint32_t EncoderContext::width() {
			return m_Width;
		}

		uint32_t EncoderContext::height() {
			return m_Height;
		}

		uint32_t EncoderContext::bitrate() {
			return m_Bitrate;
		}

		uint32_t EncoderContext::timebaseNum() {
			return m_TimebaseNum;
		}

		uint32_t EncoderContext::timebaseDen() {
			return m_TimebaseDen;
		}

		VideoTranscoder::VideoTranscoder(StreamType inputType, const VideoStreamRef& output) 
		: Transcoder(inputType, output)
		{ }

		VideoTranscoder::~VideoTranscoder() {
			m_DecodedFramePool.clear();
			m_DecodedFrame = nullptr;
			m_DecoderContext = nullptr;
			m_EncoderContext = nullptr;
		}

		void VideoTranscoder::init() {
			m_DecoderContext = createDecoder();
			if (!m_DecoderContext) {
				throw std::runtime_error("Initialisation failed: Error creating decoder context");
			}

			// Allocate frames
			for (size_t i=0; i<MAX_REF_FRAMES; i++) {
				auto frame = allocateFrame();
				if (frame) {
					m_DecodedFramePool.push_back(frame);
				}
			}
			m_DecodedFrame = allocateFrame();
		}

		void VideoTranscoder::reset() {
			m_DecodedFramePoolSize = 0;
		}

		void VideoTranscoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata) {
			if (!m_DecoderContext) {
				return;
			}

			const size_t nBytes = m_DecoderContext->decodeFrame(data, size, metadata, m_DecodedFrame);
			if (nBytes) {
				if (!m_EncoderContext) {
					auto encoder = createEncoder(m_DecodedFrame->width(),
					                             m_DecodedFrame->height(),
					                             1024*1024,
					                             m_DecodedFrame->timebaseNum(),
					                             m_DecodedFrame->timebaseDen());
					m_EncoderContext = encoder;
					if (!m_EncoderContext) {
						return;
					}
				}

				// Determine if PTS/DTS
				uint64_t pts = 0;
				uint64_t dts = 0;
				if (metadata) {
					// Expected PTS == DTS
					if (metadata->pts) {
						pts = *metadata->pts;
						if (metadata->dts) {
							dts = *metadata->dts;
						} else {
							dts = *metadata->pts;
						}
					} else {
						throw std::runtime_error("No PTS/DTS provided");
					}
				} else {
					throw std::runtime_error("No PTS/DTS provided");
				}

				// Shuffle DTS/PTS
				if (pts == dts) {
					// Flush decoded buffer
					insertFrame();
					if (m_DecodedFramePoolSize) {
						for (size_t i=0; i<m_DecodedFramePoolSize; i++) {
							size_t nBytesEncoded = m_EncoderContext->encodeFrame(m_DecodedFramePool[i], m_EncodedDataBuffer);
							if (nBytesEncoded) {
								output()->supplyFrame(m_EncodedDataBuffer.data(), nBytesEncoded);
							}
						}
						m_DecodedFramePoolSize = 0;
					}
				} else {
					insertFrame();
				}
			}
		}
		
		void VideoTranscoder::insertFrame() {
			// Assume m_DecodedFrame[1..m_DecodedFrameCount] are sorted by PTS
			if (m_DecodedFramePoolSize+1 >= m_DecodedFramePool.size()) {
				// We can't put more frames in buffer
				// TODO:
				printf("We can't put more frames in buffer\n");
				m_DecodedFramePoolSize = 0;
			}

			size_t index = m_DecodedFramePoolSize;
			for (size_t i=0; i<m_DecodedFramePoolSize; i++) {
				if (*m_DecodedFrame < *m_DecodedFramePool[i]) {
					// Insert frame between i-1 and i
					index = i;
					break;
				}
			}

			m_DecodedFramePoolSize++;
			auto unusedFrame = m_DecodedFramePool[m_DecodedFramePoolSize];
			for (size_t k=m_DecodedFramePoolSize; k>index; k--) {
				m_DecodedFramePool[k] = m_DecodedFramePool[k-1];
			}
			m_DecodedFramePool[index] = m_DecodedFrame;
			m_DecodedFrame = unusedFrame;
		}

	}
}

