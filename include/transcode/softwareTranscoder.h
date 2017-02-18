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
		class SoftwareTranscoder : public Transcoder {
		public:

			/**
			 * Create transcoder with output
			 */
			SoftwareTranscoder(Stream::Type inputType, 
			                   const StreamRef& output);
			~SoftwareTranscoder() override;

			/**
			 * Supply frame to transcoder
			 */
			void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) override;

		private:
			AVCodecContext* m_AVInputCodecContext = nullptr;
			AVCodecContext* m_AVOutputCodecContext = nullptr;

			AVFrame* m_DecodedFrame;
			std::vector<uint8_t> m_Buffer;
		};

	}
}
