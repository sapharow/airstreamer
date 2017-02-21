#pragma once
#include <transcode/videoTranscoder.h>
#ifdef RPI

namespace fp {
	namespace trans {

		class CoreIVDecodedFrame : public DecodedFrame {
		public:
			CoreIVDecodedFrame();
			~CoreIVDecodedFrame() override;
			bool operator <(const DecodedFrame& reference) override;

			void* rawFrame();
		private:
			void* m_Frame = nullptr;
		};

		class CoreIVDecoderContext : public DecoderContext {
		public:
			CoreIVDecoderContext(StreamType inputType);
			~CoreIVDecoderContext() override;
			size_t decodeFrame(const uint8_t*, size_t, Stream::Metadata*, const DecodedFrameRef&) override;

			void* rawDecoder();
		private:
			int m_TimeBaseNum = 0;
			int m_TimeBaseDen = 1;
			void* m_Decoder = nullptr;
		};

		class CoreIVEncoderContext : public EncoderContext {
		public:
			CoreIVEncoderContext(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen);
			~CoreIVEncoderContext() override;
			size_t encodeFrame(const DecodedFrameRef&, std::vector<uint8_t>&) override;
		private:
			void* m_Encoder = nullptr;
		};

		/**
		 * Transcoder class
		 */
		class CoreIVVideoTranscoder : public VideoTranscoder {
		public:
			CoreIVVideoTranscoder(StreamType inputType, const VideoStreamRef& output);
		protected:
			DecoderContextRef createDecoder() override;
			EncoderContextRef createEncoder(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) override;
			DecodedFrameRef allocateFrame() override;
		};

	}
}

#endif