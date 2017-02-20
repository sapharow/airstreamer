#include <transcode/softwareTranscoder.h>
#include <stream.h>

#define MAX_REF_FRAMES 20

namespace fp {
	namespace trans {

		SoftwareVideoTranscoder::SoftwareVideoTranscoder(StreamType inputType, const StreamRef& output) 
		: Transcoder(inputType, output)
		{
			if (inputType == StreamType::Video_H262) {

				avcodec_register_all();
				av_register_all();
				avfilter_register_all();

				auto h262 = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
				m_AVInputCodecContext = avcodec_alloc_context3(h262);
				auto ret = avcodec_open2(m_AVInputCodecContext, h262, 0);

				for (size_t i=0; i<MAX_REF_FRAMES; i++) {
					m_DecodedFramePool.push_back(avcodec_alloc_frame());
				}
				m_DecodedFrame = avcodec_alloc_frame();

				auto h264 = avcodec_find_encoder(AV_CODEC_ID_H264);
				m_AVOutputCodecContext = avcodec_alloc_context3(h264);
				m_AVOutputCodecContext->width = 1280;
				m_AVOutputCodecContext->height = 720;
				m_AVOutputCodecContext->sample_aspect_ratio = { 1, 1 };
				m_AVOutputCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
				m_AVOutputCodecContext->bit_rate = 1024;
				m_AVOutputCodecContext->codec_id = AV_CODEC_ID_H264;
				m_AVOutputCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
				m_AVOutputCodecContext->time_base.num = 1001;
				m_AVOutputCodecContext->time_base.den = 60000;
//				m_AVOutputCodecContext->thread_count = 1;

				m_AVOutputCodecContext->flags |= CODEC_FLAG_CLOSED_GOP | CODEC_FLAG_LOW_DELAY /*CODEC_FLAG_GLOBAL_HEADER*/;
				AVDictionary *opts = NULL;
//				av_dict_set(&opts, "aud", "1", 0);

				printf("refs = %u\n", m_AVOutputCodecContext->refs);

				ret = avcodec_open2(m_AVOutputCodecContext, h264, &opts);
				if (ret != 0) {
					av_free(m_AVInputCodecContext);
					m_AVInputCodecContext = 0;
					char buf[256];
					av_strerror(ret, buf, 256);
					printf("%s\n", buf);
				}
			} else {
				printf("null transcoding\n");
			}
		}

		SoftwareVideoTranscoder::~SoftwareVideoTranscoder() {
			if (m_AVInputCodecContext) {
				avcodec_close(m_AVInputCodecContext);
				av_free(m_AVInputCodecContext);
			}
			if (m_AVOutputCodecContext) {
				avcodec_close(m_AVOutputCodecContext);
				av_free(m_AVOutputCodecContext);
			}
			for (size_t i=0; i<m_DecodedFramePool.size(); i++) {
				avcodec_free_frame(&m_DecodedFramePool[i]);
			}
			avcodec_free_frame(&m_DecodedFrame);
		}

		void SoftwareVideoTranscoder::reset() {
			m_DecodedFramePoolSize = 0;
		}

		void SoftwareVideoTranscoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata) {
			if (!m_AVInputCodecContext || !m_AVOutputCodecContext) {
				return;
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

			//packet.flags |= AV_PKT_FLAG_KEY;

//			char buf[256];
//			av_strerror(-22, buf, 256);
//			printf("error = %s\n", buf);

			int gotOutput = 0;
			int nres = avcodec_decode_video2(m_AVInputCodecContext, m_DecodedFrame, &gotOutput, &decodePacket);
			if (nres == 0) {
				printf("no frame could be decompressed\n");
			} else if (nres < 0) {
				printf("error decompressing\n");
			} else {
				if (gotOutput) {
					// Determine if PTS != DTS
					uint64_t pts = 0;
					uint64_t dts = 0;
					if (metadata) {
						// Expected PTS == DTS
						if (metadata->pts) {
							pts = *metadata->pts;
							if (metadata->dts) {
								dts = *metadata->dts;
							} else {
								dts = *metadata->pts;
							}
						} else {
							// TODO: No PTS/DTS
							printf("No metadata\n");
						}
					} else {
						// TODO: No PTS/DTS
						printf("No metadata\n");
					}

					if (pts == dts) {
						// Flush decoded buffer
						insertFrame();
						if (m_DecodedFramePoolSize) {
							for (size_t i=0; i<m_DecodedFramePoolSize; i++) {
								m_DecodedFramePool[i]->pts = m_DecodedFramePool[i]->pkt_pts;

								m_DecodedFramePool[i]->pict_type = AV_PICTURE_TYPE_S;
								AVPacket encodePacket;
								memset(&encodePacket, 0, sizeof(AVPacket));
								auto ret = avcodec_encode_video2(m_AVOutputCodecContext, &encodePacket, m_DecodedFramePool[i], &gotOutput);
								if (!ret && gotOutput && m_AVOutputCodecContext->coded_frame) {
									m_AVOutputCodecContext->coded_frame->pts = encodePacket.pts;
									m_AVOutputCodecContext->coded_frame->key_frame = !!(encodePacket.flags & AV_PKT_FLAG_KEY);
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
										printf("got output\n");
										output()->supplyFrame(encodePacket.data, encodePacket.size);
									} else {
										printf("no output\n");
									}
								} else {
									printf("error %d\n", ret);
								}
							}
							m_DecodedFramePoolSize = 0;
						}
					} else {
						insertFrame();
					}
				}
			}

		}
		
		size_t SoftwareVideoTranscoder::findFrameWithPTS(int64_t pts) {
			for (size_t i=0; i<m_DecodedFramePoolSize; i++) {
				if (pts == m_DecodedFramePool[i]->pkt_pts) {
					// Found frame to encode
					return i;
				}
			}
			return -1;
		}

		void SoftwareVideoTranscoder::insertFrame() {
			// Assume m_DecodedFrame[1..m_DecodedFrameCount] are sorted by PTS
			if (m_DecodedFramePoolSize+1 > MAX_REF_FRAMES) {
				// We can't put more frames in buffer
				// TODO:
				printf("We can't put more frames in buffer\n");
				m_DecodedFramePoolSize = 0;
			}

			size_t index = m_DecodedFramePoolSize;
			for (size_t i=0; i<m_DecodedFramePoolSize; i++) {
				if (m_DecodedFrame->pkt_pts < m_DecodedFramePool[i]->pkt_pts) {
					// Insert frame between i-1 and i
					index = i;
					break;
				}
			}

			m_DecodedFramePoolSize++;
			auto unusedFrame = m_DecodedFramePool[m_DecodedFramePoolSize];
			for (size_t k=m_DecodedFramePoolSize; k>index; k--) {
				m_DecodedFramePool[k] = m_DecodedFramePool[k-1];
			}
			m_DecodedFramePool[index] = m_DecodedFrame;
			m_DecodedFrame = unusedFrame;
		}

	}
}

