#pragma once
#include <transcode/videoTranscoder.h>
#include <transcode/openmax/client.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "bcm_host.h"
#include "ilclient.h"

#ifdef __cplusplus
}
#endif

namespace fp {
	namespace trans {

		class CoreIVDecodedFrame : public DecodedFrame {
		public:
			bool operator <(const DecodedFrame& reference) override;
		};

		class CoreIVDecoderContext : public DecoderContext {
		public:
			CoreIVDecoderContext();
			size_t decodeFrame(const uint8_t*, size_t, Stream::Metadata*, const DecodedFrameRef&) override;
		};

		class CoreIVEncoderContext : public EncoderContext {
		public:
			CoreIVEncoderContext();
			size_t encodeFrame(const DecodedFrameRef&, std::vector<uint8_t>&) override;
		};

		/**
		 * Transcoder class
		 */
		class CoreIVVideoTranscoder : public VideoTranscoder, public omx::Client {
		public:
			CoreIVVideoTranscoder(StreamType inputType, const VideoStreamRef& output);
			~CoreIVVideoTranscoder() override;
			bool init() override;
			void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) override;
	
		protected:
			// Overrides from VideoTranscoder
			DecoderContextRef createDecoder() override;
			EncoderContextRef createEncoder(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) override;
			DecodedFrameRef allocateFrame() override;

			// Overrides Client
			void onPortSettingsChanged(const omx::ComponentRef& component, uint32_t port) override;
			void onEOS(const omx::ComponentRef& component, uint32_t port) override;
			void onError(const omx::ComponentRef& component, uint32_t errorCode) override;
			void onConfigurationChanged(const omx::ComponentRef& component, uint32_t index) override;
			void onFillBufferDone(const omx::ComponentRef& component, OMX_BUFFERHEADERTYPE* pBuffer) override;
			void onEmptyBufferDone(const omx::ComponentRef& component, OMX_BUFFERHEADERTYPE* pBuffer) override;

		private:
			omx::TunnelRef m_Tunnel;
			omx::ComponentRef m_Decoder;
			omx::ComponentRef m_Encoder;
			bool m_PipelineSet = false;
			bool m_FirstPacket = false;
			bool m_PipelineNeedsToSetup = false;

			std::mutex m_Mutex;
			bool m_EncoderSet = false;
		};

	}
}
