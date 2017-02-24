#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <set>
#include <map>
#include <mutex>

namespace fp {

#define nonnull __attribute__((nonnull))

#define DECLARE_CLASS(c) class c; \
	typedef std::shared_ptr<c> c ## Ref; \
	typedef std::weak_ptr<c> c ## Weak;

#define DECLARE_STRUCT(c) struct c; \
	typedef std::shared_ptr<c> c ## Ref; \
	typedef std::weak_ptr<c> c ## Weak;

	/**
	 * Forward declarations
	 */
	DECLARE_CLASS(Stream);
	DECLARE_CLASS(AudioStream);
	DECLARE_CLASS(VideoStream);

	namespace cap {
		DECLARE_CLASS(TSSource);
		DECLARE_CLASS(FileSource);
		DECLARE_CLASS(DVBSource);
		DECLARE_CLASS(Program);
	}

	namespace trans {
		DECLARE_CLASS(Transcoder);
		DECLARE_CLASS(SoftwareTranscoder);
	}

#ifdef OMX
	namespace omx {
		DECLARE_CLASS(Component);
		DECLARE_CLASS(Components);
		DECLARE_CLASS(Client);
		DECLARE_CLASS(Tunnel);
		DECLARE_CLASS(Buffer);
	}
#endif

	typedef std::string String;
	String getString(const char* fmt ...);
}
