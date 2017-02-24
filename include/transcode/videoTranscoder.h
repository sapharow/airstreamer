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

			/**
			 * Frame width
			 * Set by decoder
			 */
			uint32_t width();
			void setWidth(uint32_t);

			/**
			 * Frame height
			 * Set by decoder
			 */
			uint32_t height();
			void setHeight(uint32_t);

			/**
			 * Timebase number
			 * Set by decoder
			 */
			uint32_t timebaseNum();
			void setTimebaseNum(uint32_t);

			/**
			 * Timebase denominator
			 * Set by decoder
			 */
			uint32_t timebaseDen();
			void setTimebaseDen(uint32_t);

		private:
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
			uint32_t m_TimebaseNum = 0;
			uint32_t m_TimebaseDen = 1;
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
			EncoderContext(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen);
			virtual ~EncoderContext() = default;

			/**
			 * Video width 
			 */
			uint32_t width();

			/**
			 * Video height
			 */
			uint32_t height();

			/**
			 * Video stream bitrate
			 */
			uint32_t bitrate();

			/**
			 * Timebase number
			 */
			uint32_t timebaseNum();

			/**
			 * Timebase denominator
			 */
			uint32_t timebaseDen();

			/**
			 * Routine for video frame encoding
			 * @param[in] frame Frame to encode
			 * @param[in] dest Buffer for encoded video bitstream. Shall only grow up.
			 * @return number of bytes allocated. Return zero if no frame produced (delayed frame)
			 */
			virtual size_t encodeFrame(const DecodedFrameRef& frame, std::vector<uint8_t>& dest) = 0;

		private:
			uint32_t m_Width;
			uint32_t m_Height;
			uint32_t m_Bitrate;
			uint32_t m_TimebaseNum;
			uint32_t m_TimebaseDen;
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
			~VideoTranscoder() override;

			// Overrides
			bool init() override;
			void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) override;
			void reset() override;

		protected:
			/**
			 * Create decoder context
			 */
			virtual DecoderContextRef createDecoder() = 0;

			/**
			 * Create encoder context
			 * @param[in] width Output image width
			 * @param[in] height Output image height
			 * @param[in] bitrate Output bitstream bitrate, bps
			 * @param[in] timebaseNum Output bitstream timebase number
			 * @param[in] timebaseDen Output bitstream timebase denominator
			 */
			virtual EncoderContextRef createEncoder(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen) = 0;

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
