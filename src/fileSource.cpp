#include <fileSource.h>
#include <vector>

namespace fp {
	namespace cap {

		FileSource::FileSource(const String& fileName) 
		: m_FileName(fileName)
		{
		}

		FileSource::~FileSource() {
			stop();
			if (m_File) {
				fclose(m_File);
			}
		}

		size_t FileSource::readDataInto(uint8_t* buffer, size_t size) {
			if (ferror(m_File)) {
				throw std::runtime_error("Error reading from file");
			}
			if (feof(m_File)) {
				return 0;
			} else {
				return fread(buffer, 1, (size / 188) * 188, m_File);
			}
		}

		void FileSource::doStart() {
			m_File = fopen(m_FileName.c_str(), "rb");
			if (!m_File) {
				throw std::runtime_error("Can not open file");
			}
			fseek(m_File, 0, SEEK_END);
			m_FileSize = ftell(m_File);
			fseek(m_File, 0, SEEK_SET);

			// seek to position with 0x47
			std::vector<uint8_t> tsPacket;
			tsPacket.resize(std::min<size_t>(m_FileSize, 1024));
			size_t nRead = fread(tsPacket.data(), 1, tsPacket.size(), m_File);
			tsPacket.resize(nRead);
			size_t offset = -1;
			for (size_t i=0; i<tsPacket.size(); i++) {
				if (tsPacket[i] == 0x47 && (i + 188 < tsPacket.size()) && (tsPacket[i+188] == 0x47)) {
					offset = i;
					break;
				}
			}
			fseek(m_File, 0, SEEK_SET);
			if (offset == (size_t)-1) {
				// Wrong file
				throw std::runtime_error("File has incorrect format");
			}
			fseek(m_File, offset, SEEK_SET);
		}

		void FileSource::doStop() {
			// nothing
			if (m_File) {
				fclose(m_File);
				m_File = nullptr;
				m_FileSize = 0;
			}
		}

	}
}