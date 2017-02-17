#include <airStream.h>
#include <dvbSource.h>
#include <fileSource.h>
#include <stream.h>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0

class MyStream : public fp::cap::Stream {
public:
	MyStream(uint32_t id, Type type, bool sync)
	: fp::cap::Stream(id, type, sync)
	{ 
		char buffer[256];
		memset(buffer, 0, 256);
		switch (type) {
			case Stream::Type::Video_H261:
				sprintf(buffer, "video_%04x.mp1", id);
				break;
			case Stream::Type::Video_H262:
				sprintf(buffer, "video_%04x.mp2", id);
				break;
			case Stream::Type::Audio_11172_2:
				sprintf(buffer, "audio_%04x.mp1", id);
				break;
			case Stream::Type::Audio_13818_2:
				sprintf(buffer, "audio_%04x.mp2", id);
				break;
			case Stream::Type::Audio_AC3:
				sprintf(buffer, "audio_%04x.ac3", id);
				break;
			default:
				sprintf(buffer, "other_%04x.ts", id);
			break;
		}

		m_File = fopen(buffer, "wb");
	}

	~MyStream() override {
		fclose(m_File);
	}

	void supplyData(const uint8_t* data, size_t size, Metadata* metadata) override {
		if (m_File) {
			fwrite(data, 1, size, m_File);
		}
	}
private:
	FILE* m_File = nullptr;

};

int main(int argc, char **argv)
{
//	fp::cap::FileSource src("mpt-smart-travels-classical-clip.dvb");
	fp::cap::FileSource src("football.dvb");
//	fp::cap::DVBSource src(DVB_ADAPTER, DVB_FRONTEND, DVB_DEMUX);
	src.setStreamProvider([](uint32_t id, fp::cap::Stream::Type type, bool sync)->fp::cap::StreamRef{
		return std::make_shared<MyStream>(id, type, sync);
	});

	src.start();
	while (src.running()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    return 0;
}