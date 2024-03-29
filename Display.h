/*
 *   Copyright (C) 2016,2017,2018,2020,2021,2023 by Jonathan Naylor G4KLX
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

#include "Timer.h"

#include <string>

#include <cstdint>

class CConf;
class CModem;

class CDisplay
{
public:
	CDisplay();
	virtual ~CDisplay() = 0;

	virtual bool open() = 0;

	void setIdle();
	void setError(const char* text);
	void setQuit();

	void writeDMR(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type);
	void writeDMRRSSI(unsigned int slotNo, unsigned char rssi);
	void writeDMRBER(unsigned int slotNo, float ber);
	void writeDMRTA(unsigned int slotNo, const unsigned char* talkerAlias, const char* type);
	void clearDMR(unsigned int slotNo);

	void writePOCSAG(uint32_t ric, const std::string& message);
	void clearPOCSAG();

	void writeCW();

	virtual void close() = 0;

	void clock(unsigned int ms);

	static CDisplay* createDisplay(const CConf& conf, CModem* modem);

protected:
	virtual void setIdleInt() = 0;
	virtual void setErrorInt(const char* text) = 0;
	virtual void setQuitInt() = 0;

	virtual void writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type) = 0;
	virtual void writeDMRRSSIInt(unsigned int slotNo, unsigned char rssi);
	virtual void writeDMRTAInt(unsigned int slotNo, const unsigned char* talkerAlias, const char* type);
	virtual void writeDMRBERInt(unsigned int slotNo, float ber);
	virtual void clearDMRInt(unsigned int slotNo) = 0;

	virtual void writePOCSAGInt(uint32_t ric, const std::string& message) = 0;
	virtual void clearPOCSAGInt() = 0;

	virtual void writeCWInt() = 0;
	virtual void clearCWInt() = 0;

	virtual void clockInt(unsigned int ms);

private:
	CTimer        m_timer1;
	CTimer        m_timer2;
	unsigned char m_mode1;
	unsigned char m_mode2;
};
