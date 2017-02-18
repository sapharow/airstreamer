// Common
#include <stream.h>

// Capture
#include <capture/dvbSource.h>
#include <capture/fileSource.h>
#include <capture/program.h>

// Transcode
#include <transcode/transcoder.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0

class MyEncodedStream : public fp::Stream {
public:
	MyEncodedStream(uint32_t id, Type type, bool sync, uint32_t lang)
	: fp::Stream(id, type, sync, lang)
	{
		// Create stream
		char buffer[256];
		memset(buffer, 0, 256);
		switch (type) {
			case Stream::Type::Video_H261:
				sprintf(buffer, "enc_video_%04x.mp1", id);
				break;
			case Stream::Type::Video_H262:
				sprintf(buffer, "enc_video_%04x.mp2", id);
				break;
			case Stream::Type::Video_H264:
				sprintf(buffer, "enc_video_%04x.mp2", id);
				break;
			case Stream::Type::Audio_11172_2:
				sprintf(buffer, "enc_audio_%04x.mp1", id);
				break;
			case Stream::Type::Audio_13818_2:
				sprintf(buffer, "enc_audio_%04x.mp2", id);
				break;
			case Stream::Type::Audio_AC3:
				sprintf(buffer, "enc_audio_%04x.ac3", id);
				break;
			default:
				sprintf(buffer, "enc_other_%04x.ts", id);
			break;
		}

		m_File = fopen(buffer, "wb");
	}

	void supplyData(const uint8_t* data, size_t size, Metadata* metadata) override {
		if (m_File) {
			fwrite(data, 1, size, m_File);
		}
	}
private:
	FILE* m_File = nullptr;

};

class MyStream : public fp::Stream {
public:
	MyStream(uint32_t id, Type type, bool sync, uint32_t lang)
	: fp::Stream(id, type, sync, lang)
	{
		// Create transcoder
		m_Output = std::make_shared<MyEncodedStream>(id, type, sync, lang);
		m_Transcoder = std::make_shared<fp::trans::Transcoder>(m_Output);

		// Create stream
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
		// Supply stream data
		m_Transcoder->supplyFrame(data, size);

		if (m_File) {
			fwrite(data, 1, size, m_File);
		}
	}
private:
	FILE* m_File = nullptr;
	fp::trans::TranscoderRef m_Transcoder;
	fp::StreamRef m_Output;
};

class MySource : public fp::cap::FileSource {
public:
	MySource(const fp::String& fname) : fp::cap::FileSource(fname) {}
//	MySource(const fp::String& fname) : fp::cap::DVBSource(0,0,0) {}
	fp::StreamRef createStream(uint32_t id, fp::Stream::Type type, bool sync, uint32_t lang) override {
		return std::make_shared<MyStream>(id, type, sync, lang);
	}	
	bool programSpawned(const fp::cap::ProgramRef& program) {
		printf("New program found %u\n", program->id());
		return true;
	}
};


int main(int argc, char **argv)
{
//	fp::cap::FileSource src("mpt-smart-travels-classical-clip.dvb");
	MySource src("football.dvb");
//	fp::cap::DVBSource src(DVB_ADAPTER, DVB_FRONTEND, DVB_DEMUX);

	src.start();
	while (src.running()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    return 0;
}