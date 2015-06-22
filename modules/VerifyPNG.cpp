
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


#include "VerifyPNG.h"


VerifyPNG::VerifyPNG()
{

}

VerifyPNG::~VerifyPNG()
{

}

static bool verifyFileSignature(uint64_t startOffset, uint64_t remainingLength, FILE* image) 
{
    if(remainingLength < 8)
    {
        fprintf(stderr, METHODNAME(verifyFileSignature)"Not enough space left for the PNG signature to fit.\n");
        return false;
    }

    if(fseek(image, startOffset, SEEK_SET) != 0) 
    {
        perror(METHODNAME(verifyFileSignature)"fseek");
        return false;
    }
    char buffer[8];
    if(fread(buffer, 8, sizeof(unsigned char), image) == 0) 
    {
        perror(METHODNAME(verifyFileSignature)"fread");
        return false;
    }
    if(memcmp("\x89PNG\r\n\x1a\n", buffer, 8) != 0) 
    {
        fprintf(stderr, METHODNAME(verifyFileSignature)"The PNG signature did not match the expected value.\n");
        return false;
    }
    return true;
}

static uint32_t getChunkDataLength(PNG_Chunk_Header* header)
{
    return be32toh(header->Length);
}
static uint32_t getChunkLength(PNG_Chunk_Header* header)
{
    return getChunkDataLength(header) + 12;
}


static bool verifyHeaderType(PNG_Chunk_Header* header)
{
    for(int i = 0; i < 4; i++)
    {
        char c = header->Type[i];
        if( !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ) )
        {
            return false;
        }
    }
    return true;
}

static bool readAndVerifyChunk(uint64_t offset, uint64_t remainingLength, FILE* image, PNG_Chunk_Header* header) 
{
    if (fseek(image, offset, SEEK_SET) != 0) 
    {
	    perror(METHODNAME(readAndVerifyChunk)"fseek");
	    return 0;
	} 	
	if(fread(header, sizeof(char), 8, image) != sizeof(PNG_Chunk_Header) )
	{
	    if(feof(image))
	    {
	        fprintf(stderr, METHODNAME(readAndVerifyChunk)"End of file reached, not enough space for next PNG Header.\n");
	    }
	    else if(ferror(image))
	    {
		    perror(METHODNAME(readAndVerifyChunk)"fread");
		}
		return 0;
	}
	if(!verifyHeaderType(header))
	{
	    fprintf(stderr, METHODNAME(readAndVerifyChunk)"Header type %02x %02x %02x %02x is invalid.\n", header->Type[0], header->Type[1], header->Type[2], header->Type[3]);
	    return 0;
	}
	uint32_t chunkLength = getChunkDataLength(header);
	if(chunkLength + 12 > remainingLength)
	{
	    unsigned char temp[5];
	    memcpy(temp, &header->Type, 4);
	    temp[4] = 0;
	    fprintf(stderr, METHODNAME(readAndVerifyChunk)"Chunk \"%s\" specifies a data length of %d + 12 Byte Header but we only have %ld bytes left.\n", temp, chunkLength, remainingLength);
	    return 0;
	}
	
	// Test checksum
	uint32_t crc = start_crc();
	
	// Checksum includes Type field, but not length field.
	crc = update_crc(crc, (unsigned char*)&header->Type, sizeof(header->Type));
	
	unsigned char buf[SCAN_BUF_SIZE];
	uint64_t interval = 0;
	while(interval < chunkLength)
	{
	    uint64_t remainingToBeRead = chunkLength - interval;
	    size_t numBytesRead = fread(buf, sizeof(unsigned char), (remainingToBeRead > SCAN_BUF_SIZE) ? SCAN_BUF_SIZE : remainingToBeRead, image);
	    if(numBytesRead == 0)
	    {
	        if(feof(image))
	        {
	            fprintf(stderr, METHODNAME(readAndVerifyChunk)"Reached eof while trying to read PNG Chunk data.\n");
	            return 0;
	        }
	        if(ferror(image))
	        {
	            perror(METHODNAME(readAndVerifyChunk)"fread chunk data");
	        }
	        return 0;
	    }
	    crc = update_crc(crc, buf, numBytesRead);
	    interval += numBytesRead;
	}
	if(interval != chunkLength)
	{
	    fprintf(stderr, METHODNAME(readAndVerifyChunk)"Weird error, after reading all chunk data we expect having read %d, but actually read %ld\n", chunkLength, interval);
	    return 0;
	}
	crc = end_crc(crc);
	
	uint32_t checksumBytes;
	if(fread(&checksumBytes, sizeof(uint32_t), 1, image) != 1)
	{
	    if(feof(image))
	    {
	        fprintf(stderr, METHODNAME(readAndVerifyChunk)"Reached EOF while trying to read chunk crc checksum.\n");
	    }
	    if(ferror(image))
	    {
	        perror(METHODNAME(readAndVerifyChunk)"fread chunk checksum");
	    }
	    return 0;
	}
	uint32_t checksum = be32toh(checksumBytes);
	if(checksum != crc)
	{
	    fprintf(stderr, METHODNAME(readAndVerifyChunk)"Calculated checksum 0x%x, file says 0x%x.\n", crc, checksum);
	    return 0;
	}
	return true;
}


uint64_t VerifyPNG::getValidFileLength(uint64_t startOffset, uint64_t length, FILE* image){
	// 44 = 8 + 12 + 12 + 12 ? (FileSignature + IHDRChunk + IDATChunk + IENDChunk)
	if(length < 44) 
	{
		return 0;
	}
	if(!verifyFileSignature(startOffset, length, image))
	{
		return 0;
	}
	uint64_t index = 8;     // Skip signature
	uint64_t currentChunk = 0;
	bool foundDataChunk;
	PNG_Chunk_Header png_header;
	while( index < length )
	{ 
	    LOG_DEBUG(METHODNAME(getValidFileLength)"Verifying Chunk #%ld at %lx\n", currentChunk, startOffset + index);
	    
        if(!readAndVerifyChunk(startOffset + index, length - index, image, &png_header))
        {
            return 0;
        }
        
        uint32_t chunkLength = getChunkLength(&png_header);
        LOG_DEBUG(METHODNAME(getValidFileLength)"Chunk #%ld at %lx has length %d\n", currentChunk, startOffset + index, chunkLength);
        if(currentChunk == 0)
        {
            if(strncasecmp((char*)&png_header.Type, "IHDR", 4) != 0)
            {
                unsigned char temp[5];
	            memcpy(temp, &png_header.Type, 4);
	            temp[4] = 0;
                fprintf(stderr, METHODNAME(getValidFileLength)"The first chunk header has \"%s\" type. The first chunk has to have IHDR instead.\n", temp);
                return 0;
            }
        }
        
        if(strncasecmp((char*)&png_header.Type, "IDAT", 4) == 0)
        {
            foundDataChunk = true;
        }
        
        index += chunkLength;
        if(strncasecmp((char*)&png_header.Type, "IEND", 4) == 0)
        {
            if(!foundDataChunk)
            {
                fprintf(stderr, METHODNAME(getValidFileLength)"The PNG file does not contain an IDAT chunk, this is invalid!\n");
                return 0;
            }
            LOG_DEBUG(METHODNAME(getValidFileLength)"Found valid PNG file at %lx of length %ld\n", startOffset, index);
            return index;
        }
        
        currentChunk ++;
	}

    fprintf(stderr, METHODNAME(getValidFileLength)"Did not find a valid IEND header within %ld bytes from %lx\n", length, startOffset);
	return 0;
}
