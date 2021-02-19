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

#include <string>
#include <vector>

class CConf
{
public:
  CConf(const std::string& file);
  ~CConf();

  bool read();

  // The General section
  std::string  getCallsign() const;
  unsigned int getId() const;
  unsigned int getTimeout() const;
  bool         getDuplex() const;

  // The Info section
  unsigned int getRXFrequency() const;
  unsigned int getTXFrequency() const;
  unsigned int getPower() const;
  float        getLatitude() const;
  float        getLongitude() const;
  int          getHeight() const;
  std::string  getLocation() const;
  std::string  getDescription() const;
  std::string  getURL() const;

  // The Log section
  unsigned int getLogSyslogLevel() const;
  unsigned int getLogDisplayLevel() const;
  unsigned int getLogFileLevel() const;
  std::string  getLogFilePath() const;
  std::string  getLogFileRoot() const;
  bool         getLogFileRotate() const;

  // The CW ID section
  bool         getCWIdEnabled() const;
  unsigned int getCWIdTime() const;
  std::string  getCWIdCallsign() const;

  // The Modem section
  std::string  getModemPort() const;
  std::string  getModemProtocol() const;
  unsigned int getModemAddress() const;
  bool         getModemRXInvert() const;
  bool         getModemTXInvert() const;
  bool         getModemPTTInvert() const;
  unsigned int getModemTXDelay() const;
  unsigned int getModemDMRDelay() const;
  int          getModemTXOffset() const;
  int          getModemRXOffset() const;
  int          getModemRXDCOffset() const;
  int          getModemTXDCOffset() const;
  float        getModemRFLevel() const;
  float        getModemRXLevel() const;
  float        getModemCWIdTXLevel() const;
  float        getModemDMRTXLevel() const;
  float        getModemPOCSAGTXLevel() const;
  std::string  getModemRSSIMappingFile() const;
  bool         getModemTrace() const;
  bool         getModemDebug() const;

  // The Transparent Data section
  bool         getTransparentEnabled() const;
  std::string  getTransparentRemoteAddress() const;
  unsigned int getTransparentRemotePort() const;
  std::string  getTransparentLocalAddress() const;
  unsigned int getTransparentLocalPort() const;
  unsigned int getTransparentSendFrameType() const;

  // The DMR section
  bool         getDMREnabled() const;
  DMR_BEACONS  getDMRBeacons() const;
  unsigned int getDMRBeaconDuration() const;
  unsigned int getDMRId() const;
  unsigned int getDMRColorCode() const;
  bool         getDMREmbeddedLCOnly() const;
  bool         getDMRDumpTAData() const;
  bool         getDMRSelfOnly() const;
  std::vector<unsigned int> getDMRPrefixes() const;
  std::vector<unsigned int> getDMRBlackList() const;
  std::vector<unsigned int> getDMRWhiteList() const;
  std::vector<unsigned int> getDMRSlot1TGWhiteList() const;
  std::vector<unsigned int> getDMRSlot2TGWhiteList() const;
  unsigned int getDMRCallHang() const;
  unsigned int getDMRTXHang() const;
  unsigned int getDMRModeHang() const;
  DMR_OVCM_TYPES getDMROVCM() const;

  // The POCSAG section
  bool         getPOCSAGEnabled() const;
  unsigned int getPOCSAGFrequency() const;

  // The DMR Network section
  bool         getDMRNetworkEnabled() const;
  std::string  getDMRNetworkAddress() const;
  unsigned int getDMRNetworkPort() const;
  std::string  getDMRNetworkPassword() const;
  std::string  getDMRNetworkOptions() const;
  bool         getDMRNetworkDebug() const;
  bool         getDMRNetworkSlot1() const;
  bool         getDMRNetworkSlot2() const;
  unsigned int getDMRNetworkModeHang() const;

  // The POCSAG Network section
  bool         getPOCSAGNetworkEnabled() const;
  std::string  getPOCSAGGatewayAddress() const;
  unsigned int getPOCSAGGatewayPort() const;
  std::string  getPOCSAGLocalAddress() const;
  unsigned int getPOCSAGLocalPort() const;
  unsigned int getPOCSAGNetworkModeHang() const;
  bool         getPOCSAGNetworkDebug() const;

  // The Display section
  bool         getDisplayEnabled() const;
  std::string  getDisplayAddress() const;
  unsigned int getDisplayPort() const;

private:
  std::string  m_file;
  std::string  m_callsign;
  unsigned int m_id;
  unsigned int m_timeout;
  bool         m_duplex;
  std::string  m_display;

  unsigned int m_rxFrequency;
  unsigned int m_txFrequency;
  unsigned int m_power;
  float        m_latitude;
  float        m_longitude;
  int          m_height;
  std::string  m_location;
  std::string  m_description;
  std::string  m_url;

  unsigned int m_logSyslogLevel;
  unsigned int m_logDisplayLevel;
  unsigned int m_logFileLevel;
  std::string  m_logFilePath;
  std::string  m_logFileRoot;
  bool         m_logFileRotate;

  bool         m_cwIdEnabled;
  unsigned int m_cwIdTime;
  std::string  m_cwIdCallsign;

  std::string  m_modemPort;
  std::string  m_modemProtocol;
  unsigned int m_modemAddress;
  bool         m_modemRXInvert;
  bool         m_modemTXInvert;
  bool         m_modemPTTInvert;
  unsigned int m_modemTXDelay;
  unsigned int m_modemDMRDelay;
  int          m_modemTXOffset;
  int          m_modemRXOffset;
  int          m_modemRXDCOffset;
  int          m_modemTXDCOffset;
  float        m_modemRFLevel;
  float        m_modemRXLevel;
  float        m_modemCWIdTXLevel;
  float        m_modemDMRTXLevel;
  float        m_modemPOCSAGTXLevel;
  std::string  m_modemRSSIMappingFile;
  bool         m_modemTrace;
  bool         m_modemDebug;

  bool         m_transparentEnabled;
  std::string  m_transparentRemoteAddress;
  unsigned int m_transparentRemotePort;
  std::string  m_transparentLocalAddress;
  unsigned int m_transparentLocalPort;
  unsigned int m_transparentSendFrameType;

  bool         m_dmrEnabled;
  DMR_BEACONS  m_dmrBeacons;
  unsigned int m_dmrBeaconDuration;
  unsigned int m_dmrId;
  unsigned int m_dmrColorCode;
  bool         m_dmrSelfOnly;
  bool         m_dmrEmbeddedLCOnly;
  bool         m_dmrDumpTAData;
  std::vector<unsigned int> m_dmrPrefixes;
  std::vector<unsigned int> m_dmrBlackList;
  std::vector<unsigned int> m_dmrWhiteList;
  std::vector<unsigned int> m_dmrSlot1TGWhiteList;
  std::vector<unsigned int> m_dmrSlot2TGWhiteList;
  unsigned int m_dmrCallHang;
  unsigned int m_dmrTXHang;
  unsigned int m_dmrModeHang;
  DMR_OVCM_TYPES m_dmrOVCM;

  bool         m_pocsagEnabled;
  unsigned int m_pocsagFrequency;

  bool         m_dmrNetworkEnabled;
  std::string  m_dmrNetworkAddress;
  unsigned int m_dmrNetworkPort;
  std::string  m_dmrNetworkPassword;
  std::string  m_dmrNetworkOptions;
  bool         m_dmrNetworkDebug;
  bool         m_dmrNetworkSlot1;
  bool         m_dmrNetworkSlot2;
  unsigned int m_dmrNetworkModeHang;

  bool         m_pocsagNetworkEnabled;
  std::string  m_pocsagGatewayAddress;
  unsigned int m_pocsagGatewayPort;
  std::string  m_pocsagLocalAddress;
  unsigned int m_pocsagLocalPort;
  unsigned int m_pocsagNetworkModeHang;
  bool         m_pocsagNetworkDebug;

  bool         m_displayEnabled;
  std::string  m_displayAddress;
  unsigned int m_displayPort;
};
