#include <capture/fileSource.h>
#include <vector>
#include <memory.h>

namespace fp {
	namespace cap {

		FileSource::FileSource(const String& fileName, bool preload) 
		: m_FileName(fileName)
		, m_Preload(preload)
		{
		}

		FileSource::~FileSource() {
			stop();
			if (m_File) {
				fclose(m_File);
			}
		}

		size_t FileSource::readDataInto(uint8_t* buffer, size_t size) {
			if (m_File) {
				if (ferror(m_File)) {
					throw std::runtime_error("Error reading from file");
				}
				if (!feof(m_File)) {
					return fread(buffer, 1, (size / 188) * 188, m_File);
				}
			} else {
				if (m_Preload) {
					if (m_Content.size() > m_Offset) {
						size_t nBytesToRead = std::min<size_t>(m_Content.size() - m_Offset, size);
						memcpy(buffer, m_Content.data() + m_Offset, nBytesToRead);
						m_Offset += nBytesToRead;
						return nBytesToRead;
					}
				}
			}
			return 0;
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

			if (m_Preload) {
				m_Content.resize(m_FileSize - offset);
				fread(m_Content.data(), 1, m_Content.size(), m_File);
				fclose(m_File);
				m_File = nullptr;
			}
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