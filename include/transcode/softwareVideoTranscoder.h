#pragma once
#include <transcode/videoTranscoder.h>

namespace fp {
	namespace trans {

		class SoftwareDecodedFrame : public DecodedFrame {
		public:
			SoftwareDecodedFrame();
			~SoftwareDecodedFrame() override;
			bool operator <(const DecodedFrame& reference) override;

			void* rawFrame();
		private:
			void* m_Frame = nullptr;
		};

		class SoftwareDecoderContext : public DecoderContext {
		public:
			SoftwareDecoderContext(StreamType inputType);
			~SoftwareDecoderContext() override;
			size_t decodeFrame(const uint8_t*, size_t, Stream::Metadata*, const DecodedFrameRef&) override;
		private:
			void* m_Decoder = nullptr;
		};

		class SoftwareEncoderContext : public EncoderContext {
		public:
			SoftwareEncoderContext(const DecodedFrameRef&);
			~SoftwareEncoderContext() override;
			size_t encodeFrame(const DecodedFrameRef&, std::vector<uint8_t>&) override;
		private:
			void* m_Encoder = nullptr;
		};

		/**
		 * Transcoder class
		 */
		class SoftwareVideoTranscoder : public VideoTranscoder {
		public:
			SoftwareVideoTranscoder(StreamType inputType, const VideoStreamRef& output);
		protected:
			DecoderContextRef createDecoder() override;
			EncoderContextRef createEncoder(const DecodedFrameRef&) override;
			DecodedFrameRef allocateFrame() override;
		};

	}
}
