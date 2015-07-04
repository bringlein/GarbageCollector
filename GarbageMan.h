
/*
 * 
 * Copyright (c) 2015. Lukas Dresel, Julius Lohmann, Benedikt Lorch, Burkhard Ringlein, Andreas Rupp, Yuriy Sulima, Carola Touchy
 * 
 * This file is part of GarbageCollector. 
 * 
 *	 GarbageCollector is free software: you can redistribute it and/or modify
 * 	 it under the terms of the GNU General Public License as published by
 * 	 the Free Software Foundation, either version 3 of the License, or
 * 	 (at your option) any later version.
 * 
 * 	 GarbageCollector is distributed in the hope that it will be useful,
 * 	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	 GNU General Public License for more details.
 * 
 * 	 You should have received a copy of the GNU General Public License
 * 	 along with GarbageCollector.  If not, see <http://www.gnu.org/licenses/>.
 *
 */ 






#pragma once

#include <stack>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <map>
#include <inttypes.h>
#include <iostream>
#include "jbuffer.h"
#include "VerificationJob.h"

enum PatternType { pdfHeader, pdfFooter, jpgHeader, pngHeader, sqliteHeader};

inline std::ostream& operator<<(std::ostream& out, const PatternType value) {
	switch(value) {
		case pdfHeader:
			return out << "PDF Header";
		case pdfFooter:
			return out << "PDF Footer";
		case jpgHeader:
			return out << "JPG Header";
		case pngHeader:
			return out << "PNG Header";
		case sqliteHeader:
			return out << "SQLite Header";
	}
	return out;
}

/**
Garbage Man collects all files which look like pdf, jpg or png files
**/
class GarbageMan
{
	private:
		std::vector<uint64_t> pdfStart;

		static uint64_t rollingHash(uint64_t previousHash, char kickOut, char next, uint64_t length);
		static uint64_t initialHash(const char* const text, uint64_t length);

		uint64_t rabinKarp(const char* const text, std::map<PatternType, const char*> patterns, BNDBUF* jbuf, uint64_t fragmentLength, uint64_t fragmentOffset, uint64_t globalLength, uint64_t lastFoundAdressPar);
		static const uint64_t q = 179424673;	// 10 000 000th prime number

	public:
		GarbageMan();
		~GarbageMan();
		void work(const char* pathToImg, BNDBUF* jbuf);
};
