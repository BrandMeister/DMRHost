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

#pragma once

#include "Display.h"
#include "UDPSocket.h"

#include <string>

class CNetDisplay : public CDisplay
{
public:
  CNetDisplay(const std::string& address, unsigned int port);
  virtual ~CNetDisplay();

  virtual bool open() override;

  virtual void close() override;

protected:
	virtual void setIdleInt() override;
	virtual void setErrorInt(const char* text) override;
	virtual void setQuitInt() override;

	virtual void writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type) override;
	virtual void writeDMRRSSIInt(unsigned int slotNo, unsigned char rssi) override;
	virtual void writeDMRTAInt(unsigned int slotNo, unsigned char* talkerAlias, const char* type) override;
	virtual void writeDMRBERInt(unsigned int slotNo, float ber) override;
	virtual void clearDMRInt(unsigned int slotNo) override;

	virtual void writePOCSAGInt(uint32_t ric, const std::string& message) override;
	virtual void clearPOCSAGInt() override;

	virtual void writeCWInt() override;
	virtual void clearCWInt() override;
private:
	CUDPSocket*  m_socket;
	std::string      m_addressStr;
	sockaddr_storage m_addr;
	unsigned int     m_addrLen;
	unsigned int     m_port;

	void write(unsigned char* data, unsigned int length);
};
