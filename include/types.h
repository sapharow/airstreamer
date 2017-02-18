#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>

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
	namespace cap {
		DECLARE_CLASS(TSSource);
		DECLARE_CLASS(FileSource);
		DECLARE_CLASS(DVBSource);
		DECLARE_STRUCT(Stream);
		DECLARE_STRUCT(Program);
	}

	typedef std::string String;
}
