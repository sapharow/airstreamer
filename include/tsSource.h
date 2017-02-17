#pragma once
#include <types.h>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_set>
#include <stream.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * Abstract Transport Stream source
		 */
		class TSSource {
		public:
			TSSource();
			virtual ~TSSource();

			/**
			 * Start capturing from source
			 */
			void start();

			/**
			 * Stop capturing from source
			 */
			void stop();

			/**
			 * Check capture status
			 */
			bool running();

			/**
			 * Set user-defined stream provider
			 */
			void setStreamProvider(StreamProvider sp);

		protected:
			/**
			 * Read transport stream data into memory buffer
			 * @param[in] buffer Destination buffer pointer
			 * @param[in] size Destination buffer size
			 * @return number of bytes read. Zero means EOS reached.
			 *
			 * if reading error occurs can throw std::runtime_error
			 */
			virtual size_t readDataInto(uint8_t* buffer, size_t size) = 0;

			/**
			 * Start capturing from source
			 */
			virtual void doStart() = 0;

			/**
			 * Stop capturing from source
			 */
			virtual void doStop() = 0;

		private:
			std::recursive_mutex m_Mutex;
			bool m_Started = false;
			bool m_ThreadFinished = false;
			StreamProvider m_StreamProvider;

			std::shared_ptr<std::thread> m_Thread;
			std::unordered_set<uint32_t> m_PMTs;
			static void mainLoop(TSSource* thiz);
		};

	}

}
