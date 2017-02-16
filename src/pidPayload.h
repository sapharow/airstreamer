#pragma once
#include <libdvbv5/mpeg_pes.h>
#include <types.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * PIDPayload is PID payload handler
		 */
		struct PIDPayload {
			/**
			 * Create PID payload handler class
			 */
			PIDPayload(ProgramRef _program, uint32_t _pid, bool splitAcrossPackets = true);
			~PIDPayload();

			/**
			 * Reset payload writer to start new payload
			 * @param[in] ctr New counter value
			 */
			void reset();

			/**
			 * Append data to payload
			 */
			void append(uint8_t* src, size_t size, size_t ctr);

			/**
			 * Retrieve data size
			 */
			size_t dataSize() const;

			/**
			 * Retrieve data
			 */
			uint8_t* data();

		private:
			size_t m_Counter = 0;
			bool   m_Error = true;
			bool   m_PayloadStart = false;
			uint32_t m_PID;
			dvb_mpeg_pes* m_PESTable;
			ProgramRef m_Program;
			bool m_SplitAcrossPackets;
			std::vector<uint8_t> m_Data;
			size_t m_DataSize = 0;
		};

	}
}