/*
 *   Copyright (C) 2015-2020 by Jonathan Naylor G4KLX
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

#include "POCSAGNetwork.h"
#include "POCSAGControl.h"
#include "DMRControl.h"
#include "DMRNetwork.h"
#include "Display.h"
#include "Timer.h"
#include "Modem.h"
#include "Conf.h"

#include <string>


class CMMDVMHost
{
public:
  CMMDVMHost(const std::string& confFile);
  ~CMMDVMHost();

  int run();

private:
  CConf           m_conf;
  CModem*         m_modem;
  CDMRControl*    m_dmr;
  CPOCSAGControl* m_pocsag;
  CDMRNetwork*    m_dmrNetwork;
  CPOCSAGNetwork* m_pocsagNetwork;
  CDisplay*       m_display;
  unsigned char   m_mode;
  unsigned int    m_dmrRFModeHang;
  unsigned int    m_dmrNetModeHang;
  unsigned int    m_pocsagNetModeHang;
  CTimer          m_modeTimer;
  CTimer          m_dmrTXTimer;
  CTimer          m_cwIdTimer;
  bool            m_duplex;
  unsigned int    m_timeout;
  bool            m_dmrEnabled;
  bool            m_pocsagEnabled;
  unsigned int    m_cwIdTime;
  std::string     m_callsign;
  unsigned int    m_id;
  std::string     m_cwCallsign;

  void readParams();
  bool createModem();
  bool createDMRNetwork();
  bool createPOCSAGNetwork();

  void setMode(unsigned char mode);
};
