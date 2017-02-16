#include <airStream.h>
#include <dvbSource.h>
#include <fileSource.h>
#include <program.h>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0

class MyProgram : public fp::cap::Program {
public:
	MyProgram(uint32_t pid)
	: fp::cap::Program(pid)
	{ }

	void supplyStreamPayload(const uint8_t* data, size_t size) override {
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "%u.ts", pid());

		FILE* f = fopen(buffer, "ab");
		if (f) {
			fwrite(data, 1, size, f);
			fclose(f);
		}
	}

};

int main(int argc, char **argv)
{
	fp::cap::FileSource src("test.dvb");
//	fp::cap::DVBSource src(DVB_ADAPTER, DVB_FRONTEND, DVB_DEMUX);
	src.setProgramProvider([](uint32_t pid)->fp::cap::ProgramRef{
		printf("Found new PID %u\n", pid);
		return std::make_shared<MyProgram>(pid);
	});

	src.start();
	while (src.running()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    return 0;
}