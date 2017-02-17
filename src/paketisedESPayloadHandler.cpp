#include "paketisedESPayloadHandler.h"
#include <libdvbv5/dvb-fe.h>
#include <stream.h>
#include "libdvbv5-patch/mpeg_pes.h"

namespace fp {
	namespace cap {

		PaketisedESPayloadHandler::PaketisedESPayloadHandler(StreamRef stream)
		: PaketisedPayloadHandler(stream)
		{
			m_PESTable = (dvb_mpeg_pes*)malloc(sizeof(dvb_mpeg_pes) + sizeof(dvb_mpeg_pes_optional));
		}

		PaketisedESPayloadHandler::~PaketisedESPayloadHandler() {
			free(m_PESTable);
		}

		void PaketisedESPayloadHandler::supplyStream(const StreamRef& stream) {
			if (dataSize() >= 3) {
				if ((data()[0] == 0) && (data()[1] == 0) && (data()[2] == 1)) {
					// Content is PES
					const ssize_t pesSize = dvb_mpeg_pes_init_patched(dvb_fe_dummy(), data(), dataSize(), (uint8_t*)m_PESTable);
					if (pesSize != -1) {
						// PES table is correct
						size_t nBytesToSupply = dataSize() - pesSize;
						if (nBytesToSupply) {
							stream->supplyData(
							                     /*
							                            (m_PESTable->optional->PTS_DTS & 0x2) ? &m_PESTable->optional->pts : nullptr,
							                            (m_PESTable->optional->PTS_DTS & 0x1) ? &m_PESTable->optional->dts : nullptr,
							                            */
							                            data() + pesSize, 
							                            nBytesToSupply);
						}
						return;
					}
				}
			}
			PaketisedPayloadHandler::supplyStream(stream);
		}

	}
}
