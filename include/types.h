#pragma once
#include <memory>
#include <string>
#include <vector>

namespace fp {

#define DECLARE_CLASS(c) class c; \
	typedef std::shared_ptr<c> c ## Ref; \
	typedef std::weak_ptr<c> c ## Weak;

#define DECLARE_STRUCT(c) struct c; \
	typedef std::shared_ptr<c> c ## Ref; \
	typedef std::weak_ptr<c> c ## Weak;

	/**
	 * Forward declarations
	 */
	namespace cap {
		DECLARE_CLASS(TSSource);
		DECLARE_CLASS(FileSource);
		DECLARE_CLASS(DVBSource);
		DECLARE_CLASS(ProgramReceiver);
		DECLARE_STRUCT(TSPayload);
	}

	typedef std::string String;
}
