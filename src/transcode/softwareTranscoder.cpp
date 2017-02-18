#include <transcode/softwareTranscoder.h>
#include <stream.h>

namespace fp {
	namespace trans {

		SoftwareTranscoder::SoftwareTranscoder(Stream::Type inputType, const StreamRef& output) 
		: Transcoder(inputType, output)
		{
			if (inputType == Stream::Type::Video_H262) {

				avcodec_register_all();
				av_register_all();
				avfilter_register_all();

				auto h262 = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
				m_AVInputCodecContext = avcodec_alloc_context3(h262);
				auto ret = avcodec_open2(m_AVInputCodecContext, h262, 0);
				m_DecodedFrame = avcodec_alloc_frame();

				auto h264 = avcodec_find_encoder(AV_CODEC_ID_H264);
				m_AVOutputCodecContext = avcodec_alloc_context3(h264);
				m_AVOutputCodecContext->width = 640;
				m_AVOutputCodecContext->height = 480;
				m_AVOutputCodecContext->sample_aspect_ratio = { 16, 9 };
				m_AVOutputCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
				m_AVOutputCodecContext->bit_rate = 1024 * 1024;
				m_AVOutputCodecContext->codec_id = AV_CODEC_ID_H264;
				m_AVOutputCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
				m_AVOutputCodecContext->pix_fmt = PIX_FMT_YUV420P;

				ret = avcodec_open2(m_AVOutputCodecContext, h264, 0);
				if (ret != 0) {
					char buf[256];
					av_strerror(ret, buf, 256);
					printf("%s\n", buf);
				}
				printf("ret=%d\n", ret);

/*
				m_AVOutputContext = avformat_alloc_context();
				if (m_AVOutputContext) {
					m_AVOutputStream = avformat_new_stream(m_AVOutputContext, h264);
					if (m_AVOutputStream) {
						m_AVOutputCodecContext = m_AVInputStream->codec;
						m_AVOutputCodecContext->rc_max_rate = 1 * 1024 * 1024;
						m_AVOutputCodecContext->rc_min_rate = 1 * 1024 * 1024;
						m_AVOutputCodecContext->width = 640;
						m_AVOutputCodecContext->height = 480;
						m_AVOutputCodecContext->sample_aspect_ratio = { 16, 9 };
						m_AVOutputCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
						m_AVOutputCodecContext->time_base = { 1, 1000000 };
//						m_AVOutputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
						auto ret = avcodec_open2(m_AVOutputCodecContext, h264, nullptr);
						if (ret < 0) {
							throw std::runtime_error("Can not open codec context");
						}
						printf("ok\n");
					} else {
						av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
						throw std::runtime_error("");
					}

				} else {
					av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
					throw std::runtime_error("");
				}
*/				
			} else {
				printf("null transcoding\n");
			}
		}

		SoftwareTranscoder::~SoftwareTranscoder() {
			if (m_AVInputCodecContext) {
				avcodec_close(m_AVInputCodecContext);
				av_free(m_AVInputCodecContext);
			}
			if (m_AVOutputCodecContext) {
				avcodec_close(m_AVOutputCodecContext);
				av_free(m_AVOutputCodecContext);
			}
		}

		void SoftwareTranscoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata) {

			int frameFinished = 0;
			AVPacket packet;
			memset(&packet, 0, sizeof(AVPacket));
			packet.data = (uint8_t*)data;
			packet.size = size;
			if (metadata) {
				if (metadata->pts) {
					packet.pts = *metadata->pts;
				}
				if (metadata->dts) {
					packet.dts = *metadata->dts;
				}
			}
			/*
			if (metadata) {
				if (metadata->dts) {
					printf("DTS=%u ", *metadata->dts);
				}
				if (metadata->pts) {
					printf("PTS=%u ", *metadata->pts);
				}
				printf("\n");
			}
			*/
			//packet.flags |= AV_PKT_FLAG_KEY;
			int nres = avcodec_decode_video2(m_AVInputCodecContext, m_DecodedFrame, &frameFinished, &packet);
			if (nres == 0) {
				printf("no frame could be decompressed\n");
			} else if (nres < 0) {
				printf("error decompressing\n");
			} else {
				if (frameFinished) {
					// Frame decompressed successfully
					// Compress it
					if (m_Buffer.empty()) {
						m_Buffer.resize(1024 * 100); // 100 kb
					}
					packet.data = nullptr;// (uint8_t*)m_Buffer.data();
					packet.size = 0;//m_Buffer.size();
					printf("encode\n");
					avcodec_encode_video2(m_AVOutputCodecContext, &packet, m_DecodedFrame, &frameFinished);
					printf("encoded\n");
					output()->supplyFrame(packet.data, packet.size);
				} else {
					printf("frameFinished=%u\n", frameFinished);
				}
			}
//			auto ret = avcodec_send_packet(m_AVInputCodecContext, &m_AVInputPacket);
//			if (ret != 0) {
//				printf("Can not accept frame\n");
//			}

//			ret = avcodec_receive_frame(m_AVInputCodecContext, &m_DecodedFrame);
//			if (ret != 0) {
//				printf("Can not receive frame\n");
//			}


//			av_init_packet(&m_AVOutputPacket);

		}
		
	}
}

