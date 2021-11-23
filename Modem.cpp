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

#include "I2CController.h"
#include "DMRDefines.h"
#include "POCSAGDefines.h"
#include "Modem.h"
#include "NullModem.h"
#include "Utils.h"
#include "Log.h"

#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstdint>
#include <ctime>

#include <unistd.h>

const unsigned char MMDVM_FRAME_START = 0xE0U;

const unsigned char MMDVM_GET_VERSION = 0x00U;
const unsigned char MMDVM_GET_STATUS  = 0x01U;
const unsigned char MMDVM_SET_CONFIG  = 0x02U;
const unsigned char MMDVM_SET_MODE    = 0x03U;
const unsigned char MMDVM_SET_FREQ    = 0x04U;

const unsigned char MMDVM_SEND_CWID   = 0x0AU;

const unsigned char MMDVM_DMR_DATA1   = 0x18U;
const unsigned char MMDVM_DMR_LOST1   = 0x19U;
const unsigned char MMDVM_DMR_DATA2   = 0x1AU;
const unsigned char MMDVM_DMR_LOST2   = 0x1BU;
const unsigned char MMDVM_DMR_SHORTLC = 0x1CU;
const unsigned char MMDVM_DMR_START   = 0x1DU;
const unsigned char MMDVM_DMR_ABORT   = 0x1EU;

const unsigned char MMDVM_POCSAG_DATA = 0x50U;

const unsigned char MMDVM_ACK         = 0x70U;
const unsigned char MMDVM_NAK         = 0x7FU;

const unsigned char MMDVM_SERIAL      = 0x80U;

const unsigned char MMDVM_TRANSPARENT = 0x90U;
const unsigned char MMDVM_QSO_INFO    = 0x91U;

const unsigned char MMDVM_DEBUG1      = 0xF1U;
const unsigned char MMDVM_DEBUG2      = 0xF2U;
const unsigned char MMDVM_DEBUG3      = 0xF3U;
const unsigned char MMDVM_DEBUG4      = 0xF4U;
const unsigned char MMDVM_DEBUG5      = 0xF5U;

const unsigned int MAX_RESPONSES = 30U;

const unsigned int BUFFER_LENGTH = 2000U;

const unsigned char CAP1_DMR    = 0x02U;
const unsigned char CAP2_POCSAG = 0x01U;


CModem::CModem(const std::string& port, bool duplex, bool rxInvert, bool txInvert, bool pttInvert, unsigned int txDelay, unsigned int dmrDelay, bool trace, bool debug) :
m_port(port),
m_protocolVersion(0U),
m_dmrColorCode(0U),
m_duplex(duplex),
m_rxInvert(rxInvert),
m_txInvert(txInvert),
m_pttInvert(pttInvert),
m_txDelay(txDelay),
m_dmrDelay(dmrDelay),
m_rxLevel(0.0F),
m_cwIdTXLevel(0.0F),
m_dmrTXLevel(0.0F),
m_pocsagTXLevel(0.0F),
m_rfLevel(0.0F),
m_trace(trace),
m_debug(debug),
m_rxFrequency(0U),
m_txFrequency(0U),
m_pocsagFrequency(0U),
m_dmrEnabled(false),
m_pocsagEnabled(false),
m_rxDCOffset(0),
m_txDCOffset(0),
m_serial(NULL),
m_buffer(NULL),
m_length(0U),
m_offset(0U),
m_rxDMRData1(1000U, "Modem RX DMR1"),
m_rxDMRData2(1000U, "Modem RX DMR2"),
m_txDMRData1(1000U, "Modem TX DMR1"),
m_txDMRData2(1000U, "Modem TX DMR2"),
m_txPOCSAGData(1000U, "Modem TX POCSAG"),
m_rxTransparentData(1000U, "Modem RX Transparent"),
m_txTransparentData(1000U, "Modem TX Transparent"),
m_sendTransparentDataFrameType(0U),
m_statusTimer(1000U, 0U, 250U),
m_inactivityTimer(1000U, 2U),
m_playoutTimer(1000U, 0U, 10U),
m_dmrSpace1(0U),
m_dmrSpace2(0U),
m_pocsagSpace(0U),
m_tx(false),
m_error(false),
m_mode(MODE_IDLE),
m_hwType("MMDVM_Unknown"),
m_capabilities1(0x00U),
m_capabilities2(0x00U)
{
	m_buffer = new unsigned char[BUFFER_LENGTH];

	assert(!port.empty());
}

CModem::~CModem()
{
	delete   m_serial;
	delete[] m_buffer;
}

void CModem::setSerialParams(const std::string& protocol, unsigned int address)
{
	// Create the serial controller instance according the protocol specified in conf.
#if defined(__linux__)
	if (protocol == "i2c")
		m_serial = new CI2CController(m_port, 115200, address, true);
	else
#endif
		m_serial = new CSerialController(m_port, 115200, true);
}

void CModem::setRFParams(unsigned int rxFrequency, int rxOffset, unsigned int txFrequency, int txOffset, int txDCOffset, int rxDCOffset, float rfLevel, unsigned int pocsagFrequency)
{
	m_rxFrequency     = rxFrequency + rxOffset;
	m_txFrequency     = txFrequency + txOffset;
	m_txDCOffset      = txDCOffset;
	m_rxDCOffset      = rxDCOffset;
	m_rfLevel         = rfLevel;
	m_pocsagFrequency = pocsagFrequency + txOffset;
}

void CModem::setModeParams(bool dmrEnabled, bool pocsagEnabled)
{
	m_dmrEnabled    = dmrEnabled;
	m_pocsagEnabled = pocsagEnabled;
}

void CModem::setLevels(float rxLevel, float cwIdTXLevel, float dmrTXLevel, float pocsagTXLevel)
{
	m_rxLevel       = rxLevel;
	m_cwIdTXLevel   = cwIdTXLevel;
	m_dmrTXLevel    = dmrTXLevel;
	m_pocsagTXLevel = pocsagTXLevel;
}

void CModem::setDMRParams(unsigned int colorCode)
{
	assert(colorCode < 16U);

	m_dmrColorCode = colorCode;
}

void CModem::setTransparentDataParams(unsigned int sendFrameType)
{
    m_sendTransparentDataFrameType = sendFrameType;
}

bool CModem::open()
{
	::LogMessage("Opening the MMDVM");

	bool ret = m_serial->open();
	if (!ret)
		return false;

	ret = readVersion();
	if (!ret) {
		m_serial->close();
		delete m_serial;
		m_serial = NULL;
		return false;
	} else {
		/* Stopping the inactivity timer here when a firmware version has been
		   successfuly read prevents the death spiral of "no reply from modem..." */
		m_inactivityTimer.stop();
	}

	ret = setFrequency();
	if (!ret) {
		m_serial->close();
		delete m_serial;
		m_serial = NULL;
		return false;
	}

	ret = writeConfig();
	if (!ret) {
		m_serial->close();
		delete m_serial;
		m_serial = NULL;
		return false;
	}

	m_statusTimer.start();

	m_error  = false;
	m_offset = 0U;

	return true;
}

void CModem::clock(unsigned int ms)
{
	assert(m_serial != NULL);

	// Poll the modem status every 250ms
	m_statusTimer.clock(ms);
	if (m_statusTimer.hasExpired()) {
		readStatus();
		m_statusTimer.start();
	}

	m_inactivityTimer.clock(ms);
	if (m_inactivityTimer.hasExpired()) {
		LogError("No reply from the modem for some time, resetting it");
		m_error = true;
		close();

		usleep(2000 * 1000);		// 2s
		while (!open())
			usleep(5000 * 1000);	// 5s
	}

	RESP_TYPE_MMDVM type = getResponse();

	if (type == RTM_TIMEOUT) {
		// Nothing to do
	} else if (type == RTM_ERROR) {
		// Nothing to do
	} else {
		// type == RTM_OK
		switch (m_buffer[2U]) {
			case MMDVM_DMR_DATA1: {
					if (m_trace)
						CUtils::dump(1U, "RX DMR Data 1", m_buffer, m_length);

					unsigned char data = m_length - 2U;
					m_rxDMRData1.addData(&data, 1U);

					if (m_buffer[3U] == (DMR_SYNC_DATA | DT_TERMINATOR_WITH_LC))
						data = TAG_EOT;
					else
						data = TAG_DATA;
					m_rxDMRData1.addData(&data, 1U);

					m_rxDMRData1.addData(m_buffer + 3U, m_length - 3U);
				}
				break;

			case MMDVM_DMR_DATA2: {
					if (m_trace)
						CUtils::dump(1U, "RX DMR Data 2", m_buffer, m_length);

					unsigned char data = m_length - 2U;
					m_rxDMRData2.addData(&data, 1U);

					if (m_buffer[3U] == (DMR_SYNC_DATA | DT_TERMINATOR_WITH_LC))
						data = TAG_EOT;
					else
						data = TAG_DATA;
					m_rxDMRData2.addData(&data, 1U);

					m_rxDMRData2.addData(m_buffer + 3U, m_length - 3U);
				}
				break;

			case MMDVM_DMR_LOST1: {
					if (m_trace)
						CUtils::dump(1U, "RX DMR Lost 1", m_buffer, m_length);

					unsigned char data = 1U;
					m_rxDMRData1.addData(&data, 1U);

					data = TAG_LOST;
					m_rxDMRData1.addData(&data, 1U);
				}
				break;

			case MMDVM_DMR_LOST2: {
					if (m_trace)
						CUtils::dump(1U, "RX DMR Lost 2", m_buffer, m_length);

					unsigned char data = 1U;
					m_rxDMRData2.addData(&data, 1U);

					data = TAG_LOST;
					m_rxDMRData2.addData(&data, 1U);
				}
				break;

			case MMDVM_GET_STATUS: {
					// if (m_trace)
					//	CUtils::dump(1U, "GET_STATUS", m_buffer, m_length);

					switch (m_protocolVersion) {
					case 1U: {
						m_mode = m_buffer[4U];

						m_tx = (m_buffer[5U] & 0x01U) == 0x01U;

						bool adcOverflow = (m_buffer[5U] & 0x02U) == 0x02U;
						if (adcOverflow)
							LogError("MMDVM ADC levels have overflowed");

						bool rxOverflow = (m_buffer[5U] & 0x04U) == 0x04U;
						if (rxOverflow)
							LogError("MMDVM RX buffer has overflowed");

						bool txOverflow = (m_buffer[5U] & 0x08U) == 0x08U;
						if (txOverflow)
							LogError("MMDVM TX buffer has overflowed");

						bool dacOverflow = (m_buffer[5U] & 0x20U) == 0x20U;
						if (dacOverflow)
							LogError("MMDVM DAC levels have overflowed");

						m_dmrSpace1  = m_buffer[7U];
						m_dmrSpace2  = m_buffer[8U];

						if (m_length > 12U)
							m_pocsagSpace = m_buffer[12U];
						}
						break;
					case 2U: {
						m_mode = m_buffer[3U];

						m_tx = (m_buffer[4U] & 0x01U) == 0x01U;

						bool adcOverflow = (m_buffer[4U] & 0x02U) == 0x02U;
						if (adcOverflow)
							LogError("MMDVM ADC levels have overflowed");

						bool rxOverflow = (m_buffer[4U] & 0x04U) == 0x04U;
						if (rxOverflow)
							LogError("MMDVM RX buffer has overflowed");

						bool txOverflow = (m_buffer[4U] & 0x08U) == 0x08U;
						if (txOverflow)
							LogError("MMDVM TX buffer has overflowed");

						bool dacOverflow = (m_buffer[4U] & 0x20U) == 0x20U;
						if (dacOverflow)
							LogError("MMDVM DAC levels have overflowed");

						m_dmrSpace1   = m_buffer[7U];
						m_dmrSpace2   = m_buffer[8U];
						m_pocsagSpace = m_buffer[16U];

						}
						break;
					default:
						m_dmrSpace1   = 0U;
						m_dmrSpace2   = 0U;
						m_pocsagSpace = 0U;
						break;
					}

					m_inactivityTimer.start();
					// LogMessage("status=%02X, tx=%d, space=%u,%u,%u,%u,%u,%u,%u cd=%d", m_buffer[5U], int(m_tx), m_dmrSpace1, m_dmrSpace2, m_pocsagSpace);
				}
				break;

			case MMDVM_TRANSPARENT: {
					if (m_trace)
						CUtils::dump(1U, "RX Transparent Data", m_buffer, m_length);

					unsigned char offset = m_sendTransparentDataFrameType;
					if (offset > 1U) offset = 1U;
					unsigned char data = m_length - 3U + offset;
					m_rxTransparentData.addData(&data, 1U);

					m_rxTransparentData.addData(m_buffer + 3U - offset, m_length - 3U + offset);
				}
				break;

			// These should not be received, but don't complain if we do
			case MMDVM_GET_VERSION:
			case MMDVM_ACK:
				break;

			case MMDVM_NAK:
				LogWarning("Received a NAK from the MMDVM, command = 0x%02X, reason = %u", m_buffer[3U], m_buffer[4U]);
				break;

			case MMDVM_DEBUG1:
			case MMDVM_DEBUG2:
			case MMDVM_DEBUG3:
			case MMDVM_DEBUG4:
			case MMDVM_DEBUG5:
				printDebug();
				break;

			case MMDVM_SERIAL:
				//DMRHost does not process serial data from the display,
				// so we send it to the transparent port if sendFrameType==1
				if (m_sendTransparentDataFrameType > 0U) {
					if (m_trace)
						CUtils::dump(1U, "RX Serial Data", m_buffer, m_length);

					unsigned char offset = m_sendTransparentDataFrameType;
					if (offset > 1U) offset = 1U;
					unsigned char data = m_length - 3U + offset;
					m_rxTransparentData.addData(&data, 1U);

					m_rxTransparentData.addData(m_buffer + 3U - offset, m_length - 3U + offset);
					break; //only break when sendFrameType>0, else message is unknown
				}
			default:
				LogMessage("Unknown message, type: %02X", m_buffer[2U]);
				CUtils::dump("Buffer dump", m_buffer, m_length);
				break;
		}
	}

	// Only feed data to the modem if the playout timer has expired
	m_playoutTimer.clock(ms);
	if (!m_playoutTimer.hasExpired())
		return;

	if (m_dmrSpace1 > 1U && !m_txDMRData1.isEmpty()) {
		unsigned char len = 0U;
		m_txDMRData1.getData(&len, 1U);
		m_txDMRData1.getData(m_buffer, len);

		if (m_trace)
			CUtils::dump(1U, "TX DMR Data 1", m_buffer, len);

		int ret = m_serial->write(m_buffer, len);
		if (ret != int(len))
			LogWarning("Error when writing DMR data to the MMDVM");

		m_playoutTimer.start();

		m_dmrSpace1--;
	}

	if (m_dmrSpace2 > 1U && !m_txDMRData2.isEmpty()) {
		unsigned char len = 0U;
		m_txDMRData2.getData(&len, 1U);
		m_txDMRData2.getData(m_buffer, len);

		if (m_trace)
			CUtils::dump(1U, "TX DMR Data 2", m_buffer, len);

		int ret = m_serial->write(m_buffer, len);
		if (ret != int(len))
			LogWarning("Error when writing DMR data to the MMDVM");

		m_playoutTimer.start();

		m_dmrSpace2--;
	}

	if (m_pocsagSpace > 1U && !m_txPOCSAGData.isEmpty()) {
		unsigned char len = 0U;
		m_txPOCSAGData.getData(&len, 1U);
		m_txPOCSAGData.getData(m_buffer, len);

		if (m_trace)
			CUtils::dump(1U, "TX POCSAG Data", m_buffer, len);

		int ret = m_serial->write(m_buffer, len);
		if (ret != int(len))
			LogWarning("Error when writing POCSAG data to the MMDVM");

		m_playoutTimer.start();

		m_pocsagSpace--;
	}

	if (!m_txTransparentData.isEmpty()) {
		unsigned char len = 0U;
		m_txTransparentData.getData(&len, 1U);
		m_txTransparentData.getData(m_buffer, len);

		if (m_trace)
			CUtils::dump(1U, "TX Transparent Data", m_buffer, len);

		int ret = m_serial->write(m_buffer, len);
		if (ret != int(len))
			LogWarning("Error when writing Transparent data to the MMDVM");
	}
}

void CModem::close()
{
	assert(m_serial != NULL);

	::LogMessage("Closing the MMDVM");

	m_serial->close();
}

unsigned int CModem::readDMRData1(unsigned char* data)
{
	assert(data != NULL);

	if (m_rxDMRData1.isEmpty())
		return 0U;

	unsigned char len = 0U;
	m_rxDMRData1.getData(&len, 1U);
	m_rxDMRData1.getData(data, len);

	return len;
}

unsigned int CModem::readDMRData2(unsigned char* data)
{
	assert(data != NULL);

	if (m_rxDMRData2.isEmpty())
		return 0U;

	unsigned char len = 0U;
	m_rxDMRData2.getData(&len, 1U);
	m_rxDMRData2.getData(data, len);

	return len;
}

unsigned int CModem::readTransparentData(unsigned char* data)
{
	assert(data != NULL);

	if (m_rxTransparentData.isEmpty())
		return 0U;

	unsigned char len = 0U;
	m_rxTransparentData.getData(&len, 1U);
	m_rxTransparentData.getData(data, len);

	return len;
}

bool CModem::hasDMRSpace1() const
{
	unsigned int space = m_txDMRData1.freeSpace() / (DMR_FRAME_LENGTH_BYTES + 4U);

	return space > 1U;
}

bool CModem::hasDMRSpace2() const
{
	unsigned int space = m_txDMRData2.freeSpace() / (DMR_FRAME_LENGTH_BYTES + 4U);

	return space > 1U;
}

bool CModem::writeDMRData1(const unsigned char* data, unsigned int length)
{
	assert(data != NULL);
	assert(length > 0U);

	if (data[0U] != TAG_DATA && data[0U] != TAG_EOT)
		return false;

	unsigned char buffer[40U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = length + 2U;
	buffer[2U] = MMDVM_DMR_DATA1;

	::memcpy(buffer + 3U, data + 1U, length - 1U);

	unsigned char len = length + 2U;
	m_txDMRData1.addData(&len, 1U);
	m_txDMRData1.addData(buffer, len);

	return true;
}

bool CModem::writeDMRData2(const unsigned char* data, unsigned int length)
{
	assert(data != NULL);
	assert(length > 0U);

	if (data[0U] != TAG_DATA && data[0U] != TAG_EOT)
		return false;

	unsigned char buffer[40U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = length + 2U;
	buffer[2U] = MMDVM_DMR_DATA2;

	::memcpy(buffer + 3U, data + 1U, length - 1U);

	unsigned char len = length + 2U;
	m_txDMRData2.addData(&len, 1U);
	m_txDMRData2.addData(buffer, len);

	return true;
}

bool CModem::hasPOCSAGSpace() const
{
	unsigned int space = m_txPOCSAGData.freeSpace() / (POCSAG_FRAME_LENGTH_BYTES + 4U);

	return space > 1U;
}

bool CModem::writePOCSAGData(const unsigned char* data, unsigned int length)
{
	assert(data != NULL);
	assert(length > 0U);

	unsigned char buffer[130U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = length + 3U;
	buffer[2U] = MMDVM_POCSAG_DATA;

	::memcpy(buffer + 3U, data, length);

	unsigned char len = length + 3U;
	m_txPOCSAGData.addData(&len, 1U);
	m_txPOCSAGData.addData(buffer, len);

	return true;
}

bool CModem::writeTransparentData(const unsigned char* data, unsigned int length)
{
	assert(data != NULL);
	assert(length > 0U);

	unsigned char buffer[250U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = length + 3U;
	buffer[2U] = MMDVM_TRANSPARENT;

	if (m_sendTransparentDataFrameType > 0U) {
		::memcpy(buffer + 2U, data, length);
		length--;
		buffer[1U]--;

		//when sendFrameType==1 , only 0x80 and 0x90 (MMDVM_SERIAL and MMDVM_TRANSPARENT) are allowed
		//  and reverted to default (MMDVM_TRANSPARENT) for any other value
		//when >1, frame type is not checked
		if (m_sendTransparentDataFrameType == 1U) {
			if ((buffer[2U] & 0xE0) != 0x80)
				buffer[2U] = MMDVM_TRANSPARENT;
		}
	} else {
		::memcpy(buffer + 3U, data, length);
	}

	unsigned char len = length + 3U;
	m_txTransparentData.addData(&len, 1U);
	m_txTransparentData.addData(buffer, len);

	return true;
}

bool CModem::hasTX() const
{
	return m_tx;
}

bool CModem::hasError() const
{
	return m_error;
}

bool CModem::hasDMR() const
{
	return (m_capabilities1 & CAP1_DMR) == CAP1_DMR;
}

bool CModem::hasPOCSAG() const
{
	return (m_capabilities2 & CAP2_POCSAG) == CAP2_POCSAG;
}

bool CModem::readVersion()
{
	assert(m_serial != NULL);

	usleep(2000 * 1000);	// 2s

	for (unsigned int i = 0U; i < 6U; i++) {
		unsigned char buffer[3U];

		buffer[0U] = MMDVM_FRAME_START;
		buffer[1U] = 3U;
		buffer[2U] = MMDVM_GET_VERSION;

		// CUtils::dump(1U, "Written", buffer, 3U);

		int ret = m_serial->write(buffer, 3U);
		if (ret != 3)
			return false;

#if defined(__APPLE__)
		m_serial->setNonblock(true);
#endif

		for (unsigned int count = 0U; count < MAX_RESPONSES; count++) {
			usleep(10 * 1000);
			RESP_TYPE_MMDVM resp = getResponse();
			if (resp == RTM_OK && m_buffer[2U] == MMDVM_GET_VERSION) {
				m_protocolVersion = m_buffer[3U];
				switch (m_protocolVersion) {
				case 1U:
					LogInfo("MMDVM protocol version: %u, description: %.*s", m_protocolVersion, m_length - 4U, m_buffer + 4U);
					m_capabilities1 = CAP1_DMR;
					m_capabilities2 = CAP2_POCSAG;
					break;
				case 2U:
					LogInfo("MMDVM protocol version: %u, description: %.*s", m_protocolVersion, m_length - 23U, m_buffer + 23U);
					switch (m_buffer[6U]) {
					case 0U:
						LogInfo("CPU: Atmel ARM, UDID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", m_buffer[7U], m_buffer[8U], m_buffer[9U], m_buffer[10U], m_buffer[11U], m_buffer[12U], m_buffer[13U], m_buffer[14U], m_buffer[15U], m_buffer[16U], m_buffer[17U], m_buffer[18U], m_buffer[19U], m_buffer[20U], m_buffer[21U], m_buffer[22U]);
						break;
					case 1U:
						LogInfo("CPU: NXP ARM, UDID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", m_buffer[7U], m_buffer[8U], m_buffer[9U], m_buffer[10U], m_buffer[11U], m_buffer[12U], m_buffer[13U], m_buffer[14U], m_buffer[15U], m_buffer[16U], m_buffer[17U], m_buffer[18U], m_buffer[19U], m_buffer[20U], m_buffer[21U], m_buffer[22U]);
						break;
					case 2U:
						LogInfo("CPU: ST-Micro ARM, UDID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", m_buffer[7U], m_buffer[8U], m_buffer[9U], m_buffer[10U], m_buffer[11U], m_buffer[12U], m_buffer[13U], m_buffer[14U], m_buffer[15U], m_buffer[16U], m_buffer[17U], m_buffer[18U]);
						break;
					default:
						LogInfo("CPU: Unknown type: %u", m_buffer[6U]);
						break;
					}
					m_capabilities1 = m_buffer[4U];
					m_capabilities2 = m_buffer[5U];
					char modeText[100U];
					::strcpy(modeText, "Modes:");
					if (hasDMR())
						::strcat(modeText, " DMR");
					if (hasPOCSAG())
						::strcat(modeText, " POCSAG");
					LogInfo(modeText);
					break;
				default:
					LogError("MMDVM protocol version: %u, unsupported by this version of the MMDVM Host", m_protocolVersion);
					return false;
				}

				char _hwType[40U];
				::strcpy(_hwType, "MMDVM_Unknown");

				if (::memcmp(m_buffer + 4U, "MMDVM ", 6U) == 0)
					::strcpy(_hwType, "MMDVM");
				else if (::memcmp(m_buffer + 4U, "DVMEGA ", 7U) == 0)
					::strcpy(_hwType, "MMDVM_DVMega");
				else {
					char* hw = NULL;
					if (m_protocolVersion == 1)
						hw = ::strtok((char*)m_buffer + 4U, "-");
					if (m_protocolVersion == 2)
						hw = ::strtok((char*)m_buffer + 23U, "-");
					// we do not trust the modem fw too much
					if (hw != NULL) {
						::strcpy(_hwType, "MMDVM_");
						::strncat(_hwType, hw, 40U - 6U);
					}
				}
				m_hwType = strdup(_hwType);

				return true;
			}
		}

		usleep(1500 * 1000);
	}

	LogError("Unable to read the firmware version after six attempts");

	return false;
}

bool CModem::readStatus()
{
	assert(m_serial != NULL);

	unsigned char buffer[3U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = 3U;
	buffer[2U] = MMDVM_GET_STATUS;

	// CUtils::dump(1U, "Written", buffer, 3U);

	return m_serial->write(buffer, 3U) == 3;
}

bool CModem::writeConfig()
{
	switch (m_protocolVersion) {
	case 1U:
		return setConfig1();
	case 2U:
		return setConfig2();
	default:
		return false;
	}
}

bool CModem::setConfig1()
{
	assert(m_serial != NULL);

	unsigned char buffer[30U];

	buffer[0U] = MMDVM_FRAME_START;

	buffer[1U] = 26U;

	buffer[2U] = MMDVM_SET_CONFIG;

	buffer[3U] = 0x00U;
	if (m_rxInvert)
		buffer[3U] |= 0x01U;
	if (m_txInvert)
		buffer[3U] |= 0x02U;
	if (m_pttInvert)
		buffer[3U] |= 0x04U;
	if (m_debug)
		buffer[3U] |= 0x10U;
	if (!m_duplex)
		buffer[3U] |= 0x80U;

	buffer[4U] = 0x00U;
	if (m_dmrEnabled)
		buffer[4U] |= 0x02U;
	if (m_pocsagEnabled)
		buffer[4U] |= 0x20U;

	buffer[5U] = m_txDelay / 10U;		// In 10ms units

	buffer[6U] = MODE_IDLE;

	buffer[7U] = (unsigned char)(m_rxLevel * 2.55F + 0.5F);

	buffer[8U] = (unsigned char)(m_cwIdTXLevel * 2.55F + 0.5F);

	buffer[9U] = m_dmrColorCode;

	buffer[10U] = m_dmrDelay;

	buffer[11U] = 128U;           // Was OscOffset

	buffer[12U] = 0x00U;

	buffer[13U] = (unsigned char)(m_dmrTXLevel * 2.55F + 0.5F);

	buffer[14U] = 0x00U;
	buffer[15U] = 0x00U;

	buffer[16U] = (unsigned char)(m_txDCOffset + 128);
	buffer[17U] = (unsigned char)(m_rxDCOffset + 128);

	buffer[18U] = 0x00U;
	buffer[19U] = 0x00U;

	buffer[20U] = (unsigned char)(m_pocsagTXLevel * 2.55F + 0.5F);

	buffer[21U] = 0x00U;
	buffer[22U] = 0x00U;
	buffer[23U] = 0x00U;
	buffer[24U] = 0x00U;
	buffer[25U] = 0x00U;

	// CUtils::dump(1U, "Written", buffer, 26U);

	int ret = m_serial->write(buffer, 26U);
	if (ret != 26)
		return false;

	unsigned int count = 0U;
	RESP_TYPE_MMDVM resp;
	do {
		usleep(10 * 1000);

		resp = getResponse();
		if (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK) {
			count++;
			if (count >= MAX_RESPONSES) {
				LogError("The MMDVM is not responding to the SET_CONFIG command");
				return false;
			}
		}
	} while (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK);

	// CUtils::dump(1U, "Response", m_buffer, m_length);

	if (resp == RTM_OK && m_buffer[2U] == MMDVM_NAK) {
		LogError("Received a NAK to the SET_CONFIG command from the modem");
		return false;
	}

	m_playoutTimer.start();

	return true;
}

bool CModem::setConfig2()
{
	assert(m_serial != NULL);

	unsigned char buffer[50U];

	buffer[0U] = MMDVM_FRAME_START;

	buffer[1U] = 40U;

	buffer[2U] = MMDVM_SET_CONFIG;

	buffer[3U] = 0x00U;
	if (m_rxInvert)
		buffer[3U] |= 0x01U;
	if (m_txInvert)
		buffer[3U] |= 0x02U;
	if (m_pttInvert)
		buffer[3U] |= 0x04U;
	if (m_debug)
		buffer[3U] |= 0x10U;
	if (!m_duplex)
		buffer[3U] |= 0x80U;

	buffer[4U] = 0x00U;
	if (m_dmrEnabled)
		buffer[4U] |= 0x02U;

	buffer[5U] = 0x00U;
	if (m_pocsagEnabled)
		buffer[5U] |= 0x01U;

	buffer[6U] = m_txDelay / 10U;		// In 10ms units

	buffer[7U] = MODE_IDLE;

	buffer[8U] = (unsigned char)(m_txDCOffset + 128);
	buffer[9U] = (unsigned char)(m_rxDCOffset + 128);

	buffer[10U] = (unsigned char)(m_rxLevel * 2.55F + 0.5F);

	buffer[11U] = (unsigned char)(m_cwIdTXLevel * 2.55F + 0.5F);
	buffer[12U] = 0x00U;
	buffer[13U] = (unsigned char)(m_dmrTXLevel * 2.55F + 0.5F);
	buffer[14U] = 0x00U;
	buffer[15U] = 0x00U;
	buffer[16U] = 0x00U;
	buffer[17U] = 0x00U;
	buffer[18U] = (unsigned char)(m_pocsagTXLevel * 2.55F + 0.5F);
	buffer[19U] = 0x00U;
	buffer[20U] = 0x00U;
	buffer[21U] = 0x00U;
	buffer[22U] = 0x00U;

	buffer[23U] = 0x00U;
	buffer[24U] = 0x00U;
	buffer[25U] = 0x00U;
	buffer[26U] = 0x00U;
	buffer[27U] = 0x00U;
	buffer[28U] = 0x00U;

	buffer[29U] = m_dmrColorCode;
	buffer[30U] = m_dmrDelay;

	buffer[31U] = 0x86U; // wtf is this. m_ax25RXTwist 128 + default value 6. required by firmware even if ax25 is disabled
	buffer[32U] = 0x00U;
	buffer[33U] = 0x00U;
	buffer[34U] = 0x00U;

	buffer[35U] = 0x00U;
	buffer[36U] = 0x00U;
	buffer[37U] = 0x00U;
	buffer[38U] = 0x00U;
	buffer[39U] = 0x00U;

	// CUtils::dump(1U, "Written", buffer, 40U);

	int ret = m_serial->write(buffer, 40U);
	if (ret != 40)
		return false;

	unsigned int count = 0U;
	RESP_TYPE_MMDVM resp;
	do {
		usleep(10 * 1000);

		resp = getResponse();
		if (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK) {
			count++;
			if (count >= MAX_RESPONSES) {
				LogError("The MMDVM is not responding to the SET_CONFIG command");
				return false;
			}
		}
	} while (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK);

	// CUtils::dump(1U, "Response", m_buffer, m_length);

	if (resp == RTM_OK && m_buffer[2U] == MMDVM_NAK) {
		LogError("Received a NAK to the SET_CONFIG command from the modem");
		return false;
	}

	m_playoutTimer.start();

	return true;
}

bool CModem::setFrequency()
{
	assert(m_serial != NULL);

	unsigned char buffer[20U];
	unsigned char len;
	unsigned int  pocsagFrequency = 433000000U;

	if (m_pocsagEnabled)
		pocsagFrequency = m_pocsagFrequency;

	if (::strcmp(m_hwType, "MMDVM_DVMega") == 0)
		len = 12U;
	else {
		buffer[12U]  = (unsigned char)(m_rfLevel * 2.55F + 0.5F);

		buffer[13U] = (pocsagFrequency >> 0)  & 0xFFU;
		buffer[14U] = (pocsagFrequency >> 8)  & 0xFFU;
		buffer[15U] = (pocsagFrequency >> 16) & 0xFFU;
		buffer[16U] = (pocsagFrequency >> 24) & 0xFFU;

		len = 17U;
	}

	buffer[0U]  = MMDVM_FRAME_START;

	buffer[1U]  = len;

	buffer[2U]  = MMDVM_SET_FREQ;

	buffer[3U]  = 0x00U;

	buffer[4U]  = (m_rxFrequency >> 0) & 0xFFU;
	buffer[5U]  = (m_rxFrequency >> 8) & 0xFFU;
	buffer[6U]  = (m_rxFrequency >> 16) & 0xFFU;
	buffer[7U]  = (m_rxFrequency >> 24) & 0xFFU;

	buffer[8U]  = (m_txFrequency >> 0) & 0xFFU;
	buffer[9U]  = (m_txFrequency >> 8) & 0xFFU;
	buffer[10U] = (m_txFrequency >> 16) & 0xFFU;
	buffer[11U] = (m_txFrequency >> 24) & 0xFFU;

	// CUtils::dump(1U, "Written", buffer, len);

	int ret = m_serial->write(buffer, len);
	if (ret != len)
		return false;

	unsigned int count = 0U;
	RESP_TYPE_MMDVM resp;
	do {
		usleep(10 * 1000);

		resp = getResponse();
		if (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK) {
			count++;
			if (count >= MAX_RESPONSES) {
				LogError("The MMDVM is not responding to the SET_FREQ command");
				return false;
			}
		}
	} while (resp == RTM_OK && m_buffer[2U] != MMDVM_ACK && m_buffer[2U] != MMDVM_NAK);

	// CUtils::dump(1U, "Response", m_buffer, m_length);

	if (resp == RTM_OK && m_buffer[2U] == MMDVM_NAK) {
		LogError("Received a NAK to the SET_FREQ command from the modem");
		return false;
	}

	return true;
}

RESP_TYPE_MMDVM CModem::getResponse()
{
	assert(m_serial != NULL);

	if (m_offset == 0U) {
		// Get the start of the frame or nothing at all
		int ret = m_serial->read(m_buffer + 0U, 1U);
		if (ret < 0) {
			LogError("Error when reading from the modem");
			return RTM_ERROR;
		}

		if (ret == 0)
			return RTM_TIMEOUT;

		if (m_buffer[0U] != MMDVM_FRAME_START)
			return RTM_TIMEOUT;

		m_offset = 1U;
	}

	if (m_offset == 1U) {
		// Get the length of the frame
		int ret = m_serial->read(m_buffer + 1U, 1U);
		if (ret < 0) {
			LogError("Error when reading from the modem");
			m_offset = 0U;
			return RTM_ERROR;
		}

		if (ret == 0)
			return RTM_TIMEOUT;

		if (m_buffer[1U] >= 250U) {
			LogError("Invalid length received from the modem - %u", m_buffer[1U]);
			m_offset = 0U;
			return RTM_ERROR;
		}

		m_length = m_buffer[1U];
		m_offset = 2U;
	}

	if (m_offset == 2U) {
		// Get the frame type
		int ret = m_serial->read(m_buffer + 2U, 1U);
		if (ret < 0) {
			LogError("Error when reading from the modem");
			m_offset = 0U;
			return RTM_ERROR;
		}

		if (ret == 0)
			return RTM_TIMEOUT;

		m_offset = 3U;
	}

	if (m_offset >= 3U) {
		// Use later two byte length field
		if (m_length == 0U) {
			int ret = m_serial->read(m_buffer + 3U, 2U);
			if (ret < 0) {
				LogError("Error when reading from the modem");
				m_offset = 0U;
				return RTM_ERROR;
			}

			if (ret == 0)
				return RTM_TIMEOUT;

			m_length = (m_buffer[3U] << 8) | m_buffer[4U];
			m_offset = 5U;
		}

		while (m_offset < m_length) {
			int ret = m_serial->read(m_buffer + m_offset, m_length - m_offset);
			if (ret < 0) {
				LogError("Error when reading from the modem");
				m_offset = 0U;
				return RTM_ERROR;
			}

			if (ret == 0)
				return RTM_TIMEOUT;

			if (ret > 0)
				m_offset += ret;
		}
	}

	m_offset = 0U;

	// CUtils::dump(1U, "Received", m_buffer, m_length);

	return RTM_OK;
}

const char* CModem::getHWType() const
{
	return m_hwType;
}

bool CModem::setMode(unsigned char mode)
{
	assert(m_serial != NULL);

	unsigned char buffer[4U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = 4U;
	buffer[2U] = MMDVM_SET_MODE;
	buffer[3U] = mode;

	// CUtils::dump(1U, "Written", buffer, 4U);

	return m_serial->write(buffer, 4U) == 4;
}

bool CModem::sendCWId(const std::string& callsign)
{
	assert(m_serial != NULL);

	unsigned int length = callsign.length();
	if (length > 200U)
		length = 200U;

	unsigned char buffer[205U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = length + 3U;
	buffer[2U] = MMDVM_SEND_CWID;

	for (unsigned int i = 0U; i < length; i++)
		buffer[i + 3U] = callsign.at(i);

	// CUtils::dump(1U, "Written", buffer, length + 3U);

	return m_serial->write(buffer, length + 3U) == int(length + 3U);
}

bool CModem::writeDMRStart(bool tx)
{
	assert(m_serial != NULL);

	if (tx && m_tx)
		return true;
	if (!tx && !m_tx)
		return true;

	unsigned char buffer[4U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = 4U;
	buffer[2U] = MMDVM_DMR_START;
	buffer[3U] = tx ? 0x01U : 0x00U;

	// CUtils::dump(1U, "Written", buffer, 4U);

	return m_serial->write(buffer, 4U) == 4;
}

bool CModem::writeDMRAbort(unsigned int slotNo)
{
	assert(m_serial != NULL);

	if (slotNo == 1U)
		m_txDMRData1.clear();
	else
		m_txDMRData2.clear();

	unsigned char buffer[4U];

	buffer[0U] = MMDVM_FRAME_START;
	buffer[1U] = 4U;
	buffer[2U] = MMDVM_DMR_ABORT;
	buffer[3U] = slotNo;

	// CUtils::dump(1U, "Written", buffer, 4U);

	return m_serial->write(buffer, 4U) == 4;
}

bool CModem::writeDMRShortLC(const unsigned char* lc)
{
	assert(m_serial != NULL);
	assert(lc != NULL);

	unsigned char buffer[12U];

	buffer[0U]  = MMDVM_FRAME_START;
	buffer[1U]  = 12U;
	buffer[2U]  = MMDVM_DMR_SHORTLC;
	buffer[3U]  = lc[0U];
	buffer[4U]  = lc[1U];
	buffer[5U]  = lc[2U];
	buffer[6U]  = lc[3U];
	buffer[7U]  = lc[4U];
	buffer[8U]  = lc[5U];
	buffer[9U]  = lc[6U];
	buffer[10U] = lc[7U];
	buffer[11U] = lc[8U];

	// CUtils::dump(1U, "Written", buffer, 12U);

	return m_serial->write(buffer, 12U) == 12;
}

void CModem::printDebug()
{
	if (m_buffer[2U] == MMDVM_DEBUG1) {
		LogMessage("Debug: %.*s", m_length - 3U, m_buffer + 3U);
	} else if (m_buffer[2U] == MMDVM_DEBUG2) {
		short val1 = (m_buffer[m_length - 2U] << 8) | m_buffer[m_length - 1U];
		LogMessage("Debug: %.*s %d", m_length - 5U, m_buffer + 3U, val1);
	} else if (m_buffer[2U] == MMDVM_DEBUG3) {
		short val1 = (m_buffer[m_length - 4U] << 8) | m_buffer[m_length - 3U];
		short val2 = (m_buffer[m_length - 2U] << 8) | m_buffer[m_length - 1U];
		LogMessage("Debug: %.*s %d %d", m_length - 7U, m_buffer + 3U, val1, val2);
	} else if (m_buffer[2U] == MMDVM_DEBUG4) {
		short val1 = (m_buffer[m_length - 6U] << 8) | m_buffer[m_length - 5U];
		short val2 = (m_buffer[m_length - 4U] << 8) | m_buffer[m_length - 3U];
		short val3 = (m_buffer[m_length - 2U] << 8) | m_buffer[m_length - 1U];
		LogMessage("Debug: %.*s %d %d %d", m_length - 9U, m_buffer + 3U, val1, val2, val3);
	} else if (m_buffer[2U] == MMDVM_DEBUG5) {
		short val1 = (m_buffer[m_length - 8U] << 8) | m_buffer[m_length - 7U];
		short val2 = (m_buffer[m_length - 6U] << 8) | m_buffer[m_length - 5U];
		short val3 = (m_buffer[m_length - 4U] << 8) | m_buffer[m_length - 3U];
		short val4 = (m_buffer[m_length - 2U] << 8) | m_buffer[m_length - 1U];
		LogMessage("Debug: %.*s %d %d %d %d", m_length - 11U, m_buffer + 3U, val1, val2, val3, val4);
	}
}

CModem* CModem::createModem(const std::string& port, bool duplex, bool rxInvert, bool txInvert, bool pttInvert, unsigned int txDelay, unsigned int dmrDelay, bool trace, bool debug){
	if (port == "NullModem")
		return new CNullModem(port, duplex, rxInvert, txInvert, pttInvert, txDelay, dmrDelay, trace, debug);
	else
		return new CModem(port, duplex, rxInvert, txInvert, pttInvert, txDelay, dmrDelay, trace, debug);
}
