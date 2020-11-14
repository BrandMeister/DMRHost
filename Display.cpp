/*
 *   Copyright (C) 2016,2017,2018,2020 by Jonathan Naylor G4KLX
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

#include "Display.h"
#include "Defines.h"
#include "SerialController.h"
#include "ModemSerialPort.h"
#include "NetDisplay.h"
#include "NullDisplay.h"
#include "TFTSurenoo.h"
#include "LCDproc.h"
#include "Nextion.h"
#include "CASTInfo.h"
#include "Conf.h"
#include "Modem.h"
#include "Log.h"

#if defined(OLED)
#include "OLED.h"
#endif

#include <cstdio>
#include <cassert>
#include <cstring>

CDisplay::CDisplay() :
m_timer1(3000U, 3U),
m_timer2(3000U, 3U),
m_mode1(MODE_IDLE),
m_mode2(MODE_IDLE)
{
}

CDisplay::~CDisplay()
{
}

void CDisplay::setIdle()
{
	m_timer1.stop();
	m_timer2.stop();

	m_mode1 = MODE_IDLE;
	m_mode2 = MODE_IDLE;

	setIdleInt();
}

void CDisplay::setError(const char* text)
{
	assert(text != NULL);

	m_timer1.stop();
	m_timer2.stop();

	m_mode1 = MODE_IDLE;
	m_mode2 = MODE_IDLE;

	setErrorInt(text);
}

void CDisplay::setQuit()
{
	m_timer1.stop();
	m_timer2.stop();

	m_mode1 = MODE_QUIT;
	m_mode2 = MODE_QUIT;

	setQuitInt();
}

void CDisplay::writeDMR(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type)
{
	assert(type != NULL);

	if (slotNo == 1U) {
		m_timer1.start();
		m_mode1 = MODE_IDLE;
	} else {
		m_timer2.start();
		m_mode2 = MODE_IDLE;
	}
	writeDMRInt(slotNo, src, group, dst, type);
}

void CDisplay::writeDMRRSSI(unsigned int slotNo, unsigned char rssi)
{
	if (rssi != 0U)
		writeDMRRSSIInt(slotNo, rssi);
}

void CDisplay::writeDMRTA(unsigned int slotNo, unsigned char* talkerAlias, const char* type)
{
    if (strcmp(type," ")==0) { writeDMRTAInt(slotNo, (unsigned char*)"", type); return; }
    if (strlen((char*)talkerAlias)>=4U) writeDMRTAInt(slotNo, (unsigned char*)talkerAlias, type);
}

void CDisplay::writeDMRBER(unsigned int slotNo, float ber)
{
	writeDMRBERInt(slotNo, ber);
}

void CDisplay::clearDMR(unsigned int slotNo)
{
	if (slotNo == 1U) {
		if (m_timer1.hasExpired()) {
			clearDMRInt(slotNo);
			m_timer1.stop();
			m_mode1 = MODE_IDLE;
		} else {
			m_mode1 = MODE_DMR;
		}
	} else {
		if (m_timer2.hasExpired()) {
			clearDMRInt(slotNo);
			m_timer2.stop();
			m_mode2 = MODE_IDLE;
		} else {
			m_mode2 = MODE_DMR;
		}
	}
}

void CDisplay::writePOCSAG(uint32_t ric, const std::string& message)
{
	m_timer1.start();
	m_mode1 = MODE_POCSAG;

	writePOCSAGInt(ric, message);
}

void CDisplay::clearPOCSAG()
{
	if (m_timer1.hasExpired()) {
		clearPOCSAGInt();
		m_timer1.stop();
		m_mode1 = MODE_IDLE;
	} else {
		m_mode1 = MODE_POCSAG;
	}
}

void CDisplay::writeCW()
{
	m_timer1.start();
	m_mode1 = MODE_CW;

	writeCWInt();
}

void CDisplay::clock(unsigned int ms)
{
	m_timer1.clock(ms);
	if (m_timer1.isRunning() && m_timer1.hasExpired()) {
		switch (m_mode1) {
		case MODE_DMR:
			clearDMRInt(1U);
			m_mode1 = MODE_IDLE;
			m_timer1.stop();
			break;
		case MODE_POCSAG:
			clearPOCSAGInt();
			m_mode1 = MODE_IDLE;
			m_timer1.stop();
			break;
		case MODE_CW:
			clearCWInt();
			m_mode1 = MODE_IDLE;
			m_timer1.stop();
			break;
		default:
			break;
		}
	}

	// Timer/mode 2 are only used for DMR
	m_timer2.clock(ms);
	if (m_timer2.isRunning() && m_timer2.hasExpired()) {
		if (m_mode2 == MODE_DMR) {
			clearDMRInt(2U);
			m_mode2 = MODE_IDLE;
			m_timer2.stop();
		}
	}

	clockInt(ms);
}

void CDisplay::clockInt(unsigned int ms)
{
}

void CDisplay::writeDMRRSSIInt(unsigned int slotNo, unsigned char rssi)
{
}

void CDisplay::writeDMRTAInt(unsigned int slotNo, unsigned char* talkerAlias, const char* type)
{
}

void CDisplay::writeDMRBERInt(unsigned int slotNo, float ber)
{
}

/* Factory method extracted from MMDVMHost.cpp - BG5HHP */
CDisplay* CDisplay::createDisplay(const CConf& conf, CModem* modem)
{
        CDisplay *display = NULL;

        std::string type   = conf.getDisplay();
	unsigned int dmrid = conf.getDMRId();

	LogInfo("Display Parameters");
	LogInfo("    Type: %s", type.c_str());

	if (type == "TFT Surenoo") {
		std::string port        = conf.getTFTSerialPort();
		unsigned int brightness = conf.getTFTSerialBrightness();

		LogInfo("    Port: %s", port.c_str());
		LogInfo("    Brightness: %u", brightness);

		ISerialPort* serial = NULL;
		if (port == "modem")
			serial = new CModemSerialPort(modem);
		else
			serial = new CSerialController(port, 115200);

		display = new CTFTSurenoo(conf.getCallsign(), dmrid, serial, brightness, conf.getDuplex());
	} else if (type == "Nextion") {
		std::string port            = conf.getNextionPort();
		unsigned int brightness     = conf.getNextionBrightness();
		bool displayClock           = conf.getNextionDisplayClock();
		bool utc                    = conf.getNextionUTC();
		unsigned int idleBrightness = conf.getNextionIdleBrightness();
		unsigned int screenLayout   = conf.getNextionScreenLayout();
		unsigned int txFrequency    = conf.getTXFrequency();
		unsigned int rxFrequency    = conf.getRXFrequency();
		bool displayTempInF         = conf.getNextionTempInFahrenheit();

		LogInfo("    Port: %s", port.c_str());
		LogInfo("    Brightness: %u", brightness);
		LogInfo("    Clock Display: %s", displayClock ? "yes" : "no");
		if (displayClock)
			LogInfo("    Display UTC: %s", utc ? "yes" : "no");
		LogInfo("    Idle Brightness: %u", idleBrightness);
		LogInfo("    Temperature in Fahrenheit: %s ", displayTempInF ? "yes" : "no");
 
		switch (screenLayout) {
		case 0U:
			LogInfo("    Screen Layout: G4KLX (Default)");
			break;
		case 2U:
			LogInfo("    Screen Layout: ON7LDS");
			break;
		case 3U:
			LogInfo("    Screen Layout: DIY by ON7LDS");
			break;
		case 4U:
			LogInfo("    Screen Layout: DIY by ON7LDS (High speed)");
			break;
		default:
			LogInfo("    Screen Layout: %u (Unknown)", screenLayout);
			break;
		}

		if (port == "modem") {
			ISerialPort* serial = new CModemSerialPort(modem);
			display = new CNextion(conf.getCallsign(), dmrid, serial, brightness, displayClock, utc, idleBrightness, screenLayout, txFrequency, rxFrequency, displayTempInF);
		} else {
			unsigned int baudrate = 9600;
			if (screenLayout&0x0cU)
				baudrate = 115200;
			
			LogInfo("    Display baudrate: %u ",baudrate);
			ISerialPort* serial = new CSerialController(port, baudrate);
			display = new CNextion(conf.getCallsign(), dmrid, serial, brightness, displayClock, utc, idleBrightness, screenLayout, txFrequency, rxFrequency, displayTempInF);
		}
	} else if (type == "LCDproc") {
		std::string address       = conf.getLCDprocAddress();
		unsigned int port         = conf.getLCDprocPort();
		unsigned int localPort    = conf.getLCDprocLocalPort();
		bool displayClock         = conf.getLCDprocDisplayClock();
		bool utc                  = conf.getLCDprocUTC();
		bool dimOnIdle            = conf.getLCDprocDimOnIdle();

		LogInfo("    Address: %s", address.c_str());
		LogInfo("    Port: %u", port);

		if (localPort == 0 )
			LogInfo("    Local Port: random");
		else
			LogInfo("    Local Port: %u", localPort);

		LogInfo("    Dim Display on Idle: %s", dimOnIdle ? "yes" : "no");
		LogInfo("    Clock Display: %s", displayClock ? "yes" : "no");

		if (displayClock)
			LogInfo("    Display UTC: %s", utc ? "yes" : "no");

		display = new CLCDproc(address.c_str(), port, localPort, conf.getCallsign(), dmrid, displayClock, utc, conf.getDuplex(), dimOnIdle);
	} else if (type == "NetDisplay") {
	        std::string address = conf.getNetDisplayAddress();
	        unsigned int port   = conf.getNetDisplayPort();

		LogInfo("    Address: %s", address.c_str());
		LogInfo("    Port: %u", port);

		display = new CNetDisplay(address, port);
#if defined(OLED)
	} else if (type == "OLED") {
	        unsigned char type       = conf.getOLEDType();
	        unsigned char brightness = conf.getOLEDBrightness();
	        bool          invert     = conf.getOLEDInvert();
	        bool          scroll     = conf.getOLEDScroll();
		bool          rotate     = conf.getOLEDRotate();
		bool          logosaver  = conf.getOLEDLogoScreensaver();

		display = new COLED(type, brightness, invert, scroll, rotate, logosaver, conf.getDMRNetworkSlot1(), conf.getDMRNetworkSlot2());
#endif
	} else if (type == "CAST") {
		display = new CCASTInfo(modem);
	} else {
		LogWarning("No valid display found, disabling");
		display = new CNullDisplay;
	}

	bool ret = display->open();
	if (!ret) {
		delete display;
		display = new CNullDisplay;
	}

	return display;
}
