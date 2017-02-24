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

		class Component : public std::enable_shared_from_this<Component> {
		public:
			Component(const ClientRef& client,
			          COMPONENT_T* component,
			          const String& name, 
			          const std::set<int>& inputPorts,
			          const std::set<int>& outputPorts);

			virtual ~Component() = default;

			/**
			 * Change component's state
			 */
			void changeState(OMX_STATETYPE state);
			OMX_STATETYPE state();

			/**
			 * Remove events
			 */
			bool removeEventPortSettingsChanged(int port);

			/**
			 * Set/Get parameters
			 */
			void setParameter(OMX_INDEXTYPE index, const void* parameter);
			void getParameter(OMX_INDEXTYPE index, void* parameter);

			/**
			 * Set/Get configuration
			 */
			void setConfig(OMX_INDEXTYPE index, const void* parameter);
			void getConfig(OMX_INDEXTYPE index, void* parameter);

			/**
			 * Get configuration
			 */
			void getConfig(OMX_INDEXTYPE index);

			/**
			 * Get raw component
			 */
			COMPONENT_T* comp() const;

			/**
			 * Get component name
			 */
			const String& name() const;

			/**
			 * Enable particular port
			 */
			void enablePort(int port);

			/**
			 * Disable particular port
			 */
			void disablePort(int port);

			/**
			 * Enable particular port's buffers
			 */
			void enablePortBuffers(int port);

			/**
			 * Disable particular port's buffers
			 */
			void disablePortBuffers(int port);

			/**
			 * Disable particular port's buffers
			 */
			void disablePortBuffers();

			/**
			 * Get input buffer of particular port
			 */
			OMX_BUFFERHEADERTYPE* getInputBuffer(int port, bool bBlocking);

			/**
			 * Get output buffer of particular port
			 */
			OMX_BUFFERHEADERTYPE* getOutputBuffer(int port, bool bBlocking);

			/**
			 * Send a buffer full of data to an input port of a component.
			 * The buffer will be emptied by the component and returned to the 
			 * application via the EmptyBufferDone call back. 
			 * This is a non-blocking call in that the component will record the buffer
			 * and return immediately and then empty the buffer, later, at the proper time.
			 */
			void emptyBuffer(OMX_BUFFERHEADERTYPE* buffer);

			/**
			 * Send an empty buffer to an output port of a component. The buffer will be 
			 * filled by the component and returned to the application via the FillBufferDone call back.
			 * This is a non-blocking call in that the component will record the buffer
			 * and return immediately and then fill the buffer, later, at the proper time.
			 */
			void fillBuffer(OMX_BUFFERHEADERTYPE* buffer);

			/**
			 * Allocate new buffer for specific port
			 * @param[in] port Port to attach this buffer to
			 * @param[in] size Buffer size
			 */
			BufferRef allocateBuffer(uint32_t port, uint32_t size);

		private:
			ClientRef    m_Client;
			COMPONENT_T* m_Component = nullptr;
			String       m_Name;
			std::set<int> m_EnabledPorts;
			std::set<int> m_InputPorts;
			std::set<int> m_OutputPorts;
		};
	}
}