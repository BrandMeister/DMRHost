/*
 *	Copyright (C) 2009,2014,2015 by Jonathan Naylor, G4KLX
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 */

#pragma once

#include <string>

class CUtils {
public:
	static void dump(const std::string& title, const unsigned char* data, unsigned int length);
	static void dump(int level, const std::string& title, const unsigned char* data, unsigned int length);

	static void byteToBitsBE(unsigned char byte, bool* bits);
	static void bitsToByteBE(const bool* bits, unsigned char& byte);
};
