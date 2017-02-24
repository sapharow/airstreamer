#include <transcode/openmax/client.h>
#include <transcode/openmax/component.h>

namespace fp {
	namespace omx {

		struct OpenMAX {
			OpenMAX() {
				printf("Initialising BCM host\n");
				bcm_host_init();
				printf("Initialising OpenMAX IL host...");
				auto omxResult = OMX_Init();
				if (omxResult != OMX_ErrorNone) {
					bcm_host_deinit();
					printf("Can not initialise OpenMAX IL (%x)\n", omxResult);
					throw std::runtime_error("Can not initialise OpenMAX IL");
				}
				printf("Ok\n");
			}

			~OpenMAX() {
				OMX_Deinit();
				bcm_host_deinit();
			}
		};

		static OpenMAX g_OpenMAX;
		
		Client::Client() {
			m_Client = ilclient_init();
			if (!m_Client) {
				OMX_Deinit();
				bcm_host_deinit();
				throw std::runtime_error("Can not initialise client");
			}

			// Register callbacks
			ilclient_set_port_settings_callback(m_Client, omxPortSettingsCallback, this);
			ilclient_set_eos_callback(m_Client, omxEOSCallback, this);
			ilclient_set_error_callback(m_Client, omxErrorCallback, this);
			ilclient_set_configchanged_callback(m_Client, omxConfChangedCallback, this);
			ilclient_set_fill_buffer_done_callback(m_Client, omxFillBufferDoneCallback, this);
			ilclient_set_empty_buffer_done_callback(m_Client, omxEmptyBufferDoneCallback, this);
		}

		Client::~Client() {
			if (m_Client) {
				// Unregister callbacks
				ilclient_set_port_settings_callback(m_Client, nullptr, nullptr);
				ilclient_set_eos_callback(m_Client, nullptr, nullptr);
				ilclient_set_error_callback(m_Client, nullptr, nullptr);
				ilclient_set_configchanged_callback(m_Client, nullptr, nullptr);
				ilclient_set_fill_buffer_done_callback(m_Client, nullptr, nullptr);
				ilclient_set_empty_buffer_done_callback(m_Client, nullptr, nullptr);

				ilclient_destroy(m_Client);
				m_Client = nullptr;
			}
		}

		ComponentRef Client::createDecoderComponent(bool enableInputBuffers, bool enableOutputBuffers) {
			return createComponent("video_decode", {130}, {131}, enableInputBuffers, enableOutputBuffers);
		}

		ComponentRef Client::createClockComponent(bool enableInputBuffers, bool enableOutputBuffers) {
			return createComponent("clock", {}, {80,81,82,83,84,85}, enableInputBuffers, enableOutputBuffers);
		}

		ComponentRef Client::createSchedulerComponent(bool enableInputBuffers, bool enableOutputBuffers) {
			return createComponent("video_scheduler", {10,12}, {11}, enableInputBuffers, enableOutputBuffers);
		}

		ComponentRef Client::createEncoderComponent(bool enableInputBuffers, bool enableOutputBuffers) {
			return createComponent("video_encode", {200}, {201}, enableInputBuffers, enableOutputBuffers);
		}

		ComponentRef Client::createImageResizeComponent(bool enableInputBuffers, bool enableOutputBuffers) {
			return createComponent("resize", {60}, {61}, enableInputBuffers, enableOutputBuffers);
		}

		ComponentRef Client::createComponent(const String& name, 
		                                     const std::set<int>& iPorts,
		                                     const std::set<int>& oPorts,
		                                     bool enableInputBuffers, 
		                                     bool enableOutputBuffers)
		{
			COMPONENT_T* omxComponent = nullptr;

			unsigned flags = ILCLIENT_DISABLE_ALL_PORTS;
			if (enableInputBuffers) {
				flags |= ILCLIENT_ENABLE_INPUT_BUFFERS;
			}
			if (enableOutputBuffers) {
				flags |= ILCLIENT_ENABLE_OUTPUT_BUFFERS;
			}

			if (ilclient_create_component(m_Client, &omxComponent, (char*)name.c_str(), (ILCLIENT_CREATE_FLAGS_T)flags) == 0) {
				if (omxComponent) {
					auto component = std::make_shared<Component>(shared_from_this(), omxComponent, name, iPorts, oPorts);
					std::lock_guard<std::mutex> lock(m_Mutex);
					m_Components.emplace(omxComponent, component);
					return component;
				}
			}
			throw std::runtime_error(getString("Can not create component \"%s\"", name.c_str()));
		}

		void Client::omxPortSettingsCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onPortSettingsChanged(component, data);
				}
			}
		}

		void Client::omxEOSCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onEOS(component, data);
				}
			}
		}

		void Client::omxErrorCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onError(component, data);
				}
			}
		}

		void Client::omxConfChangedCallback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onConfigurationChanged(component, data);
				}
			}
		}

		void Client::omxFillBufferDoneCallback(void *userdata, COMPONENT_T *comp, OMX_BUFFERHEADERTYPE* pBuffer) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onFillBufferDone(component, pBuffer);
				}
			}
		}

		void Client::omxEmptyBufferDoneCallback(void *userdata, COMPONENT_T *comp, OMX_BUFFERHEADERTYPE* pBuffer) {
			Client* client = (Client*)userdata;
			std::unique_lock<std::mutex> lock(client->m_Mutex);
			auto it = client->m_Components.find(comp);
			if (it != client->m_Components.end()) {
				auto component = it->second.lock();
				if (component) {
					lock.unlock();
					client->onEmptyBufferDone(component, pBuffer);
				}
			}
		}

	}
}