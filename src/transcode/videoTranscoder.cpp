#include <transcode/videoTranscoder.h>
#include <videoStream.h>

#define MAX_REF_FRAMES 20

namespace fp {
	namespace trans {

		VideoTranscoder::VideoTranscoder(StreamType inputType, const VideoStreamRef& output) 
		: Transcoder(inputType, output)
		{ }

		void VideoTranscoder::init() {
			auto decoderContext = createDecoder();
			if (!decoderContext) {
				throw std::runtime_error("Initialisation failed: Error creating decoder context");
			}
			m_DecoderContext = decoderContext;

			// Allocate frames
			for (size_t i=0; i<MAX_REF_FRAMES; i++) {
				auto frame = allocateFrame();
				if (!frame) {
					throw std::runtime_error("Initialisation failed: Error allocating decoder frames");
				}
				m_DecodedFramePool.push_back(frame);
			}
			auto frame = allocateFrame();
			if (!frame) {
				throw std::runtime_error("Initialisation failed: Error allocating decoder frames");
			}
			m_DecodedFrame = frame;
		}

		void VideoTranscoder::reset() {
			m_DecodedFramePoolSize = 0;
		}

		void VideoTranscoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata) {
			if (!m_DecoderContext) {
				throw std::runtime_error("Transcoder is not initialised");
			}

			try {
				const size_t nBytes = m_DecoderContext->decodeFrame(data, size, metadata, m_DecodedFrame);
				if (nBytes) {
					if (!m_EncoderContext) {
						auto encoder = createEncoder(m_DecodedFrame);
						if (!encoder) {
							throw std::runtime_error("Can not initialise encoder");
						}
						m_EncoderContext = encoder;
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
				} else {
					printf("no frame decompressed\n");
				}
			} catch (std::exception&) {
				throw;
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

