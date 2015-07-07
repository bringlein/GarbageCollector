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



#include "VerifySQLite.h"

VerifySQLite::VerifySQLite()
{

}

VerifySQLite::~VerifySQLite()
{

}


static bool isPowerOfTwo(unsigned int number) {
	while ((number % 2 == 0) && number > 1) {
		number /= 2;
	}
	return number == 1;
}


static unsigned int asUnsignedInt(char* header, unsigned int offset, unsigned int length) {
	// length must not be longer than 4 to match unsigned int
	if (length > 4) {
		std::cerr << "[VerifySQLite] method: asUnsignedInt: length must not be longer than 4 bytes" << std::endl;
	}

	unsigned int val = (unsigned int) header[offset];
	for (unsigned int i=1; i<length; i++) {
		val = val << 8;	// as this is big endian, shift current number by one byte
		val = val + (unsigned int) header[offset + i];
	}

	return val;
}


static bool assertEquals(char* header, unsigned int offset, unsigned int length, unsigned int val) {
	unsigned int read = asUnsignedInt(header, offset, length);
	return (read == val);
}


static bool verifyHeader(uint64_t startOffset, FILE* myImageFile, uint64_t& length) {

	if (fseek(myImageFile, startOffset, SEEK_SET) != 0) {
		return false;
	}

	char buffer[100];
	if (0 == fread(buffer, sizeof(char), 100, myImageFile)) {
		perror(METHODNAME(verifyHeader)"fread");
		return false;
	}

	// Compare first 16 bytes to magic header string
	if (0 != strncmp("\x53\x51\x4c\x69\x74\x65\x20\x66\x6f\x72\x6d\x61\x74\x20\x33\x00", buffer, 16)) {
		return false;
	}

	unsigned int version = asUnsignedInt(buffer, 96, 4);
	unsigned int majorVersion = version / 1000000;
	unsigned int minorVersion = (version / 1000) % 1000;
	unsigned int releaseVersion = version % 1000;

	unsigned int pageSize = (((unsigned int) buffer[16]) << 8) + (unsigned int) buffer[17];
	// From version 3.7.1 a page size of 65536 bytes is encoded as magic 1
	if ((majorVersion > 3 || (majorVersion == 3 && minorVersion > 7) || (majorVersion == 3 && minorVersion == 7 && releaseVersion >= 1)) && 1 == pageSize) {
		pageSize = 65536;
	}
	else {
		// Otherwise, the page size must be a power of two between 512 and 32768
		if (pageSize < 512 || pageSize > 32768 || !isPowerOfTwo(pageSize)) {
			return false;
		}
	}

	// Maximum embedded payload fraction must be 64
	if (!assertEquals(buffer, 20, 1, 64)) {
		return false;
	}

	// Minimum embedded payload fraction must be 32
	if (!assertEquals(buffer, 21, 1, 32)) {
		return false;
	}

	// Leaf payload fraction must be 32
	if (!assertEquals(buffer, 22, 1, 32)) {
		return false;
	}

	// The number of pages consists of 4 bytes in big-endian byte order starting at offset 28
	unsigned int pages = asUnsignedInt(buffer, 28, 4);

	// In-header database size is always valid when the database is only modified using recent version of SQLite (versions 3.7.0) and later
	if (majorVersion < 3 || (majorVersion == 3 && minorVersion < 7)) {

		// defined to be non-zero
		if (0 == pages) {
			return false;
		}

		unsigned int changeCounter = asUnsignedInt(buffer, 24, 4);
		unsigned int versionValidForNumber = asUnsignedInt(buffer, 92, 4);
		if (changeCounter != versionValidForNumber) {
			return false;
		}
	}

	length = ((uint64_t) pageSize) * ((uint64_t) pages);
	return true;
}



uint64_t VerifySQLite::getValidFileLength(uint64_t startOffset, uint64_t maxLength, FILE* myImageFile)
{
	LOG_DEBUG(METHODNAME(getValidFileLength)"Verifying SQLite at 0x%lx of length %ld\n", startOffset, length);

	uint64_t length = maxLength;
	if (length < 100) {
		// First 100 bytes correspond to header
		return 0;
	}

	if (!verifyHeader(startOffset, myImageFile, length)) {
		LOG_DEBUG(METHODNAME(verifyHeader)"failed\n");
		return 0;
	}
	// length has now been set by verifyHeader

	// maxLength points to the end of the image file or device
	if (length > maxLength) {
		return 0;
	}

	return length;
}
