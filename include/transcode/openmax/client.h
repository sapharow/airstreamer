#pragma once
#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "bcm_host.h"
#include "ilclient.h"

#ifdef __cplusplus
}
#endif

namespace fp {
	namespace omx {
		
		class Client : public std::enable_shared_from_this<Client> {
		public:
			Client();
			virtual ~Client();

			ComponentRef createDecoderComponent(bool enableInputBuffers = false, bool enableOutputBuffers = false);
			ComponentRef createEncoderComponent(bool enableInputBuffers = false, bool enableOutputBuffers = false);
			ComponentRef createClockComponent(bool enableInputBuffers = false, bool enableOutputBuffers = false);
			ComponentRef createSchedulerComponent(bool enableInputBuffers = false, bool enableOutputBuffers = false);

			/**
			 * When OMX_EventPortSettingsChanged event occurs this function triggers. 
			 */
			virtual void onPortSettingsChanged(const ComponentRef& component, uint32_t port) {}

			/**
			 * When OMX_EventBufferFlag received with OMX_BUFFERFLAG_EOS set this function triggers. 
			 */
			virtual void onEOS(const ComponentRef& component, uint32_t port) {}

			/**
			 * When OMX_EventError event occurs this function triggers. 
			 */
			virtual void onError(const ComponentRef& component, uint32_t errorCode) {}

			/**
			 * When OMX_EventParamOrConfigChanged event occurs this function triggers. 
			 * The user may then use an getConfig call with the index as specified to retrieve the updated information.
			 */
			virtual void onConfigurationChanged(const ComponentRef& component, uint32_t index) {}

			/**
			 * When a buffer passed to an output port using the fillBuffer call is returned with this callback.
			 * The user may then use the getOutputBuffer to retrieve the buffer
			 */
			virtual void onFillBufferDone(const ComponentRef& component) {}

			/**
			 * When a buffer passed to an input port using the emptyBuffer call is returned with this callback.
			 * The user may then use the getInputBuffer to retrieve the buffer
			 */
			virtual void onEmptyBufferDone(const ComponentRef& component) {}

		private:
			ComponentRef createComponent(const String& name, 
			                             const std::set<int>& iPorts,
			                             const std::set<int>& oPorts,
			                             bool enableInputBuffers, 
			                             bool enableOutputBuffers);

			static void omxPortSettingsCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data);
			static void omxEOSCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data);
			static void omxErrorCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data);
			static void omxConfChangedCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data);
			static void omxFillBufferDoneCallback(void *userdata, COMPONENT_T *comp);
			static void omxEmptyBufferDoneCallback(void *userdata, COMPONENT_T *comp);


			ILCLIENT_T* m_Client = nullptr;
			std::map<COMPONENT_T*, ComponentWeak> m_Components;
			std::mutex m_Mutex;
		};
	}
}