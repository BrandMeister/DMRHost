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

#include "NullDisplay.h"

CNullDisplay::CNullDisplay() :
CDisplay()
{
}

CNullDisplay::~CNullDisplay()
{
}

bool CNullDisplay::open()
{
	return true;
}

void CNullDisplay::setIdleInt()
{
}

void CNullDisplay::setErrorInt(const char* text)
{
}

void CNullDisplay::setQuitInt()
{
}

void CNullDisplay::writeDMRInt(unsigned int slotNo, const std::string& src, bool group, const std::string& dst, const char* type)
{
}

void CNullDisplay::clearDMRInt(unsigned int slotNo)
{
}

void CNullDisplay::writePOCSAGInt(uint32_t ric, const std::string& message)
{
}

void CNullDisplay::clearPOCSAGInt()
{
}

void CNullDisplay::writeCWInt()
{
}

void CNullDisplay::clearCWInt()
{
}

void CNullDisplay::close()
{
}
