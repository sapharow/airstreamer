#pragma once
#include <libdvbv5/mpeg_pes.h>
#include "paketisedPayloadHandler.h"
#include <types.h>

namespace fp {

	/**
	 * Capture classes
	 */
	namespace cap {

		/**
		 * Handler of paketised payload
		 */
		class PaketisedESPayloadHandler : public PaketisedPayloadHandler {
		public:
			PaketisedESPayloadHandler(StreamRef stream);
			virtual ~PaketisedESPayloadHandler();
		protected:
			void supplyStream(const StreamRef& stream) override;
		private:
			dvb_mpeg_pes* m_PESTable;
		};

	}
}