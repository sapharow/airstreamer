#pragma once
#include <transcode/transcoder.h>

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
		};

	}
}
