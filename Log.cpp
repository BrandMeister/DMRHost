/*
 *   Copyright (C) 2015,2016,2020 by Jonathan Naylor G4KLX
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

#include "Log.h"

#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cassert>
#include <cstring>
#include <syslog.h>

static unsigned int m_fileLevel = 2U;
static std::string m_filePath;
static std::string m_fileRoot;
static bool m_fileRotate = true;

static FILE* m_fpLog = NULL;

static unsigned int m_syslogLevel = 2U;
static unsigned int m_displayLevel = 2U;

static struct tm m_tm;

static char LEVELS[] = " DMIWEF";

static bool logOpenRotate()
{
	bool status = false;
	
	if (m_fileLevel == 0U)
		return true;

	time_t now;
	::time(&now);

	struct tm* tm = ::gmtime(&now);

	if (tm->tm_mday == m_tm.tm_mday && tm->tm_mon == m_tm.tm_mon && tm->tm_year == m_tm.tm_year) {
		if (m_fpLog != NULL)
		    return true;
	} else {
		if (m_fpLog != NULL)
			::fclose(m_fpLog);
	}

	char filename[200U];
	::sprintf(filename, "%s/%s-%04d-%02d-%02d.log", m_filePath.c_str(), m_fileRoot.c_str(), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

	if ((m_fpLog = ::fopen(filename, "a+t")) != NULL) {
		status = true;
	}
	
	m_tm = *tm;

	return status;
}

static bool logOpenNoRotate()
{
	bool status = false;

	if (m_fileLevel == 0U)
		return true;

	if (m_fpLog != NULL)
		return true;

	char filename[200U];
	::sprintf(filename, "%s/%s.log", m_filePath.c_str(), m_fileRoot.c_str());

	if ((m_fpLog = ::fopen(filename, "a+t")) != NULL) {
		status = true;
	}

	return status;
}

bool LogOpen()
{
	if (m_fileRotate)
		return logOpenRotate();
	else
		return logOpenNoRotate();
}

bool LogInitialise(const std::string& filePath, const std::string& fileRoot, unsigned int fileLevel, unsigned int displayLevel, unsigned int syslogLevel, bool rotate)
{
	m_filePath     = filePath;
	m_fileRoot     = fileRoot;
	m_fileLevel    = fileLevel;
	m_displayLevel = displayLevel;
	m_syslogLevel  = syslogLevel;
	m_fileRotate   = rotate;

	return ::LogOpen();
}

void LogFinalise()
{
	if (m_fpLog != NULL)
		::fclose(m_fpLog);
}

void Log(unsigned int level, const char* fmt, ...)
{
	assert(fmt != NULL);

	char buffer[501U];
	struct timeval now;
	::gettimeofday(&now, NULL);

	struct tm* tm = ::gmtime(&now.tv_sec);

	::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	va_list vl;
	va_start(vl, fmt);

	::vsnprintf(buffer + ::strlen(buffer), 500, fmt, vl);

	va_end(vl);

	if (level >= m_fileLevel && m_fileLevel != 0U) {
		bool ret = ::LogOpen();
		if (!ret)
			return;

		::fprintf(m_fpLog, "%s\n", buffer);
		::fflush(m_fpLog);
	}

	if (level >= m_displayLevel && m_displayLevel != 0U) {
		::fprintf(stdout, "%s\n", buffer);
		::fflush(stdout);
	}

	if (level >= m_syslogLevel && m_syslogLevel != 0U) {
		syslog(LOG_INFO, "DMRHost: %s\n", buffer);
	}

	if (level == 6U) {		// Fatal
		::fclose(m_fpLog);
		exit(1);
	}
}
