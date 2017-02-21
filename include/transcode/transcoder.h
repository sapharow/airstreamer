#pragma once
#include <types.h>
#include <stream.h>

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
			Transcoder(StreamType inputType, const StreamRef& output);
			virtual ~Transcoder() = default;

			/**
			 * Initialise transcoder
			 */
			virtual void init() = 0;

			/**
			 * Supply frame to transcoder in decode order
			 */
			virtual void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*);

			/**
			 * Retrieve assigned output
			 */
			const StreamRef& output() const;

			/**
			 * Retrieve input stream type
			 */
			StreamType inputType() const;

			/**
			 * When stream is changed, transcoder shall be reset
			 */
			virtual void reset() = 0;

		private:
			StreamRef m_Output;
			StreamType m_InputType;
		};

	}
}
