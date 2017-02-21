#include <transcode/softwareVideoTranscoder.h>
#include <stream.h>

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

		// SoftwareDecodedFrame /////////////////////////////////////////////////////////////////////////////

		SoftwareDecodedFrame::SoftwareDecodedFrame() {
			m_Frame = avcodec_alloc_frame();
			if (!m_Frame) {
				throw std::runtime_error("Frame is failed to initialise");
			}
		}

		SoftwareDecodedFrame::~SoftwareDecodedFrame() {
			if (m_Frame) {
				auto frame = (AVFrame*)m_Frame;
				avcodec_free_frame(&frame);
				m_Frame = nullptr;
			}
		}

		bool SoftwareDecodedFrame::operator <(const DecodedFrame& reference) {
			auto refFrame = dynamic_cast<const SoftwareDecodedFrame*>(&reference);
			if (!refFrame) {
				throw std::runtime_error("Provided incompatible class implementation");
			}
			return ((AVFrame*)m_Frame)->pkt_pts < ((AVFrame*)refFrame->m_Frame)->pkt_pts;
		}

		void* SoftwareDecodedFrame::rawFrame() {
			return m_Frame;
		}

		// SoftwareDecoderContext ///////////////////////////////////////////////////////////////////////////

		SoftwareDecoderContext::SoftwareDecoderContext(StreamType inputType) {
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
		}

		SoftwareDecoderContext::~SoftwareDecoderContext() {
			if (m_Decoder) {
				avcodec_close((AVCodecContext*)m_Decoder);
				av_free(m_Decoder);
			}
		}

		size_t SoftwareDecoderContext::decodeFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata, const DecodedFrameRef& outFrame) {
			if (!outFrame) {
				throw std::invalid_argument("Provided null output frame");
			}
			auto sFrame = dynamic_cast<SoftwareDecodedFrame*>(outFrame.get());
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
				throw std::runtime_error("error decompressing");
			}

			if (nres) {
				if (gotOutput) {
					((AVFrame*)sFrame->rawFrame())->pts = ((AVFrame*)sFrame->rawFrame())->pkt_pts;
					((AVFrame*)sFrame->rawFrame())->pict_type = AV_PICTURE_TYPE_S;
					return nres;
				}
			}

			return 0;
		}

		// SoftwareEncoderContext ///////////////////////////////////////////////////////////////////////////

		SoftwareEncoderContext::SoftwareEncoderContext(const DecodedFrameRef& frame) {
			auto sFrame = dynamic_cast<SoftwareDecodedFrame*>(frame.get());
			if (!sFrame) {
				throw std::runtime_error("Provided incompatible class implementation");
			}

			avcodec_register_all();
			av_register_all();

			auto h264 = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (h264) {
				auto encoder = avcodec_alloc_context3(h264);
				if (encoder) {
					encoder->width = ((AVFrame*)sFrame->rawFrame())->width;
					encoder->height = ((AVFrame*)sFrame->rawFrame())->height;
					encoder->sample_aspect_ratio = { 1, 1 };
					encoder->pix_fmt = AV_PIX_FMT_YUV420P;
					encoder->bit_rate = 1024;
					encoder->codec_id = AV_CODEC_ID_H264;
					encoder->codec_type = AVMEDIA_TYPE_VIDEO;
					encoder->time_base.num = 1001;
					encoder->time_base.den = 60000;
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
		}

		SoftwareEncoderContext::~SoftwareEncoderContext() {
			if (m_Encoder) {
				avcodec_close((AVCodecContext*)m_Encoder);
				av_free(m_Encoder);
			}
		}

		size_t SoftwareEncoderContext::encodeFrame(const DecodedFrameRef& frame, std::vector<uint8_t>& data) {
			if (!frame) {
				throw std::invalid_argument("Frame can not be nullptr");
			}
			auto sFrame = dynamic_cast<SoftwareDecodedFrame*>(frame.get());

			AVPacket encodePacket;
			memset(&encodePacket, 0, sizeof(AVPacket));

			// TODO: Revise this code
			if (data.size() < 1024 * 1024) {
				// 1 MB for single frame???
				data.resize(1024 * 1024);
			}

			encodePacket.data = data.data();
			encodePacket.size = data.size();

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
		}

		SoftwareVideoTranscoder::SoftwareVideoTranscoder(StreamType inputType, const VideoStreamRef& output) 
		: VideoTranscoder(inputType, output)
		{ }

		DecoderContextRef SoftwareVideoTranscoder::createDecoder() {
			return std::make_shared<SoftwareDecoderContext>(inputType());
		}

		EncoderContextRef SoftwareVideoTranscoder::createEncoder(const DecodedFrameRef& frame) {
			return std::make_shared<SoftwareEncoderContext>(frame);
		}

		DecodedFrameRef SoftwareVideoTranscoder::allocateFrame() {
			return std::make_shared<SoftwareDecodedFrame>();
		}

	}
}

