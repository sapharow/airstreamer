#pragma once
#include <transcode/videoTranscoder.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>


#ifdef __cplusplus
}
#endif

namespace fp {
	namespace trans {

#if 0
		/**
		 * Transcoder class
		 */
		class SoftwareVideoTranscoder : public Transcoder {
		public:

			/**
			 * Create transcoder with output
			 */
			SoftwareVideoTranscoder(StreamType inputType, 
			                   		const StreamRef& output);
			~SoftwareVideoTranscoder() override;

			void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) override;
			void reset() override;

		private:
			AVCodecContext* m_AVInputCodecContext = nullptr;
			AVCodecContext* m_AVOutputCodecContext = nullptr;

			std::vector<AVFrame*> m_DecodedFramePool;
			AVFrame* m_DecodedFrame;
			size_t m_DecodedFramePoolSize = 0;

			uint64_t m_ExpectedPTS;
			size_t findFrameWithPTS(int64_t);
			void insertFrame();
			void initialiseEncoder(uint32_t width, uint32_t height);
		};
#else
		DECLARE_CLASS(SoftwareDecodedFrame);
		class SoftwareDecodedFrame : public DecodedFrame {
		public:
			SoftwareDecodedFrame();
			~SoftwareDecodedFrame() override;
			bool operator <(const DecodedFrame& reference) override;

			AVFrame* rawFrame();
		private:
			AVFrame* m_Frame = nullptr;
		};

		class SoftwareDecoderContext : public DecoderContext {
		public:
			SoftwareDecoderContext(StreamType inputType);
			~SoftwareDecoderContext() override;
			size_t decodeFrame(const uint8_t*, size_t, Stream::Metadata*, const DecodedFrameRef&) override;
		private:
			AVCodecContext* m_Decoder = nullptr;
		};

		class SoftwareEncoderContext : public EncoderContext {
		public:
			SoftwareEncoderContext(const DecodedFrameRef&);
			~SoftwareEncoderContext() override;
			size_t encodeFrame(const DecodedFrameRef&, std::vector<uint8_t>&) override;
		private:
			AVCodecContext* m_Encoder = nullptr;
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
#endif
	}
}
