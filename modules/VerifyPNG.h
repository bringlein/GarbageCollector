
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

#include <string.h>
#include <strings.h>
#include <iostream>
#include <stdbool.h>
#include <endian.h>

#include "../crc.h"
#include "../util.h"

#define SCAN_BUF_SIZE 1000

#undef MODULENAME
#define MODULENAME DEBUGINFO(VerifyPNG,)

#undef METHODNAME
#define METHODNAME(method) DEBUGINFO(VerifyPNG, method)


struct PNG_Chunk_Header
{
    uint32_t    Length;     // CAREFUL!!!!!!! This is big-endian!!!!!!
    uint8_t     Type[4];
};

class VerifyPNG	:	FileVerifier
{
    public:
        VerifyPNG();
	    ~VerifyPNG();
	
	    uint64_t getValidFileLength(uint64_t startOffset, uint64_t length, FILE* myImageFile);
};
