/*
 *   Copyright (C) 2016,2018,2020 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "NetDisplay.h"

#include "Log.h"

#include <string.h>

CNetDisplay::CNetDisplay(const std::string& address, unsigned int port) :
CDisplay(),
m_socket(NULL),
m_addressStr(address),
m_addr(),
m_addrLen(0U),
m_port(port)
{
}

CNetDisplay::~CNetDisplay()
{
}

bool CNetDisplay::open()
{
	LogInfo("Display, opening socket");

	if (CUDPSocket::lookup(m_addressStr, m_port, m_addr, m_addrLen) != 0) {
		LogError("Display, Unable to resolve the address");
		return 1;
	}

	m_socket = new CUDPSocket();
	int ret = m_socket->open(m_addr);
	if (!ret) {
		LogWarning("Display, Could not open socket, disabling");
		delete m_socket;
		m_socket = NULL;
	}

	return true;
}

void CNetDisplay::write(unsigned char* data, unsigned int length)
{
	if (m_socket)
		m_socket->write(data, length, m_addr, m_addrLen);
}

void CNetDisplay::setIdleInt()
{
	unsigned char data[1U];
	data[0] = 0x01;
	write(data, 1U);
}

void CNetDisplay::setErrorInt(const char* text)
{
	unsigned char data[100U];
	data[0]  = 0x02;

	uint8_t count = 0;
	for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
		data[2 + i] = text[i];

	data[1]  = count;

	write(data, 2 + count);
}

void CNetDisplay::setQuitInt()
{
	unsigned char data[1U];
	data[0] = 0x03;
	write(data, 1U);
}

void CNetDisplay::writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type)
{
	unsigned char data[12U];
	data[0]  = 0x04;
	data[1]  = slotNo;
        data[2]  = atoi(src.c_str()) >> 24;
        data[3]  = atoi(src.c_str()) >> 16;
        data[4]  = atoi(src.c_str()) >> 8;
        data[5]  = atoi(src.c_str()) >> 0;
	data[6]  = group;
        data[7]  = atoi(dst.c_str()) >> 24;
        data[8]  = atoi(dst.c_str()) >> 16;
        data[9]  = atoi(dst.c_str()) >> 8;
        data[10] = atoi(dst.c_str()) >> 0;
        data[11] = type[0];

	write(data, 12);
}

void CNetDisplay::writeDMRRSSIInt(unsigned int slotNo, unsigned char rssi)
{
	unsigned char data[100U];
	data[0]  = 0x05;
	data[1]  = slotNo;
	data[2]  = rssi;

	write(data, 3);
}

void CNetDisplay::writeDMRTAInt(unsigned int slotNo, unsigned char* talkerAlias, const char* type)
{
	unsigned char data[100U];
	data[0]  = 0x06;
	data[1]  = slotNo;
	data[2]  = type[0];

	uint8_t count = 0;
	for (uint8_t i = 0U; talkerAlias[i] != '\0'; i++, count++)
		data[4 + i] = talkerAlias[i];

	data[3]  = count;

	write(data, 4 + count);
}

void CNetDisplay::writeDMRBERInt(unsigned int slotNo, float ber)
{
	unsigned char data[100U];
	data[0]  = 0x07;
	data[1]  = slotNo;

	char _ber[10];
        _ber[0] = 0;
	snprintf(_ber, sizeof(_ber), "%f", ber);

        uint8_t count = 0;
        for (uint8_t i = 0U; _ber[i] != '\0'; i++, count++)
                data[3 + i] = _ber[i];

        data[2]  = count;

        write(data, 3 + count);
}

void CNetDisplay::clearDMRInt(unsigned int slotNo)
{
	unsigned char data[2U];
	data[0]  = 0x08;
	data[1]  = slotNo;

	write(data, 2);
}

void CNetDisplay::writePOCSAGInt(uint32_t ric, const std::string& message)
{
	unsigned char data[100U];
	data[0]  = 0x09;

        data[1]  = ric >> 24;
        data[2]  = ric >> 16;
        data[3]  = ric >> 8;
        data[4]  = ric >> 0;

	uint8_t count = 0;
	for (uint8_t i = 0U; message[i] != '\0'; i++, count++)
		data[6 + i] = message[i];

        data[5]  = count;

	write(data, 6 + count);
}

void CNetDisplay::clearPOCSAGInt()
{
	unsigned char data[1U];
	data[0] = 0x0A;
	write(data, 1U);
}

void CNetDisplay::writeCWInt()
{
	unsigned char data[1U];
	data[0] = 0x0B;
	write(data, 1U);
}

void CNetDisplay::clearCWInt()
{
	unsigned char data[1U];
	data[0] = 0x0C;
	write(data, 1U);
}

void CNetDisplay::close()
{
	unsigned char data[1U];
	data[0] = 0x0D;
	write(data, 1U);
}
