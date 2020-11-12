/*
 *   Copyright (C) 2011-2018,2020 by Jonathan Naylor G4KLX
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

#include "SerialController.h"
#include "RingBuffer.h"
#include "Defines.h"
#include "Timer.h"

#include <string>

enum RESP_TYPE_MMDVM {
	RTM_OK,
	RTM_TIMEOUT,
	RTM_ERROR
};

class CModem {
public:
	CModem(const std::string& port, bool duplex, bool rxInvert, bool txInvert, bool pttInvert, unsigned int txDelay, unsigned int dmrDelay, bool trace, bool debug);
	virtual ~CModem();

	virtual void setSerialParams(const std::string& protocol, unsigned int address);
	virtual void setRFParams(unsigned int rxFrequency, int rxOffset, unsigned int txFrequency, int txOffset, int txDCOffset, int rxDCOffset, float rfLevel, unsigned int pocsagFrequency);
	virtual void setModeParams(bool dmrEnabled, bool pocsagEnabled);
	virtual void setLevels(float rxLevel, float cwIdTXLevel, float dmrTXLevel, float pocsagLevel);
	virtual void setDMRParams(unsigned int colorCode);
	virtual void setTransparentDataParams(unsigned int sendFrameType);

	virtual bool open();

	virtual unsigned int readDMRData1(unsigned char* data);
	virtual unsigned int readDMRData2(unsigned char* data);
	virtual unsigned int readTransparentData(unsigned char* data);

	virtual unsigned int readSerial(unsigned char* data, unsigned int length);

	virtual bool hasDMRSpace1() const;
	virtual bool hasDMRSpace2() const;
	virtual bool hasPOCSAGSpace() const;

	virtual bool hasTX() const;
	virtual bool hasCD() const;

	virtual bool hasError() const;

	virtual bool writeConfig();
	virtual bool writeDMRData1(const unsigned char* data, unsigned int length);
	virtual bool writeDMRData2(const unsigned char* data, unsigned int length);
	virtual bool writePOCSAGData(const unsigned char* data, unsigned int length);

	virtual bool writeTransparentData(const unsigned char* data, unsigned int length);

	virtual bool writeDMRInfo(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type);

	virtual bool writePOCSAGInfo(unsigned int ric, const std::string& message);

	virtual bool writeDMRStart(bool tx);
	virtual bool writeDMRShortLC(const unsigned char* lc);
	virtual bool writeDMRAbort(unsigned int slotNo);

	virtual bool writeSerial(const unsigned char* data, unsigned int length);

	virtual unsigned char getMode() const;
	virtual bool setMode(unsigned char mode);

	virtual bool sendCWId(const std::string& callsign);

	virtual HW_TYPE getHWType() const;

	virtual void clock(unsigned int ms);

	virtual void close();

	static CModem* createModem(const std::string& port, bool duplex, bool rxInvert, bool txInvert, bool pttInvert, unsigned int txDelay, unsigned int dmrDelay, bool trace, bool debug);

private:
	std::string                m_port;
	unsigned int               m_dmrColorCode;
	bool                       m_duplex;
	bool                       m_rxInvert;
	bool                       m_txInvert;
	bool                       m_pttInvert;
	unsigned int               m_txDelay;
	unsigned int               m_dmrDelay;
	float                      m_rxLevel;
	float                      m_cwIdTXLevel;
	float                      m_dmrTXLevel;
	float                      m_pocsagTXLevel;
	float                      m_rfLevel;
	bool                       m_trace;
	bool                       m_debug;
	unsigned int               m_rxFrequency;
	unsigned int               m_txFrequency;
	unsigned int               m_pocsagFrequency;
	bool                       m_dmrEnabled;
	bool                       m_pocsagEnabled;
	int                        m_rxDCOffset;
	int                        m_txDCOffset;
	CSerialController*         m_serial;
	unsigned char*             m_buffer;
	unsigned int               m_length;
	unsigned int               m_offset;
	CRingBuffer<unsigned char> m_rxDMRData1;
	CRingBuffer<unsigned char> m_rxDMRData2;
	CRingBuffer<unsigned char> m_txDMRData1;
	CRingBuffer<unsigned char> m_txDMRData2;
	CRingBuffer<unsigned char> m_txPOCSAGData;
	CRingBuffer<unsigned char> m_rxTransparentData;
	CRingBuffer<unsigned char> m_txTransparentData;
	unsigned int               m_sendTransparentDataFrameType;
	CTimer                     m_statusTimer;
	CTimer                     m_inactivityTimer;
	CTimer                     m_playoutTimer;
	unsigned int               m_dmrSpace1;
	unsigned int               m_dmrSpace2;
	unsigned int               m_pocsagSpace;
	bool                       m_tx;
	bool                       m_cd;
	bool                       m_error;
	unsigned char              m_mode;
	HW_TYPE                    m_hwType;

	bool readVersion();
	bool readStatus();
	bool setConfig();
	bool setFrequency();

	void printDebug();

	RESP_TYPE_MMDVM getResponse();
};
