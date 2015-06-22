
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




#ifndef FILE_VERIFIER_H
#define FILE_VERIFIER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

class FileVerifier
{
    public:
	/*
	 *	This is the method that identifies whether a specific region in our image is a valid file of our type.
	 *	startOffset	-	This is the offset where the header found by our search algorithm was found
	 *	length		-	The number of bytes to consider, including the footer.
	 *	myImageFile	-	The FILE pointer to use for reading the image file.
	 *	@return		-	returns the number of bytes the file actually occupies. This can at most be length. On Error 0 will be returned;
	 */
        virtual uint64_t getValidFileLength(uint64_t startOffset, uint64_t length, FILE* myImageFile) = 0;
};

#endif


