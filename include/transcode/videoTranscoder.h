#pragma once
#include <transcode/transcoder.h>

namespace fp {
	namespace trans {

		DECLARE_CLASS(DecodedFrame);
		DECLARE_CLASS(DecoderContext);
		DECLARE_CLASS(EncoderContext);

		class DecodedFrame {
		public:
			virtual ~DecodedFrame() = default;

			/**
			 * Return true if this frame has earlier presentation time than reference one
			 */
			virtual bool operator <(const DecodedFrame& reference) = 0;
		};

		class DecoderContext {
		public:
			virtual ~DecoderContext() = default;

			/**
			 * Routine for video frame decoding
			 * @param[in] data Video bitstream data
			 * @param[in] size Video bitstream size
			 * @param[in] metadata Frame metadata
			 * @param[out] outFrame Decoded frame
			 * @return number of bytes decoded
			 */
			virtual size_t decodeFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata, const DecodedFrameRef& outFrame) = 0;
		};

		class EncoderContext {
		public:
			virtual ~EncoderContext() = default;

			/**
			 * Routine for video frame encoding
			 * @param[in] frame Frame to encode
			 * @param[in] dest Buffer for encoded video bitstream. Shall only grow up.
			 * @return number of bytes allocated. Return zero if no frame produced (delayed frame)
			 */
			virtual size_t encodeFrame(const DecodedFrameRef& frame, std::vector<uint8_t>& dest) = 0;
		};

		/**
		 * Transcoder class
		 */
		class VideoTranscoder : public Transcoder {
		public:

			/**
			 * Create transcoder with output
			 */
			VideoTranscoder(StreamType inputType, const VideoStreamRef& output);

			/**
			 * Initialise transcoder
			 */
			void init();

			// Overrides
			void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) override;
			void reset() override;

		protected:
			/**
			 * Create decoder context
			 */
			virtual DecoderContextRef createDecoder() = 0;

			/**
			 * Create encoder context based on decoded frame
			 */
			virtual EncoderContextRef createEncoder(const DecodedFrameRef&) = 0;

			/**
			 * Allocate frame for decoding
			 */
			virtual DecodedFrameRef allocateFrame() = 0;

		private:
			DecoderContextRef m_DecoderContext;
			EncoderContextRef m_EncoderContext;

			std::vector<DecodedFrameRef> m_DecodedFramePool;
			std::vector<uint8_t> m_EncodedDataBuffer;
			DecodedFrameRef m_DecodedFrame;
			size_t m_DecodedFramePoolSize = 0;

			uint64_t m_ExpectedPTS;
			void insertFrame();
		};

	}
}
