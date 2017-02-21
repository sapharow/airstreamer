#include <transcode/softwareVideoTranscoder.h>
#include <stream.h>

#define MAX_REF_FRAMES 20

namespace fp {
	namespace trans {
#if 0
		SoftwareVideoTranscoder::SoftwareVideoTranscoder(StreamType inputType, const StreamRef& output) 
		: Transcoder(inputType, output)
		{
			if (inputType == StreamType::Video_H262) {
				avcodec_register_all();
				av_register_all();
				avfilter_register_all();

				auto h262 = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
				if (h262) {
					m_AVInputCodecContext = avcodec_alloc_context3(h262);
					if (m_AVInputCodecContext) {
						auto ret = avcodec_open2(m_AVInputCodecContext, h262, 0);
						if (ret == 0) {
							// Allocate frames
							for (size_t i=0; i<MAX_REF_FRAMES; i++) {
								m_DecodedFramePool.push_back(avcodec_alloc_frame());
							}
							m_DecodedFrame = avcodec_alloc_frame();
						} else {
							throw std::runtime_error("Can not initialise H.262 decoder");
						}
					} else {
						throw std::runtime_error("Can not allocate decoder context");
					}
				} else {
				throw std::runtime_error("Can not find H.264 decoder");
				}
			} else {
				printf("null transcoding\n");
			}
		}

		void SoftwareVideoTranscoder::initialiseEncoder(uint32_t width, uint32_t height) {

			auto h264 = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (h264) {
				m_AVOutputCodecContext = avcodec_alloc_context3(h264);
				if (m_AVOutputCodecContext) {
					m_AVOutputCodecContext->width = width;
					m_AVOutputCodecContext->height = height;
					m_AVOutputCodecContext->sample_aspect_ratio = { 1, 1 };
					m_AVOutputCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
					m_AVOutputCodecContext->bit_rate = 1024;
					m_AVOutputCodecContext->codec_id = AV_CODEC_ID_H264;
					m_AVOutputCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
					m_AVOutputCodecContext->time_base.num = 1001;
					m_AVOutputCodecContext->time_base.den = 60000;
					m_AVOutputCodecContext->flags |= CODEC_FLAG_CLOSED_GOP;

					auto ret = avcodec_open2(m_AVOutputCodecContext, h264, nullptr);
					if (ret != 0) {
						av_free(m_AVInputCodecContext);
						m_AVInputCodecContext = 0;
						char buf[256];
						av_strerror(ret, buf, 256);
						printf("%s\n", buf);
						throw std::runtime_error("Can not initialise H.264 decoder");
					}
				}
			} else {
				throw std::runtime_error("Can not find H.262 decoder");
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
			if (!m_AVInputCodecContext) {
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

			int gotOutput = 0;
			int nres = avcodec_decode_video2(m_AVInputCodecContext, m_DecodedFrame, &gotOutput, &decodePacket);
			if (nres == 0) {
				printf("no frame could be decompressed\n");
			} else if (nres < 0) {
				printf("error decompressing\n");
			} else {
				if (gotOutput) {

					if (!m_AVOutputCodecContext) {
						initialiseEncoder(m_DecodedFrame->width, m_DecodedFrame->height);
					}

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
										output()->supplyFrame(encodePacket.data, encodePacket.size);
									} else {
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
#else
		// SoftwareDecodedFrame /////////////////////////////////////////////////////////////////////////////

		SoftwareDecodedFrame::SoftwareDecodedFrame() {
			m_Frame = avcodec_alloc_frame();
			if (!m_Frame) {
				throw std::runtime_error("Frame is failed to initialise");
			}
		}

		SoftwareDecodedFrame::~SoftwareDecodedFrame() {
			if (m_Frame) {
				avcodec_free_frame(&m_Frame);
				m_Frame = nullptr;
			}
		}

		bool SoftwareDecodedFrame::operator <(const DecodedFrame& reference) {
			auto refFrame = dynamic_cast<const SoftwareDecodedFrame*>(&reference);
			if (!refFrame) {
				throw std::runtime_error("Provided incompatible class implementation");
			}
			return m_Frame->pkt_pts < refFrame->m_Frame->pkt_pts;
		}

		AVFrame* SoftwareDecodedFrame::rawFrame() {
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
						auto ret = avcodec_open2(m_Decoder, h262, 0);
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
				avcodec_close(m_Decoder);
				av_free(m_Decoder);
			}
		}

		size_t SoftwareDecoderContext::decodeFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata, const DecodedFrameRef& outFrame) {
			auto sFrame = dynamic_cast<SoftwareDecodedFrame*>(outFrame.get());
			if (!sFrame) {
				throw std::runtime_error("Provided incompatible class implementation");
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
			int nres = avcodec_decode_video2(m_Decoder, sFrame->rawFrame(), &gotOutput, &decodePacket);
			if (nres < 0) {
				throw std::runtime_error("error decompressing");
			}

			if (nres) {
				if (gotOutput) {
					sFrame->rawFrame()->pts = sFrame->rawFrame()->pkt_pts;
					sFrame->rawFrame()->pict_type = AV_PICTURE_TYPE_S;
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
				m_Encoder = avcodec_alloc_context3(h264);
				if (m_Encoder) {
					m_Encoder->width = sFrame->rawFrame()->width;
					m_Encoder->height = sFrame->rawFrame()->height;
					m_Encoder->sample_aspect_ratio = { 1, 1 };
					m_Encoder->pix_fmt = AV_PIX_FMT_YUV420P;
					m_Encoder->bit_rate = 1024;
					m_Encoder->codec_id = AV_CODEC_ID_H264;
					m_Encoder->codec_type = AVMEDIA_TYPE_VIDEO;
					m_Encoder->time_base.num = 1001;
					m_Encoder->time_base.den = 60000;
					m_Encoder->flags |= CODEC_FLAG_CLOSED_GOP;

					auto ret = avcodec_open2(m_Encoder, h264, nullptr);
					if (ret != 0) {
						av_free(m_Encoder);
						m_Encoder = nullptr;
						throw std::runtime_error("Can not initialise H.264 decoder");
					}
				} else {
					throw std::runtime_error("Can not allocate H.264 decoder context");
				}
			} else {
				throw std::runtime_error("Can not find H.264 decoder");
			}
		}

		SoftwareEncoderContext::~SoftwareEncoderContext() {
			if (m_Encoder) {
				avcodec_close(m_Encoder);
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
			auto ret = avcodec_encode_video2(m_Encoder, &encodePacket, sFrame->rawFrame(), &gotOutput);
			if (!ret && gotOutput && m_Encoder->coded_frame) {
				m_Encoder->coded_frame->pts = encodePacket.pts;
				m_Encoder->coded_frame->key_frame = !!(encodePacket.flags & AV_PKT_FLAG_KEY);
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

#endif
	}
}

