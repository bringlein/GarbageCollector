
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



#include "../FileVerifier.h"

#include <endian.h>
#include "../util.h"

#define INVALID_OFFSET ((uint64_t)-1)

#undef MODULENAME
#define MODULENAME DEBUGINFO(VerifyJPEG,)

#undef METHODNAME
#define METHODNAME(method) DEBUGINFO(VerifyJPEG, method)


struct JPEGSegment
{
	uint8_t		magic;
	uint8_t		type;
	uint16_t    length;  // CAREFUL!!!!!!! This is big-endian!!!!!!
};

class VerifyJPEG	:	FileVerifier
{
    public:
        VerifyJPEG();
	    ~VerifyJPEG();
	
	    uint64_t getValidFileLength(uint64_t startOffset, uint64_t length, FILE* myImageFile);
};
