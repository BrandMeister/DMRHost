/*
 *   Copyright (C) 2009-2011,2013,2015,2016,2020 by Jonathan Naylor G4KLX
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

#pragma once

#include <string>

#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h> // freebsd
#include <arpa/inet.h>
#include <errno.h>

class CUDPSocket {
public:
	CUDPSocket(const std::string& address, unsigned short port = 0U);
	CUDPSocket(unsigned short port = 0U);
	~CUDPSocket();

	bool open(unsigned int af = AF_UNSPEC);
	bool open(const unsigned int af, const std::string& address, const unsigned short port);

	int  read(unsigned char* buffer, unsigned int length, sockaddr_storage& address, unsigned int &address_length);
	bool write(const unsigned char* buffer, unsigned int length, const sockaddr_storage& address, unsigned int address_length);

	void close();

	static int lookup(const std::string& hostName, unsigned short port, sockaddr_storage& address, unsigned int& address_length);
	static int lookup(const std::string& hostName, unsigned short port, sockaddr_storage& address, unsigned int& address_length, struct addrinfo& hints);

	static bool match(const sockaddr_storage& addr1, const sockaddr_storage& addr2);

private:
	std::string    m_address;
	unsigned short m_port;
	int            m_fd;
};
