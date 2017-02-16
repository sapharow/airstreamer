#pragma once
#include <types.h>
#include <tsSource.h>

namespace fp {

	/**
	 * AirStream opens DVB device and gets transport stream out of it
	 */
	class AirStream {
	public:
		AirStream();

		// Callbacks

		/**
		 * Retrieved program table
		 */
//		virtual void gotPAT() = 0;

	private:
		DECLARE_STRUCT(Private);
		PrivateRef m_Private;
	};

	/**
	 * Elementary stream 
	 */
	class ElementaryStream {
	public:
		ElementaryStream(uint32_t pid);
	};

}