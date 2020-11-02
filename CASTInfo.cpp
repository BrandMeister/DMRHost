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

#include "CASTInfo.h"

static bool networkInfoInitialized = false;
static unsigned char passCounter = 0;

CCASTInfo::CCASTInfo(CModem* modem) :
CDisplay(),
m_modem(modem),
m_ipaddress()
{
}

CCASTInfo::~CCASTInfo()
{
}

bool CCASTInfo::open()
{
	return true;
}

void CCASTInfo::setIdleInt()
{
    unsigned char info[100U];
    CNetworkInfo* m_network;

    passCounter ++;
    if (passCounter > 253U)
        networkInfoInitialized = false;

    if (! networkInfoInitialized) {
        //LogMessage("Initialize CNetworkInfo");
        info[0]=0;
        m_network = new CNetworkInfo;
        m_network->getNetworkInterface(info);
        m_ipaddress = (char*)info;
        delete m_network;

        if (m_modem != NULL)
            m_modem->writeIPInfo(m_ipaddress);

        networkInfoInitialized = true;
        passCounter = 0;
    }


}

void CCASTInfo::setErrorInt(const char* text)
{
}

void CCASTInfo::setLockoutInt()
{
}

void CCASTInfo::setQuitInt()
{
}

void CCASTInfo::writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type)
{
   if (m_modem != NULL)
        m_modem->writeDMRInfo(slotNo, src, group, dst, type);
}

void CCASTInfo::clearDMRInt(unsigned int slotNo)
{
}

void CCASTInfo::writePOCSAGInt(uint32_t ric, const std::string& message)
{
    if (m_modem != NULL)
        m_modem->writePOCSAGInfo(ric, message);
}

void CCASTInfo::clearPOCSAGInt()
{
}

void CCASTInfo::writeCWInt()
{
}

void CCASTInfo::clearCWInt()
{
}

void CCASTInfo::close()
{
}
