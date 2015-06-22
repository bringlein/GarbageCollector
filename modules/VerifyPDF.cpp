
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



#include "VerifyPDF.h"

VerifyPDF::VerifyPDF()
{

}

VerifyPDF::~VerifyPDF()
{

}


static bool verifyHeader(uint64_t startOffset, FILE* myImageFile) 
{
    if (fseek(myImageFile, startOffset, SEEK_SET) != 0)
    {
        return false;
    }
    char buffer[8];
    if(fread(buffer, sizeof(char), 8, myImageFile) == 0)
    {
        perror(METHODNAME(verifyHeader)"fread");
        return false;
    }
    if(strncmp("%PDF-1.", buffer, 7) != 0) 
    {
        return false;
    }
    if(!(buffer[7] >= '0' && buffer[7] <= '7'))
    {
        return false;
    }
    return true;
}

static bool verifyObjInFirst100Bytes(uint64_t startOffset, uint64_t length, FILE* image)
{
    if (fseek(image, startOffset, SEEK_SET) != 0)
    {
        perror(METHODNAME(verifyObjInFirst100Bytes)"fseek failed");
        return false;
    }
    char buffer[201];
    size_t numBytesRead = fread(buffer, sizeof(char), ((length < 200) ? length : 200), image);
    if(numBytesRead == 0)
    {
        perror(METHODNAME(verifyObjInFirst100Bytes)"fread");
        return false;
    }
    buffer[numBytesRead] = 0;
    buffer[200] = 0;
    if(strstr(buffer, "obj") == NULL) 
    {
        fprintf(stderr, METHODNAME(verifyObjInFirst100Bytes)"Did not find \"obj\" within first 100 Bytes of PDF document.\n");
        return false;
    }
    return true;
}

static bool verifyEnd(uint64_t startOffset, uint64_t length, FILE* image)
{
	fseek(image, startOffset + length - 6, SEEK_SET);
    int buffer = fgetc(image);
    if(buffer == EOF)
    {
        perror(METHODNAME(verifyEnd)"fgetc trying to verify end failed");
        return false;
    } 
    unsigned char c = (unsigned char)buffer;
	if(c != '\n' && c != '\r') 
	{ 
        return false;
    }
	return true;
}

static bool verifyTrailer(uint64_t startOffset, uint64_t length, FILE *myImageFile) 
{
    bool trailerFound = false;
    char buf;
    char buf2[7];
    for (int i = 1; i < (int)length+1; i++) 
    {
        if (fseek(myImageFile,startOffset+length-i,SEEK_SET) != 0)
        {
	        return false;
	    }
	    if (fread(&buf,1,1,myImageFile) == 0)
	    {
	        return false;
	    }
	    if (!(buf == 't' && i > 8)) 
	    {
	        continue;
	    }
	    if (fread(buf2,1,7,myImageFile) == 0) 
	    {
	        return false;
	    }
	    if (buf2[6] != '\r' && buf2[6] != '\n') 
	    {
	        continue;
	    }
	    buf2[6] = '\0';
	    if (strncmp(buf2,"railer",7) == 0) 
	    {
	        trailerFound = true;
	        break;
	    }
    }
    if(trailerFound) 
    {
        return true;
    }
    return false;
}

static bool verifyStartXref(uint64_t startOffset, uint64_t length, FILE *myImageFile)
{
	bool startxrefFound = false;
    char buf;
    char buf2[9];
    for (int i = 1; i < (int)length+1; i++) 
    {
        if (fseek(myImageFile,startOffset+length-i,SEEK_SET) != 0)
        {
	        return false;
	    }
	    if (fread(&buf,1,1,myImageFile) == 0)
	    {
	        return false;
	    }
	    if (!(buf == 's' && i > 10)) 
	    {
	        continue;
	    }
	    if (fread(buf2,1,9,myImageFile) == 0) 
	    {
	        return false;
	    }
	    if (buf2[8] != '\r' && buf2[8] != '\n') 
	    {
	        continue;
	    }
	    buf2[8] = '\0';
	    if (strncmp(buf2,"tartxref",9) == 0) 
	    {
	        startxrefFound = true;
	        break;
	    }
    }
    if(startxrefFound) 
    {
        return true;
    }
    return false;

}


uint64_t VerifyPDF::getValidFileLength(uint64_t startOffset, uint64_t length, FILE* myImageFile)
{
    LOG_DEBUG(METHODNAME(getValidFileLength)"Verifying PDF at 0x%lx of length 0d%ld\n", startOffset, length);


    //header+startxref+xref+trailer
    if(length < 24)
	{
	    std::cerr << METHODNAME(getValidFileLength)"Valid PDF files should not be able to fit into 24 bytes length.";
		return 0;
	}
	if(!verifyHeader(startOffset, myImageFile))
	{
		return 0;
	}
	if(!verifyObjInFirst100Bytes(startOffset, length, myImageFile))
	{
	    return 0;
	}
	if(!verifyEnd(startOffset, length, myImageFile)) 
	{
		return 0;
	}
	
    if ( !(verifyTrailer(startOffset,length,myImageFile)) && !(verifyStartXref(startOffset, length, myImageFile)) )
    {
        std::cerr << METHODNAME(verifyTrailer)"no startxref and no trailer found" << std::endl;
        return 0;
    }
	return length;
}

