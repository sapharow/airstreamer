#pragma once
#include <types.h>

namespace fp {
	namespace trans {

		/**
		 * Transcoder class
		 */
		class Transcoder {
		public:

			/**
			 * Create transcoder with output
			 */
			Transcoder(const StreamRef& output);
			virtual ~Transcoder() = default;

			/**
			 * Supply frame to transcoder
			 */
			virtual void supplyFrame(const uint8_t* data, size_t size);

		private:
			StreamRef m_Output;
		};

	}
}
