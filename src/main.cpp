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
	{ }
/*
	void supplyData(const uint8_t* data, size_t size) override {
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "%04x.raw", pid());

		FILE* f = fopen(buffer, "ab");
		if (f) {
			fwrite(data, 1, size, f);
			fclose(f);
		}
	}
*/
	void supplyData(/*uint64_t* pts, uint64_t* dts, */const uint8_t* data, size_t size) override {
		char buffer[256];
		memset(buffer, 0, 256);
		switch (type()) {
			case Stream::Type::Video_11172_2:
				sprintf(buffer, "video_%04x.ts", id());
				break;
			case Stream::Type::Video_13818_2:
				sprintf(buffer, "video_%04x.ts", id());
				break;
			case Stream::Type::Audio_11172_2:
				sprintf(buffer, "audio_%04x.ts", id());
				break;
			case Stream::Type::Audio_13818_2:
				sprintf(buffer, "audio_%04x.ts", id());
				break;
/*
			case Stream::Type::Audio:
				sprintf(buffer, "audio_%04x.ts", id());
				break;
			case Stream::Type::Video:
				sprintf(buffer, "video_%04x.mp2", id());
				break;
			case Stream::Type::Map:
				sprintf(buffer, "map_%04x.ts", id());
				break;
			case Stream::Type::Private2:
				sprintf(buffer, "private2_%04x.ts", id());
				break;
			case Stream::Type::ECM:
				sprintf(buffer, "ecm_%04x.ts", id());
				break;
			case Stream::Type::EMM:
				sprintf(buffer, "emm_%04x.ts", id());
				break;
			case Stream::Type::Directory:
				sprintf(buffer, "directory_%04x.ts", id());
				break;
			case Stream::Type::DSMCC:
				sprintf(buffer, "dsmcc_%04x.ts", id());
				break;
			case Stream::Type::H222E:
				sprintf(buffer, "h222e_%04x.ts", id());
				break;
*/				
			default:
				sprintf(buffer, "other_%04x.ts", id());
			break;
		}

		FILE* f = fopen(buffer, "ab");
		if (f) {
			fwrite(data, 1, size, f);
			fclose(f);
		}
	}

};

int main(int argc, char **argv)
{
	fp::cap::FileSource src("mpt-smart-travels-classical-clip.dvb");
//	fp::cap::FileSource src("football.dvb");
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