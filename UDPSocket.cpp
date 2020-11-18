/*
 *   Copyright (C) 2006-2016,2020 by Jonathan Naylor G4KLX
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

#include "UDPSocket.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include "Log.h"

CUDPSocket::CUDPSocket(const std::string& address, unsigned int port) :
m_address(address),
m_port(port),
m_af(0U),
m_fd(-1)
{
}

CUDPSocket::CUDPSocket(unsigned int port) :
m_port(port),
m_af(0U),
m_fd(-1)
{
}

CUDPSocket::~CUDPSocket()
{
}

void CUDPSocket::startup()
{
}

void CUDPSocket::shutdown()
{
}

int CUDPSocket::lookup(const std::string& hostname, unsigned int port, sockaddr_storage& addr, unsigned int& address_length)
{
	struct addrinfo hints;
	::memset(&hints, 0, sizeof(hints));

	return lookup(hostname, port, addr, address_length, hints);
}

int CUDPSocket::lookup(const std::string& hostname, unsigned int port, sockaddr_storage& addr, unsigned int& address_length, struct addrinfo& hints)
{
	std::string portstr = std::to_string(port);
	struct addrinfo *res;

	/* port is always digits, no needs to lookup service */
	hints.ai_flags |= AI_NUMERICSERV;

	int err = getaddrinfo(hostname.empty() ? NULL : hostname.c_str(), portstr.c_str(), &hints, &res);
	if (err != 0) {
		sockaddr_in* paddr = (sockaddr_in*)&addr;
		::memset(paddr, 0x00U, address_length = sizeof(sockaddr_in));
		paddr->sin_family = AF_INET;
		paddr->sin_port = htons(port);
		paddr->sin_addr.s_addr = htonl(INADDR_NONE);
		LogError("Cannot find address for host %s", hostname.c_str());
		return err;
	}

	::memcpy(&addr, res->ai_addr, address_length = res->ai_addrlen);

	freeaddrinfo(res);

	return 0;
}

bool CUDPSocket::match(const sockaddr_storage& addr1, const sockaddr_storage& addr2, IPMATCHTYPE type)
{
	if (addr1.ss_family != addr2.ss_family)
		return false;

	if (type == IMT_ADDRESS_AND_PORT) {
		switch (addr1.ss_family) {
		case AF_INET:
			struct sockaddr_in *in_1, *in_2;
			in_1 = (struct sockaddr_in*)&addr1;
			in_2 = (struct sockaddr_in*)&addr2;
			return (in_1->sin_addr.s_addr == in_2->sin_addr.s_addr) && (in_1->sin_port == in_2->sin_port);
		case AF_INET6:
			struct sockaddr_in6 *in6_1, *in6_2;
			in6_1 = (struct sockaddr_in6*)&addr1;
			in6_2 = (struct sockaddr_in6*)&addr2;
			return IN6_ARE_ADDR_EQUAL(&in6_1->sin6_addr, &in6_2->sin6_addr) && (in6_1->sin6_port == in6_2->sin6_port);
		default:
			return false;
		}
	} else if (type == IMT_ADDRESS_ONLY) {
		switch (addr1.ss_family) {
		case AF_INET:
			struct sockaddr_in *in_1, *in_2;
			in_1 = (struct sockaddr_in*)&addr1;
			in_2 = (struct sockaddr_in*)&addr2;
			return in_1->sin_addr.s_addr == in_2->sin_addr.s_addr;
		case AF_INET6:
			struct sockaddr_in6 *in6_1, *in6_2;
			in6_1 = (struct sockaddr_in6*)&addr1;
			in6_2 = (struct sockaddr_in6*)&addr2;
			return IN6_ARE_ADDR_EQUAL(&in6_1->sin6_addr, &in6_2->sin6_addr);
		default:
			return false;
		}
	} else {
		return false;
	}
}

bool CUDPSocket::open(const sockaddr_storage& address)
{
	return open(address.ss_family);
}

bool CUDPSocket::open(unsigned int af)
{
	return open(0, af, m_address, m_port);
}

bool CUDPSocket::open(const unsigned int index, const unsigned int af, const std::string& address, const unsigned int port)
{
	sockaddr_storage addr;
	unsigned int addrlen;
	struct addrinfo hints;

	::memset(&hints, 0, sizeof(hints));
	hints.ai_flags  = AI_PASSIVE;
	hints.ai_family = af;

	/* to determine protocol family, call lookup() first. */
	int err = lookup(address, port, addr, addrlen, hints);
	if (err != 0) {
		LogError("The local address is invalid - %s", address.c_str());
		return false;
	}

	m_fd = ::socket(addr.ss_family, SOCK_DGRAM, 0);
	if (m_fd < 0) {
		LogError("Cannot create the UDP socket, err: %d", errno);
		return false;
	}

	m_af = addr.ss_family;

	if (port > 0U) {
		int reuse = 1;
		if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == -1) {
			LogError("Cannot set the UDP socket option, err: %d", errno);
			return false;
		}

		if (::bind(m_fd, (sockaddr*)&addr, addrlen) == -1) {
			LogError("Cannot bind the UDP address, err: %d", errno);
			return false;
		}

		LogInfo("Opening UDP port on %s:%u", address.c_str(), port);
	}

	return true;
}

int CUDPSocket::read(unsigned char* buffer, unsigned int length, sockaddr_storage& address, unsigned int &address_length)
{
	assert(buffer != NULL);
	assert(length > 0U);

	// Check that the readfrom() won't block
	struct pollfd pfd[1];
	if (m_fd >= 0) {
		pfd[0].fd = m_fd;
		pfd[0].events = POLLIN;
	} else
		return 0;

	// Return immediately
	int ret = ::poll(pfd, 1, 0);
	if (ret < 0) {
		LogError("Error returned from UDP poll, err: %d", errno);
		return -1;
	}

	if (!(pfd[0].revents & POLLIN))
               return 0;

	socklen_t size = sizeof(sockaddr_storage);

	ssize_t len = ::recvfrom(m_fd, (char*)buffer, length, 0, (sockaddr *)&address, &size);
	if (len <= 0) {
		LogError("Error returned from recvfrom, err: %d", errno);

		if (len == -1 && errno == ENOTSOCK) {
			LogMessage("Re-opening UDP port on %hn", m_port);
			close();
			open();
		}
		return -1;
	}

	address_length = size;
	return len;
}

bool CUDPSocket::write(const unsigned char* buffer, unsigned int length, const sockaddr_storage& address, unsigned int address_length)
{
	assert(buffer != NULL);
	assert(length > 0U);

	bool result = false;

	ssize_t ret = ::sendto(m_fd, (char *)buffer, length, 0, (sockaddr *)&address, address_length);

	if (ret < 0) {
		LogError("Error returned from sendto, err: %d", errno);
	} else {
		if (ret == ssize_t(length))
			result = true;
	}

	return result;
}

void CUDPSocket::close()
{
	::close(m_fd);
}

