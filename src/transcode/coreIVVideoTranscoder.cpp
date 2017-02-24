#include <transcode/coreIVVideoTranscoder.h>
#include <transcode/openmax/tunnel.h>
#include <transcode/openmax/client.h>
#include <transcode/openmax/component.h>
#include <transcode/openmax/buffer.h>
#include <stream.h>
#include <thread>

#include <IL/OMX_Core.h>

#define WIDTH     640
#define PITCH     ((WIDTH+31)&~31)
#define HEIGHT    ((WIDTH)*9/16)
#define HEIGHT16  ((HEIGHT+15)&~15)
#define SIZE      ((WIDTH * HEIGHT16 * 3)/2)

namespace fp {
	String getString(const char* fmt ...) {
		va_list args;
		va_start(args, fmt);

		String message;
		int nChars = vsnprintf(nullptr, 0, fmt, args);
		message.resize(nChars);
		vsnprintf((char*)message.c_str(), message.size()+1, fmt, args);
		va_end(args);
		return message;
	}

	namespace trans {

		// CoreIVDecodedFrame /////////////////////////////////////////////////////////////////////////////

		bool CoreIVDecodedFrame::operator <(const DecodedFrame& reference) {
			return false;
		}

		// CoreIVDecoderContext ///////////////////////////////////////////////////////////////////////////

		CoreIVDecoderContext::CoreIVDecoderContext() 
		{ }

		size_t CoreIVDecoderContext::decodeFrame(const uint8_t* data, size_t size, Stream::Metadata* metadata, const DecodedFrameRef& outFrame) {
			return 0;
		}

		// CoreIVEncoderContext ///////////////////////////////////////////////////////////////////////////

		size_t CoreIVEncoderContext::encodeFrame(const DecodedFrameRef& frame, std::vector<uint8_t>& data) {
			return 0;
		}

		// CoreIVVideoTranscoder //////////////////////////////////////////////////////////////////////////

		CoreIVVideoTranscoder::CoreIVVideoTranscoder(StreamType inputType, const VideoStreamRef& output) 
		: VideoTranscoder(inputType, output)
		{ 
			m_File = fopen("test.h264", "wb");
		}

		static String getCompressionFormatStr(OMX_VIDEO_CODINGTYPE coding) {
			switch (coding) {
				case OMX_VIDEO_CodingUnused: return "Unused";
				case OMX_VIDEO_CodingAutoDetect: return "Autodetect";
				case OMX_VIDEO_CodingMPEG2: return "MPEG2";
				case OMX_VIDEO_CodingH263: return "H263";
				case OMX_VIDEO_CodingMPEG4: return "MPEG4";
				case OMX_VIDEO_CodingWMV: return "WMV";
				case OMX_VIDEO_CodingRV: return "RV";
				case OMX_VIDEO_CodingAVC: return "AVC";
				case OMX_VIDEO_CodingMJPEG: return "MJPEG";
				case OMX_VIDEO_CodingKhronosExtensions: return "KhronosExtensions";
				case OMX_VIDEO_CodingVendorStartUnused: return "VendorStartUnused";
				case OMX_VIDEO_CodingVP6: return "VP6";
				case OMX_VIDEO_CodingVP7: return "VP7";
				case OMX_VIDEO_CodingVP8: return "VP8";
				case OMX_VIDEO_CodingYUV: return "YUV";
				case OMX_VIDEO_CodingSorenson: return "Sorenson";
				case OMX_VIDEO_CodingTheora: return "Theora";
				case OMX_VIDEO_CodingMVC: return "MVC";
				default:  return getString("Unknown (%d)", (int)coding);
			}
		}

		static String getColorFormatStr(OMX_COLOR_FORMATTYPE color) {
			switch (color) {
				case OMX_COLOR_FormatUnused: return "Unused";
				case OMX_COLOR_FormatMonochrome: return "Monochrome";
				case OMX_COLOR_Format8bitRGB332: return "8 bit RGB 332";
				case OMX_COLOR_Format12bitRGB444: return "12 bit RGB 444";
				case OMX_COLOR_Format16bitARGB4444: return "16 bit ARGB 4444";
				case OMX_COLOR_Format16bitARGB1555: return "16 bit ARGB 1555";
				case OMX_COLOR_Format16bitRGB565: return "16 bit RGB 565";
				case OMX_COLOR_Format16bitBGR565: return "16 bit BGR 565";
				case OMX_COLOR_Format18bitRGB666: return "18 bit RGB 666";
				case OMX_COLOR_Format18bitARGB1665: return "18 bit ARGB 1665";
				case OMX_COLOR_Format19bitARGB1666: return "19 bit ARGB 1666"; 
				case OMX_COLOR_Format24bitRGB888: return "24 bit RGB 888";
				case OMX_COLOR_Format24bitBGR888: return "24 bit BGR 888";
				case OMX_COLOR_Format24bitARGB1887: return "24 bit ARGB 1887";
				case OMX_COLOR_Format25bitARGB1888: return "25 bit ARGB 1888";
				case OMX_COLOR_Format32bitBGRA8888: return "32 bit BGRA 8888";
				case OMX_COLOR_Format32bitARGB8888: return "32 bit ARGB 8888";
				case OMX_COLOR_FormatYUV411Planar: return "YUV 411 planar";
				case OMX_COLOR_FormatYUV411PackedPlanar: return "YUV 411 packed planar";
				case OMX_COLOR_FormatYUV420Planar: return "YUV 420 planar";
				case OMX_COLOR_FormatYUV420PackedPlanar: return "YUV 420 packed planar";
				case OMX_COLOR_FormatYUV420SemiPlanar: return "YUV 420 semiplanar";
				case OMX_COLOR_FormatYUV422Planar: return "YUV 422 planar";
				case OMX_COLOR_FormatYUV422PackedPlanar: return "YUV 422 packed planar";
				case OMX_COLOR_FormatYUV422SemiPlanar: return "YUV 422 semiplanar";
				case OMX_COLOR_FormatYCbYCr: return "YCbYCr";
				case OMX_COLOR_FormatYCrYCb: return "YCrYCb";
				case OMX_COLOR_FormatCbYCrY: return "CbYCrY";
				case OMX_COLOR_FormatCrYCbY: return "CrYCbY";
				case OMX_COLOR_FormatYUV444Interleaved: return "YUV 444 interleaved";
				case OMX_COLOR_FormatRawBayer8bit: return "Raw bayer 8 bit";
				case OMX_COLOR_FormatRawBayer10bit: return "Raw bayer 10 bit";
				case OMX_COLOR_FormatRawBayer8bitcompressed: return "Raw bayer 8 bit compressed";
				case OMX_COLOR_FormatL2: return "L2"; 
				case OMX_COLOR_FormatL4: return "L4"; 
				case OMX_COLOR_FormatL8: return "L8"; 
				case OMX_COLOR_FormatL16: return "L16"; 
				case OMX_COLOR_FormatL24: return "L24"; 
				case OMX_COLOR_FormatL32: return "L32";
				case OMX_COLOR_FormatYUV420PackedSemiPlanar: return "YUV 420 packed semiplanar";
				case OMX_COLOR_FormatYUV422PackedSemiPlanar: return "YUV 422 packed semiplanar";
				case OMX_COLOR_Format18BitBGR666: return "18 bit BGR 666";
				case OMX_COLOR_Format24BitARGB6666: return "24 bit ARGB 6666";
				case OMX_COLOR_Format24BitABGR6666: return "24 bit ABGR 6666";
				case OMX_COLOR_FormatKhronosExtensions: return "Reserved Khronos"; /**< Reserved region for introducing Khronos Standard Extensions */ 
				case OMX_COLOR_FormatVendorStartUnused: return "Reserved vendors"; /**< Reserved region for introducing Vendor Extensions */
				case OMX_COLOR_Format32bitABGR8888: return "32 bit ABGR 8888";
				case OMX_COLOR_Format8bitPalette: return "8 bit palette";
				case OMX_COLOR_FormatYUVUV128: return "YUVUV 128";
				case OMX_COLOR_FormatRawBayer12bit: return "Raw bayer 12 bit";
				case OMX_COLOR_FormatBRCMEGL: return "BRCM EGL";
				case OMX_COLOR_FormatBRCMOpaque: return "BRCM opaque";
				case OMX_COLOR_FormatYVU420PackedPlanar: return "YVU 420 packed planar";
				case OMX_COLOR_FormatYVU420PackedSemiPlanar: return "YVU 420 semiplanar";
				case OMX_COLOR_FormatRawBayer16bit: return "Raw bayer 16 bit";
				default:  return getString("Unknown (%d)", (int)color);
			}
		}

		static void printPortDefinitionType(OMX_PARAM_PORTDEFINITIONTYPE* def) {
			if (!def) {
				printf("(null)");
				return;
			}

			std::string domain;
			std::string formatStr;
			switch (def->eDomain) {
				case OMX_PortDomainAudio:
					domain = "Audio";
					formatStr = "\t\tNot implemented";
					break;
				case OMX_PortDomainVideo:
					domain = "Video";
					formatStr = getString("\t\tcMIMEType = %s,\n\t\tpNativeRender = %p,\n\t\tnFrameWidth = %u,\n\t\tnFrameHeight = %u,\n"
					                      "\t\tnStride = %d,\n\t\tnSliceHeight = %u,\n\t\tnBitrate = %u,\n\t\txFramerate = %u.%u,\n"
					                      "\t\tbFlagErrorConcealment = %s,\n\t\teCompressionFormat = %s,\n\t\teColorFormat = %s,\n"
					                      "\t\tpNativeWindow = %p",
					                      def->format.video.cMIMEType, 
					                      def->format.video.pNativeWindow,
					                      def->format.video.nFrameWidth,
					                      def->format.video.nFrameHeight,
					                      def->format.video.nStride,
					                      def->format.video.nSliceHeight,
					                      def->format.video.nBitrate,
					                      def->format.video.xFramerate >> 16,
					                      def->format.video.xFramerate & 0xFFFF,
					                      def->format.video.bFlagErrorConcealment ? "YES" : "NO",
					                      getCompressionFormatStr(def->format.video.eCompressionFormat).c_str(),
					                      getColorFormatStr(def->format.video.eColorFormat).c_str(),
					                      def->format.video.pNativeWindow);
					break;
				case OMX_PortDomainImage:
					domain = "Image";
					formatStr = "\t\tNot implemented";
					break;
				case OMX_PortDomainOther:
					domain = "Other";
					formatStr = "\t\tNot implemented";
					break;
				case OMX_PortDomainKhronosExtensions:
					domain = "KhronosExtensions";
					formatStr = "\t\tNot implemented";
					break;
				case OMX_PortDomainVendorStartUnused:
					domain = "VendorStartUnused";
					formatStr = "\t\tNot implemented";
					break;
				default:
					domain = "Unknown";
					formatStr = "\t\tNot implemented";
					break;
			}
			printf("OMX_PARAM_PORTDEFINITIONTYPE {\n\tnSize = %u,\n\tnVersion = %u.%u.%u.%u,\n\tnPortIndex = %u,\n"
			       "\teDir = %s,\n\tnBufferCountActual = %d,\n\tnBufferCountMin = %d,\n\tnBufferSize = %d,\n"
			       "\tbEnabled = %s,\n\tbPopulated = %s,\n\teDomain = %s,\n\tbBuffersContiguous = %s,\n"
			       "\tnBufferAlignment = %d,\n\tformat: {\n%s\n\t}\n}\n",
				def->nSize, 
				def->nVersion.s.nVersionMajor, def->nVersion.s.nVersionMinor, def->nVersion.s.nRevision, def->nVersion.s.nStep, 
				def->nPortIndex, def->eDir == OMX_DirInput ? "Input" : "Output",
				def->nBufferCountActual, def->nBufferCountMin, def->nBufferSize, def->bEnabled ? "YES" : "NO",
				def->bPopulated ? "YES" : "NO", domain.c_str(), def->bBuffersContiguous ? "YES" : "NO", 
				def->nBufferAlignment, formatStr.c_str());
		}

		void CoreIVVideoTranscoder::onPortSettingsChanged(const omx::ComponentRef& component, uint32_t port) {
			printf("Port %u of %s has changed\n", port, component->name().c_str());
			if ((component->name() == "video_decode") && (port == 131)) {
				if (!m_PipelineSet) {
					m_PipelineNeedsToSetup = true;
				}
			}
			if ((component->name() == "video_encode") && (port == 201)) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				m_Encoder->disablePortBuffers(201);
				m_Encoder->enablePortBuffers(201);
				m_EncoderSet = true;
			}
		}

		void CoreIVVideoTranscoder::onEOS(const omx::ComponentRef& component, uint32_t port) {
			printf("!!!! Callback onEOS (%s -> port %d)\n", component->name().c_str(), port);
		}

		void CoreIVVideoTranscoder::onError(const omx::ComponentRef& component, uint32_t errorCode) {
			printf("!!!! Callback onError (%s -> code %x)\n", component->name().c_str(), errorCode);
		}

		void CoreIVVideoTranscoder::onConfigurationChanged(const omx::ComponentRef& component, uint32_t index) {
			printf("!!!! Callback onConfigurationChanged (%s -> index %d)\n", component->name().c_str(), index);
		}

		void CoreIVVideoTranscoder::onFillBufferDone(const omx::ComponentRef& component, OMX_BUFFERHEADERTYPE* buf) {
			printf("!!!! Callback onFillBufferDone (%s)\n", component->name().c_str());
			if (buf) {
				if (buf->nFilledLen) {
					printf("\tencoder: filled output buffer with %u bytes of data\n", buf->nFilledLen);
					fwrite(buf->pBuffer, 1, buf->nFilledLen, m_File);
					buf->nFilledLen = 0;
				} else {
					printf("Got empty buffer!!!\n");
				}
			}
		}

		void CoreIVVideoTranscoder::onEmptyBufferDone(const omx::ComponentRef& component, OMX_BUFFERHEADERTYPE* pBuffer) {
			printf("!!!! Callback onEmptyBufferDone (%s)\n", component->name().c_str());
		}

		bool CoreIVVideoTranscoder::init() {
			// Destroy previous tunnel
			m_Decoder   = nullptr;
			m_Encoder   = nullptr;
			m_Clock     = nullptr;
//			m_Scheduler = nullptr;
			m_Tunnel    = nullptr;

			// Create components list
			m_Decoder   = createDecoderComponent(true, false);
			m_Encoder   = createEncoderComponent(false, true);
			m_Clock     = createClockComponent();
			m_Scheduler = createSchedulerComponent();

			// Create tunnel
			m_Tunnel = std::make_shared<omx::Tunnel>();

			// Configure clock
			OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
			memset(&cstate, 0, sizeof(cstate));
			cstate.nSize = sizeof(cstate);
			cstate.nVersion.nVersion = OMX_VERSION;
			cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
			cstate.nWaitMask = 1;
			m_Clock->setParameter(OMX_IndexConfigTimeClockState, &cstate);

			// Setup clock -> scheduler tunnel
			m_Tunnel->set(m_Clock, 80, m_Scheduler, 12);
			m_Clock->changeState(OMX_StateExecuting);

			// Setup decoder
			m_Decoder->changeState(OMX_StateIdle);

			OMX_VIDEO_PARAM_PORTFORMATTYPE format;
			memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
			format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
			format.nVersion.nVersion = OMX_VERSION;
			format.nPortIndex = 130;
			format.eCompressionFormat = OMX_VIDEO_CodingMPEG2;
			format.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
			m_Decoder->setParameter(OMX_IndexParamVideoPortFormat, &format);

			OMX_PARAM_DATAUNITTYPE dataUnit;
			memset(&dataUnit, 0, sizeof(OMX_PARAM_DATAUNITTYPE));
			dataUnit.nSize = sizeof(OMX_PARAM_DATAUNITTYPE);
			dataUnit.nVersion.nVersion = OMX_VERSION;
			dataUnit.nPortIndex = 130;
			dataUnit.eUnitType = OMX_DataUnitCodedPicture;
			dataUnit.eEncapsulationType = OMX_DataEncapsulationElementaryStream;
			m_Decoder->setParameter(OMX_IndexParamBrcmDataUnit, &dataUnit);

			OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE errorConceal;
			memset(&errorConceal, 0, sizeof(OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE));
			errorConceal.nSize = sizeof(OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE);
			errorConceal.nVersion.nVersion = OMX_VERSION;
			errorConceal.bStartWithValidFrame = OMX_FALSE;
			m_Decoder->setParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &errorConceal);

			m_Decoder->enablePortBuffers(130);
			m_Decoder->changeState(OMX_StateExecuting);

			// Wait for event
			printf("Transcoder initialised\n");
			return true;
		}

		void CoreIVVideoTranscoder::supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*) {
			if (!m_Decoder) {
				throw std::runtime_error("Decoder is not initialised");
			}
    		printf("--- Frame supply start ---------------\n");
#if 1
			if (m_PipelineNeedsToSetup) {
				if (!m_PipelineSet) {
					printf("Setup pipeline\n");

					// Get output settings from decoder
					OMX_PARAM_PORTDEFINITIONTYPE def;
					memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
					def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
					def.nVersion.nVersion = OMX_VERSION;
					def.nPortIndex = 131;
					m_Decoder->getParameter(OMX_IndexParamPortDefinition, &def);

					printf("Decoder reported the following output format:\n\tWidth = %u\n\tHeight = %u\n\tFramerate = %u.%u\n\tCompression = %s\n\tColorFormat = %s\n",
					       def.format.video.nFrameWidth, def.format.video.nFrameHeight, def.format.video.xFramerate >> 16, def.format.video.xFramerate & 0xffff,
					       getCompressionFormatStr(def.format.video.eCompressionFormat).c_str(), getColorFormatStr(def.format.video.eColorFormat).c_str());

//					m_Encoder->changeState(OMX_StateIdle);
					printf("Get encoder input format\n");
					// Get input settings from encoder
					OMX_VIDEO_PORTDEFINITIONTYPE decoderVideoDef = def.format.video;
					memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
					def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
					def.nVersion.nVersion = OMX_VERSION;
					def.nPortIndex = 200;
					m_Encoder->getParameter(OMX_IndexParamPortDefinition, &def);

					// Adjust encoder input settings and re-set back to encoder
					printf("Adjust encoder input format to match decoder output\n");
					def.format.video.nFrameWidth = decoderVideoDef.nFrameWidth;
					def.format.video.nFrameHeight = decoderVideoDef.nFrameHeight;
					def.format.video.nStride = decoderVideoDef.nStride;
					def.format.video.nSliceHeight = decoderVideoDef.nSliceHeight;
					def.format.video.xFramerate = decoderVideoDef.xFramerate;
					def.format.video.eCompressionFormat = decoderVideoDef.eCompressionFormat;
					def.format.video.eColorFormat = decoderVideoDef.eColorFormat;
					m_Encoder->setParameter(OMX_IndexParamPortDefinition, &def);

					// Setup encoder output
					printf("Setup encoder output format\n");
					OMX_VIDEO_PARAM_PORTFORMATTYPE format;
					memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
					format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
					format.nVersion.nVersion = OMX_VERSION;
					format.nPortIndex = 201;
					format.eCompressionFormat = OMX_VIDEO_CodingAVC;
					m_Encoder->setParameter(OMX_IndexParamVideoPortFormat, &format);

					// Setup codec
					printf("Setup encoder coding settings\n");
					OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
					// set current bitrate to 1Mbit
					memset(&bitrateType, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
					bitrateType.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
					bitrateType.nVersion.nVersion = OMX_VERSION;
					bitrateType.eControlRate = OMX_Video_ControlRateVariable;
					bitrateType.nTargetBitrate = 1000000;
					bitrateType.nPortIndex = 201;
					m_Encoder->setParameter(OMX_IndexParamVideoBitrate, &bitrateType);

					memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
					def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
					def.nVersion.nVersion = OMX_VERSION;
					def.nPortIndex = 201;
					m_Encoder->getParameter(OMX_IndexParamPortDefinition, &def);

#if 1
					m_Tunnel->set(m_Decoder, 131, m_Encoder, 200, 1000);
#else
					// Set Scheduler -> Encoder
					m_Tunnel->set(m_Decoder, 131, m_Scheduler, 10);
					m_Scheduler->changeState(OMX_StateExecuting);

					m_Tunnel->set(m_Scheduler, 11, m_Encoder, 200, 1000);
#endif
					m_Encoder->enablePortBuffers(201);
					m_Encoder->changeState(OMX_StateExecuting);

					m_PipelineSet = true;
				}
				m_PipelineNeedsToSetup = false;
			}
#endif

			printf("\tdecoder: getting input buffer\n");
			OMX_BUFFERHEADERTYPE* buf = m_Decoder->getInputBuffer(130, true);
			printf("\tdecoder: got input buffer %p\n", buf);
			if (buf) {
				if (buf->nAllocLen < size) {
					throw std::runtime_error("Buffer size is less than frame size");
				}

				memcpy(buf->pBuffer, data, size);

				buf->nFilledLen = size;
				buf->nOffset = 0;

				if (m_FirstPacket) {
					m_FirstPacket = false;
					buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
				} else {
					buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
				}

				printf("\tdecoder: emptying input buffer\n");
				m_Decoder->emptyBuffer(buf);
				printf("\tdecoder: emptied input buffer\n");
			}

			if (m_EncoderSet) {
				while ( (buf = m_Encoder->getOutputBuffer(201, false)) != nullptr ) {
					printf("\tencoder: filling output buffer\n");
					m_Encoder->fillBuffer(buf);
					printf("\tencoder: filled output buffer\n");
				}
			}

#if 1
#endif

#if 0
//			if (m_PipelineSet) 
			{
				printf("\tencoder: getting buffer\n");
				buf = m_Encoder->getOutputBuffer(201, false);
				if (buf) {
					printf("\tencoder: got buffer %p\n", buf);
					printf("\tencoder: filling buffer\n");
					m_Encoder->fillBuffer(buf);
					printf("\tencoder: filled buffer\n");
				} else {
					printf("\tencoder: got null buffer, skipped\n");
				}
				/*
				if (buf) {
				} else {
		    		printf("\tencoder: got null buffer\n");
				}
				*/
			}
#endif
			static int i = 0;
    		printf("... Supplied frame %d ................\n", i++);
		}

		CoreIVVideoTranscoder::~CoreIVVideoTranscoder() {

			OMX_BUFFERHEADERTYPE* buf = m_Decoder->getInputBuffer(130, true);
			buf->nFilledLen = 0;
			buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;
			m_Decoder->emptyBuffer(buf);
			if (m_EncoderSet) {
				while ( (buf = m_Encoder->getOutputBuffer(201, false)) != nullptr ) {
					printf("\tencoder: filling output buffer\n");
					m_Encoder->fillBuffer(buf);
					printf("\tencoder: filled output buffer\n");
				}
			}

			if (m_File) {
				fclose(m_File);
			}
			m_Tunnel = nullptr;
		}

		DecoderContextRef CoreIVVideoTranscoder::createDecoder() {
			return std::make_shared<CoreIVDecoderContext>();
		}

		EncoderContextRef CoreIVVideoTranscoder::createEncoder(uint32_t width, uint32_t height, uint32_t bitrate, uint32_t timebaseNum, uint32_t timebaseDen) {
			return nullptr;
		}

		DecodedFrameRef CoreIVVideoTranscoder::allocateFrame() {
			return std::make_shared<CoreIVDecodedFrame>();
		}

	}
}
