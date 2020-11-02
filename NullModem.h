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

#ifndef	NULLMODEM_H
#define	NULLMODEM_H

#include "Modem.h"
#include "Defines.h"

#include <string>


class CNullModem : public CModem {
public:
	CNullModem(const std::string& port, bool duplex, bool rxInvert, bool txInvert, bool pttInvert, unsigned int txDelay, unsigned int dmrDelay, bool useCOSAsLockout, bool trace, bool debug);
	virtual ~CNullModem();

	virtual void setSerialParams(const std::string& protocol, unsigned int address){};
	virtual void setRFParams(unsigned int rxFrequency, int rxOffset, unsigned int txFrequency, int txOffset, int txDCOffset, int rxDCOffset, float rfLevel, unsigned int pocsagFrequency){};
	virtual void setModeParams(bool dmrEnabled, bool pocsagEnabled, bool fmEnabled){};
	virtual void setLevels(float rxLevel, float cwIdTXLevel, float dmrTXLevel, float pocsagLevel, float fmTXLevel){};
	virtual void setDMRParams(unsigned int colorCode){};
	virtual void setTransparentDataParams(unsigned int sendFrameType){};

	virtual bool open();

	virtual unsigned int readDMRData1(unsigned char* data){return 0;};
	virtual unsigned int readDMRData2(unsigned char* data){return 0;};
	virtual unsigned int readTransparentData(unsigned char* data){return 0;};

	virtual unsigned int readSerial(unsigned char* data, unsigned int length){return 0;};

	virtual bool hasDMRSpace1() const {return true;};
	virtual bool hasDMRSpace2() const {return true;};
	virtual bool hasPOCSAGSpace() const{return true;}; 

	virtual bool hasTX() const {return false;};
	virtual bool hasCD() const {return false;};

	virtual bool hasLockout() const {return false;};
	virtual bool hasError() const   {return false;};

	virtual bool writeDMRData1(const unsigned char* data, unsigned int length){return true;};
	virtual bool writeDMRData2(const unsigned char* data, unsigned int length){return true;};
	virtual bool writePOCSAGData(const unsigned char* data, unsigned int length){return true;};

	virtual bool writeTransparentData(const unsigned char* data, unsigned int length){return true;};

	virtual bool writeDMRInfo(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type){return true;};
	virtual bool writePOCSAGInfo(unsigned int ric, const std::string& message){return true;};
	virtual bool writeIPInfo(const std::string& address){return true;};

	virtual bool writeDMRStart(bool tx){return true;};
	virtual bool writeDMRShortLC(const unsigned char* lc){return true;};
	virtual bool writeDMRAbort(unsigned int slotNo){return true;};

	virtual bool writeSerial(const unsigned char* data, unsigned int length){return true;};

	virtual bool setMode(unsigned char mode){return true;};

	virtual bool sendCWId(const std::string& callsign){return true;};

	virtual HW_TYPE getHWType() const {return m_hwType;};

	virtual void clock(unsigned int ms){};

	virtual void close(){};

private:
	HW_TYPE                    m_hwType;
};

#endif
