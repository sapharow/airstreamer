// Common
#include <videoStream.h>

// Capture
#include <capture/dvbSource.h>
#include <capture/fileSource.h>
#include <capture/program.h>

// Transcode
#include <transcode/softwareVideoTranscoder.h>
#include <transcode/coreIVVideoTranscoder.h>

#define DVB_ADAPTER   0
#define DVB_FRONTEND  0
#define DVB_DEMUX     0

class MyEncodedStream : public fp::VideoStream {
public:
	MyEncodedStream(fp::VideoStreamMeta* meta)
	: fp::VideoStream(meta)
	{
		uint32_t id = 0;
		if (meta) {
			id = meta->id;
		}
		// Create stream
		char buffer[256];
		memset(buffer, 0, 256);
		switch (meta->type) {
			case fp::StreamType::Video_H261:
				sprintf(buffer, "enc_video_%04x.mp1", id);
				break;
			case fp::StreamType::Video_H262:
				sprintf(buffer, "enc_video_%04x.mp2", id);
				break;
			case fp::StreamType::Video_H264:
				sprintf(buffer, "enc_video_%04x.h264", id);
				break;
			case fp::StreamType::Audio_11172_2:
				sprintf(buffer, "enc_audio_%04x.mp1", id);
				break;
			case fp::StreamType::Audio_13818_2:
				sprintf(buffer, "enc_audio_%04x.mp2", id);
				break;
			case fp::StreamType::Audio_AC3:
				sprintf(buffer, "enc_audio_%04x.ac3", id);
				break;
			default:
				sprintf(buffer, "enc_other_%04x.ts", id);
			break;
		}

		m_File = fopen(buffer, "wb");
	}

	void supplyFrame(const uint8_t* data, size_t size, Metadata* metadata) override {
		if (m_File) {
			fwrite(data, 1, size, m_File);
		}
	}
private:
	FILE* m_File = nullptr;

};

class MyVideoStream : public fp::VideoStream {
public:
	MyVideoStream(fp::VideoStreamMeta* meta)
	: fp::VideoStream(meta)
	{
		// Create transcoder
		fp::VideoStreamMeta outputMeta;
		outputMeta.id = id();
		outputMeta.type = fp::StreamType::Video_H264;
		outputMeta.sync = sync();
		outputMeta.width = width();
		outputMeta.height = height();

		m_Output = std::make_shared<MyEncodedStream>(&outputMeta);
#ifdef RPI
		m_Transcoder = std::make_shared<fp::trans::CoreIVVideoTranscoder>(type(), m_Output);
#else
		m_Transcoder = std::make_shared<fp::trans::SoftwareVideoTranscoder>(type(), m_Output);
#endif
		if (!m_Transcoder->init()) {
			printf("error\n");
		}

		// Create stream
		char buffer[256];
		memset(buffer, 0, 256);
		switch (type()) {
			case fp::StreamType::Video_H261:
				sprintf(buffer, "video_%04x.mp1", id());
				break;
			case fp::StreamType::Video_H262:
				sprintf(buffer, "video_%04x.mp2", id());
				break;
			case fp::StreamType::Audio_11172_2:
				sprintf(buffer, "audio_%04x.mp1", id());
				break;
			case fp::StreamType::Audio_13818_2:
				sprintf(buffer, "audio_%04x.mp2", id());
				break;
			case fp::StreamType::Audio_AC3:
				sprintf(buffer, "audio_%04x.ac3", id());
				break;
			default:
				sprintf(buffer, "other_%04x.ts", id());
			break;
		}

		m_File = fopen(buffer, "wb");

		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	~MyVideoStream() override {
		auto end = std::chrono::high_resolution_clock::now();
		auto msPassed = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_StartTime).count();
		printf("transcode speed = %.2f frames/sec\n", (float)m_NFrames * 1000.f / (float)msPassed);

		if (m_File) {
			fclose(m_File);
		}
	}

	void supplyFrame(const uint8_t* data, size_t size, Metadata* metadata) override {
		// Supply stream data
		if (m_Transcoder) {
			m_Transcoder->supplyFrame(data, size, metadata);
			m_NFrames++;
		}

		if (m_File) {
			fwrite(data, 1, size, m_File);
		}
	}

	uint64_t nFrames() const { return m_NFrames; }

private:
	FILE* m_File = nullptr;
	fp::trans::TranscoderRef m_Transcoder;
	fp::VideoStreamRef m_Output;
	uint64_t m_NFrames = 0;
	std::chrono::high_resolution_clock::time_point m_StartTime;
};

class MySource : public fp::cap::FileSource {
public:
	MySource(const fp::String& fname) : fp::cap::FileSource(fname, true) {}
//	MySource(const fp::String& fname) : fp::cap::DVBSource(0,0,0) {}

	fp::AudioStreamRef createAudioStream(fp::AudioStreamMeta* meta) override {
		// No audio is required now
		return nullptr;
	}

	fp::VideoStreamRef createVideoStream(fp::VideoStreamMeta* meta) override {
		static int i = 0;
		i++;
		if (i != 2) {
			return nullptr;
		}
		return std::make_shared<MyVideoStream>(meta);
	}

	bool programSpawned(const fp::cap::ProgramRef& program) override {
		printf("New program found %u\n", program->id());
		return true;
	}
};


int main(int argc, char **argv)
{
//	MySource src("mpt-smart-travels-classical-clip.dvb");
	MySource src("football.dvb");
//	fp::cap::DVBSource src(DVB_ADAPTER, DVB_FRONTEND, DVB_DEMUX);

	src.start();
	while (src.running()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    return 0;
}
