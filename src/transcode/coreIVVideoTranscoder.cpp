#include <transcode/coreIVVideoTranscoder.h>
#include <stream.h>
#ifdef RPI

#include "bcm_host.h"
#include "ilclient.h"

namespace fp {
	namespace trans {

		// CoreIVDecodedFrame /////////////////////////////////////////////////////////////////////////////

		CoreIVDecodedFrame::CoreIVDecodedFrame() {
#if 0
			m_Frame = avcodec_alloc_frame();
			if (!m_Frame) {
				throw std::runtime_error("Frame is failed to initialise");
			}
#else
			// TODO
#endif
		}

		CoreIVDecodedFrame::~CoreIVDecodedFrame() {
#if 0
			if (m_Frame) {
				auto frame = (AVFrame*)m_Frame;
				avcodec_free_frame(&frame);
				m_Frame = nullptr;
			}
#else
			// TODO
#endif
		}

		bool CoreIVDecodedFrame::operator <(const DecodedFrame& reference) {
#if 0
			auto refFrame = dynamic_cast<const CoreIVDecodedFrame*>(&reference);
			if (!refFrame) {
				throw std::runtime_error("Provided incompatible class implementation");
			}
			return ((AVFrame*)m_Frame)->pkt_pts < ((AVFrame*)refFrame->m_Frame)->pkt_pts;
#else
			// TODO
			return false;
#endif
		}

		void* CoreIVDecodedFrame::rawFrame() {
#if 0
			return m_Frame;
#else
			// TODO
			return nullptr;
#endif
		}

		// CoreIVDecoderContext ///////////////////////////////////////////////////////////////////////////

		CoreIVDecoderContext::CoreIVDecoderContext(StreamType inputType) {
#if 0
			avcodec_register_all();
			av_register_all();

			if (inputType == StreamType::Video_H262) {
				auto h262 = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
				if (h262) {
					m_Decoder = avcodec_alloc_context3(h262);
					if (m_Decoder) {
						auto ret = avcodec_open2((AVCodecContext*)m_Decoder, h262, 0);
						if (ret != 0) {
							av_free(m_Decoder);
							m_Decoder = nullptr;
							throw std::runtime_error("Can not initialise H.262 decoder");
						}
					} else {
						throw std::runtime_error("Can not allocate H.262 decoder context");
					}
				} else {
					throw std::runtime_error("Can not find H.262 decoder");
				}
			} else {
				throw std::invalid_argument("Unsupported input stream type");
			}
#else
			// TODO
#endif
		}

		CoreIVDecoderContext::~CoreIVDecoderContext() {
#if 0
			if (m_Decoder) {
				avcodec_close((AVCodecContext*)m_Decoder);
				av_free(m_Decoder);
			}
#else
			// TODO
#endif
		}

		void* CoreIVDecoderContext::rawDecoder() {
#if 0
			return m_Decoder;
#else
			// TODO
			return nullptr;
#endif
		}

		size_t CoreIVDecoderContext::decodeFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata, const DecodedFrameRef& outFrame) {
#if 0
			if (!outFrame) {
				throw std::invalid_argument("Provided null output frame");
			}
			auto sFrame = dynamic_cast<CoreIVDecodedFrame*>(outFrame.get());
			if (!sFrame) {
				throw std::invalid_argument("Provided incompatible class implementation");
			}

			AVPacket decodePacket;
			memset(&decodePacket, 0, sizeof(AVPacket));
			decodePacket.data = (uint8_t*)data;
			decodePacket.size = size;

			if (metadata) {
				if (metadata->pts) {
					decodePacket.pts = *metadata->pts;
				}
				if (metadata->dts) {
					decodePacket.dts = *metadata->dts;
				}
			}

			int gotOutput = 0;
			int nres = avcodec_decode_video2((AVCodecContext*)m_Decoder, (AVFrame*)sFrame->rawFrame(), &gotOutput, &decodePacket);
			if (nres < 0) {
				throw std::runtime_error("error decoding frame");
			}

			if (nres) {
				if (gotOutput) {
					// Setup timebase
					m_TimeBaseNum = ((AVCodecContext*)m_Decoder)->time_base.num;
					m_TimeBaseDen = ((AVCodecContext*)m_Decoder)->time_base.den;

					if (((AVCodecContext*)m_Decoder)->ticks_per_frame) {
						m_TimeBaseNum *= ((AVCodecContext*)m_Decoder)->ticks_per_frame;
					}

					outFrame->setWidth(((AVFrame*)sFrame->rawFrame())->width);
					outFrame->setHeight(((AVFrame*)sFrame->rawFrame())->height);
					outFrame->setTimebaseNum(m_TimeBaseNum);
					outFrame->setTimebaseDen(m_TimeBaseDen);
					return nres;
				}
			}

#else
			// TODO
#endif
			return 0;
		}

		// CoreIVEncoderContext ///////////////////////////////////////////////////////////////////////////

		CoreIVEncoderContext::CoreIVEncoderContext(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen) 
		: EncoderContext(width, height, bitrate, timebaseNum, timebaseDen)
		{
#if 0
			avcodec_register_all();
			av_register_all();

			auto h264 = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (h264) {
				auto encoder = avcodec_alloc_context3(h264);
				if (encoder) {
					encoder->width = width;
					encoder->height = height;
					encoder->sample_aspect_ratio = { 1, 1 };
					encoder->pix_fmt = AV_PIX_FMT_YUV420P;
					encoder->bit_rate = bitrate / 1024;
					encoder->codec_id = AV_CODEC_ID_H264;
					encoder->codec_type = AVMEDIA_TYPE_VIDEO;
					encoder->time_base.num = timebaseNum;
					encoder->time_base.den = timebaseDen;
					encoder->flags |= CODEC_FLAG_CLOSED_GOP;

					auto ret = avcodec_open2(encoder, h264, nullptr);
					if (ret != 0) {
						av_free(encoder);
						m_Encoder = nullptr;
						throw std::runtime_error("Can not initialise H.264 decoder");
					}

					m_Encoder = encoder;
				} else {
					throw std::runtime_error("Can not allocate H.264 decoder context");
				}
			} else {
				throw std::runtime_error("Can not find H.264 decoder");
			}
#else
			// TODO
#endif
		}

		CoreIVEncoderContext::~CoreIVEncoderContext() {
#if 0
			if (m_Encoder) {
				avcodec_close((AVCodecContext*)m_Encoder);
				av_free(m_Encoder);
			}
#else
			// TODO
#endif
		}

		size_t CoreIVEncoderContext::encodeFrame(const DecodedFrameRef& frame, std::vector<uint8_t>& data) {
#if 0
			if (!frame) {
				throw std::invalid_argument("Frame can not be nullptr");
			}
			auto sFrame = dynamic_cast<CoreIVDecodedFrame*>(frame.get());
			if (!sFrame) {
				throw std::invalid_argument("Provided incompatible class implementation");
			}

			AVPacket encodePacket;
			memset(&encodePacket, 0, sizeof(AVPacket));

			// TODO: Revise this code
			if (data.size() < 1024 * 1024) {
				// 1 MB for single frame???
				data.resize(1024 * 1024);
			}

			encodePacket.data = data.data();
			encodePacket.size = data.size();

			// Copy PTS data for encoder
			((AVFrame*)sFrame->rawFrame())->pts = ((AVFrame*)sFrame->rawFrame())->pkt_pts;
			((AVFrame*)sFrame->rawFrame())->pict_type = AV_PICTURE_TYPE_S;

			int gotOutput;
			auto ret = avcodec_encode_video2((AVCodecContext*)m_Encoder, &encodePacket, (AVFrame*)sFrame->rawFrame(), &gotOutput);
			if (!ret && gotOutput && ((AVCodecContext*)m_Encoder)->coded_frame) {
				((AVCodecContext*)m_Encoder)->coded_frame->pts = encodePacket.pts;
				((AVCodecContext*)m_Encoder)->coded_frame->key_frame = !!(encodePacket.flags & AV_PKT_FLAG_KEY);
			}
/*
			if (encodePacket.side_data_elems > 0) {
				for (auto i=0; i<encodePacket.side_data_elems; i++) {
					av_free(encodePacket.side_data[i].data);
				}
				av_freep(&encodePacket.side_data);
			}
*/
			if (ret == 0) {
				if (gotOutput) {
					return encodePacket.size;
				} else {
					return 0;
				}
			} else {
				throw std::runtime_error("Error encoding video");
			}
#else
			// TODO
			return 0;
#endif
		}

		CoreIVVideoTranscoder::CoreIVVideoTranscoder(StreamType inputType, const VideoStreamRef& output) 
		: VideoTranscoder(inputType, output)
		{ }

		DecoderContextRef CoreIVVideoTranscoder::createDecoder() {
#if 0
			return std::make_shared<CoreIVDecoderContext>(inputType());
#else
			// TODO
			return nullptr;
#endif
		}

		EncoderContextRef CoreIVVideoTranscoder::createEncoder(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen) {
#if 0
			return std::make_shared<CoreIVEncoderContext>(width, height, bitrate, timebaseNum, timebaseDen);
#else
			// TODO
			return nullptr;
#endif
		}

		DecodedFrameRef CoreIVVideoTranscoder::allocateFrame() {
#if 0
			return std::make_shared<CoreIVDecodedFrame>();
#else
			// TODO
			return nullptr;
#endif
		}

	}
}

#endif