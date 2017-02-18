#pragma once
#include <capture/tsSource.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * File Transport Stream source
		 */
		class FileSource : public TSSource {
		public:
			/**
			 * Create file source
			 * @param[in] fileName File source name
			 */
			FileSource(const String& fileName);
			~FileSource() override;

		protected:
			size_t readDataInto(uint8_t* buffer, size_t size) override;
			void doStart() override;
			void doStop() override;
		private:
			String m_FileName;
			FILE* m_File;
			size_t m_FileSize;
		};

	}

}