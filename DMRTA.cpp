/*
*	Copyright (C) 2015,2016,2017,2018,2023 Jonathan Naylor, G4KLX
*	Copyright (C) 2018 by Shawn Chain, BG5HHP
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation; version 2 of the License.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*/

#include "DMRTA.h"
#include "Log.h"

#include <cstring>
#include <cassert>

CDMRTA::CDMRTA(unsigned int slotNo) :
m_slotNo(slotNo),
m_ta(),
m_buf()
{
}

CDMRTA::~CDMRTA()
{
}

bool CDMRTA::add(unsigned int blockId, const unsigned char* data, unsigned int len)
{
    assert(data != NULL);

    if (blockId > 3U) {
        // invalid block id
        reset();
        return false;
    }

    unsigned int offset = blockId * 7U;

    if (offset + len >= sizeof(m_buf)) {
        // buffer overflow
        reset();
        return false;
    }

    ::memcpy(m_buf + offset, data, len);

    return decodeTA();
}

const unsigned char* CDMRTA::get()
{
    return (unsigned char*)m_ta;
}

void CDMRTA::reset()
{
    ::memset(m_ta,  0x00U, sizeof(m_ta));
    ::memset(m_buf, 0x00U, sizeof(m_buf));
}

bool CDMRTA::decodeTA()
{
	unsigned int taFormat = (m_buf[0] >> 6U) & 0x03U;
	unsigned int taSize   = (m_buf[0] >> 1U) & 0x1FU;
	::strcpy(m_ta, "(could not decode)");

	switch (taFormat) {
		case 0U: {		// 7 bit
				::memset(m_ta, 0x00U, sizeof(m_ta));

				unsigned char* b = m_buf;
				unsigned int t1  = 0U;
				unsigned int t2  = 0U;
				unsigned char c  = 0U;

				for (unsigned int i = 0U; (i < 32U) && (t2 < taSize); i++) {
					for (int j = 7; j >= 0; j--) {
						c = (c << 1U) | (b[i] >> j);

						if (++t1 == 7U) {
							if (i > 0U)
								m_ta[t2++] = c & 0x7FU;

							t1 = 0U;
							c  = 0U;
						}
					}
				}

				m_ta[taSize] = 0;
			}
			break;

		case 1U:		// ISO 8 bit
		case 2U:		// UTF8
			::memcpy(m_ta, m_buf + 1U, sizeof(m_ta));
			break;

		case 3U: {		// UTF16 poor man's conversion
				unsigned int t2 = 0U;
				::memset(&m_ta, 0x00U, sizeof(m_ta));

				for (unsigned int i = 0U; (i < 15U) && (t2 < taSize); i++) {
					if (m_buf[2U * i + 1U] == 0)
						m_ta[t2++] = m_buf[2U * i + 2U];
					else
						m_ta[t2++] = '?';
				}

				m_ta[taSize] = 0;
			}
			break;
	}

	size_t taLen = ::strlen(m_ta);

	if (taLen == taSize)
		LogMessage("DMR Slot %u, Talker Alias \"%s\"", m_slotNo, m_ta);

	LogDebug("DMR Slot %u, Talker Alias (Data Format %u, Received %u/%u char): '%s'", m_slotNo, taFormat, taLen, taSize, m_ta);

	if (taLen > taSize) {
		if (taLen < 29U)
			::strcat(m_ta, " ?");
		else
			::strcpy(m_ta + 28U, " ?");
	}

	return taLen >= taSize;
}

