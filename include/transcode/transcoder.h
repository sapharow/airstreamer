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
			Transcoder(Stream::Type inputType, const StreamRef& output);
			virtual ~Transcoder() = default;

			/**
			 * Supply frame to transcoder
			 */
			virtual void supplyFrame(const uint8_t* data, size_t size, Stream::Metadata*);

			/**
			 * Retrieve assigned output
			 */
			const StreamRef& output() const;

			/**
			 * Retrieve input stream type
			 */
			Stream::Type inputType() const;

		private:
			StreamRef m_Output;
			Stream::Type m_InputType;
		};

	}
}
