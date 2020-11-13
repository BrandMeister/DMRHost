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

template<typename... Args>
std::string string_format(const char* fmt, Args... args)
{
	size_t size = snprintf(nullptr, 0, fmt, args...);
	std::string buf;
	buf.reserve(size + 1);
	buf.resize(size);
	snprintf(&buf[0], size + 1, fmt, args...);
	return buf;
}

void CNetDisplay::write(const std::string& data)
{
	if (m_socket)
		m_socket->write((unsigned char*)data.c_str(), data.size(), m_addr, m_addrLen);
}

void CNetDisplay::setIdleInt()
{
	std::string data = "setIdle";
	write(data);
}

void CNetDisplay::setErrorInt(const char* text)
{
	std::string data = string_format("setError %s", text);
	write(data);
}

void CNetDisplay::setQuitInt()
{
	std::string data = "setQuit";
	write(data);
}

void CNetDisplay::writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type)
{
	std::string data = string_format("writeDMR %u %s %u %s %s", slotNo, src.c_str(), group, dst.c_str(), type);
	write(data);
}

void CNetDisplay::writeDMRRSSIInt(unsigned int slotNo, unsigned char rssi)
{
// TODO. meh
//	std::string data = string_format("writeDMRRSSI %u %s", slotNo, rssi);
//	write(data);
}

void CNetDisplay::writeDMRTAInt(unsigned int slotNo, unsigned char* talkerAlias, const char* type)
{
	std::string data = string_format("writeDMRTA %u %s %s", slotNo, type, talkerAlias);
	write(data);
}

void CNetDisplay::writeDMRBERInt(unsigned int slotNo, float ber)
{
	std::string data = string_format("writeDMRBER %u %f", slotNo, ber);
	write(data);
}

void CNetDisplay::clearDMRInt(unsigned int slotNo)
{
	std::string data = string_format("clearDMR %u", slotNo);
	write(data);
}

void CNetDisplay::writePOCSAGInt(uint32_t ric, const std::string& message)
{
	std::string data = string_format("writePOCSAG %u %s", ric, message.c_str());
	write(data);
}

void CNetDisplay::clearPOCSAGInt()
{
	std::string data = "clearPOCSAG";
	write(data);
}

void CNetDisplay::writeCWInt()
{
	std::string data = "writeCW";
	write(data);
}

void CNetDisplay::clearCWInt()
{
	std::string data = "clearCW";
	write(data);
}

void CNetDisplay::close()
{
	std::string data = "close";
	write(data);
}
