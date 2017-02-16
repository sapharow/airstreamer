#pragma once
#include <types.h>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

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
			typedef std::function<ProgramRef(uint32_t pid)> ProgramProvider;
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
			 * Set program provider which will create program receiver for given PID
			 * @param[in] pp Custom program provider. Can be nullptr
			 */
			void setProgramProvider(ProgramProvider pp);

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
			ProgramProvider m_ProgramProvider;

			std::shared_ptr<std::thread> m_Thread;
			static void mainLoop(TSSource* thiz);
		};

	}

}
