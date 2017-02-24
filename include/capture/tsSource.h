#pragma once
#include <types.h>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_set>
#include <audioStream.h>
#include <videoStream.h>

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

		protected:

			/**
			 * Called when program is spawned and attached to this source
			 * Once spawned its streams will continuously receive payload
			 * @param[in] program Program created
			 * @return true if this program is required and false otherwise
			 */
			virtual bool programSpawned(const ProgramRef& program) { return true; }

			/**
			 * Called when source needs to create audio stream
			 * @param[in] meta Stream metadata (nonnull)
			 * @return created stream. Can return nullptr if this stream is not required
			 */
			virtual AudioStreamRef createAudioStream(AudioStreamMeta* meta) = 0;

			/**
			 * Called when source needs to create video stream
			 * @param[in] meta Stream metadata (nonnull)
			 * @return created stream. Can return nullptr if this stream is not required
			 */
			virtual VideoStreamRef createVideoStream(VideoStreamMeta* meta) = 0;

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

			std::shared_ptr<std::thread> m_Thread;
			std::unordered_set<uint32_t> m_PMTs;
			static void mainLoop(TSSource* thiz);
		};

	}

}
