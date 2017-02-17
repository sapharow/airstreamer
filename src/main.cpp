#include <airStream.h>
#include <dvbSource.h>
#include <fileSource.h>
#include <programReceiver.h>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0

class MyProgramReceiver : public fp::cap::ProgramReceiver {
public:
	MyProgramReceiver(uint32_t pid)
	: fp::cap::ProgramReceiver(pid)
	{ }

	void supplyPayload(const uint8_t* data, size_t size) override {
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "%u.raw", pid());

		FILE* f = fopen(buffer, "ab");
		if (f) {
			fwrite(data, 1, size, f);
			fclose(f);
		}
	}

	void supplyES(Stream stream, uint32_t streamId, uint64_t* pts, uint64_t* dts, const uint8_t* data, size_t size) override {
		char buffer[256];
		memset(buffer, 0, 256);
		switch (stream) {
			case Stream::Audio:
				sprintf(buffer, "audio_%u.ts", pid());
				break;
			case Stream::Video:
				sprintf(buffer, "video_%u.ts", pid());
				break;
			case Stream::Map:
				sprintf(buffer, "map_%u.ts", pid());
				break;
			case Stream::Private2:
				sprintf(buffer, "private2_%u.ts", pid());
				break;
			case Stream::ECM:
				sprintf(buffer, "ecm_%u.ts", pid());
				break;
			case Stream::EMM:
				sprintf(buffer, "emm_%u.ts", pid());
				break;
			case Stream::Directory:
				sprintf(buffer, "directory_%u.ts", pid());
				break;
			case Stream::DSMCC:
				sprintf(buffer, "dsmcc_%u.ts", pid());
				break;
			case Stream::H222E:
				sprintf(buffer, "h222e_%u.ts", pid());
				break;
			default:
				sprintf(buffer, "other_%u.ts", pid());
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
//	fp::cap::FileSource src("mpt-smart-travels-classical-clip.dvb");
	fp::cap::FileSource src("football.dvb");
//	fp::cap::DVBSource src(DVB_ADAPTER, DVB_FRONTEND, DVB_DEMUX);
	src.setProgramReceiverProvider([](uint32_t pid)->fp::cap::ProgramReceiverRef{
		return std::make_shared<MyProgramReceiver>(pid);
	});

	src.start();
	while (src.running()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    return 0;
}