
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


#include "VerifyJPEG.h"

VerifyJPEG::VerifyJPEG()
{
	
}
VerifyJPEG::~VerifyJPEG()
{

}

const char* getSegmentDescriptor(JPEGSegment* segment)
{
    // Reference: http://www.w3.org/Graphics/JPEG/itu-t81.pdf  (around Page 36)
    switch(segment->type)
    {
        case 0x00:  return "INVALID!!! This is not a valid segment, this is a stuff byte";
        case 0xFF:  return "INVALID!!! This should only be padding!";
        case 0x01:  return "TEM: Temporary private use in arithmetic coding";
    
        case 0xC0:  return "SOF0: ";
        case 0xC1:  return "SOF1: ";
        case 0xC2:  return "SOF2: ";
        case 0xC3:  return "SOF3: ";
        
        case 0xC5:  return "SOF5: ";
        case 0xC6:  return "SOF6: ";
        case 0xC7:  return "SOF7: ";
        
        case 0xC8:  return "JPG: Reserved for JPEG Extensions";
        case 0xC9:  return "SOF9: ";
        case 0xCA:  return "SOF10: ";
        case 0xCB:  return "SOF11: ";
        
        case 0xCD:  return "SOF13: ";
        case 0xCE:  return "SOF14: ";
        case 0xCF:  return "SOF15: ";
        
        case 0xC4:  return "DHT: Define Huffman table(s)";
        case 0xCC:  return "DAC: Define Arithmetic coding conditioning(s)";

        case 0xD0:  return "RST0: Restart with modulo 8 count";
        case 0xD1:  return "RST1: Restart with modulo 8 count";
        case 0xD2:  return "RST2: Restart with modulo 8 count";
        case 0xD3:  return "RST3: Restart with modulo 8 count";
        case 0xD4:  return "RST4: Restart with modulo 8 count";
        case 0xD5:  return "RST5: Restart with modulo 8 count";
        case 0xD6:  return "RST6: Restart with modulo 8 count";
        case 0xD7:  return "RST7: Restart with modulo 8 count";
        
        case 0xD8:  return "SOI: Start of Image";
        case 0xD9:  return "EOI: End of Image";
        case 0xDA:  return "SOS: Start of scan";
        case 0xDB:  return "DQT: Define quantization table";
        case 0xDC:  return "DNL: Define number of lines";
        case 0xDD:  return "DRI: Define restart interval";
        case 0xDE:  return "DHP: Define hierarchical progression";
        case 0xDF:  return "EXP: Expand reference component(s)";
        
        case 0xE0:  return "APP0: Application specific segment";
        case 0xE1:  return "APP1: Application specific segment";
        case 0xE2:  return "APP2: Application specific segment";
        case 0xE3:  return "APP3: Application specific segment";
        case 0xE4:  return "APP4: Application specific segment";
        case 0xE5:  return "APP5: Application specific segment";
        case 0xE6:  return "APP6: Application specific segment";
        case 0xE7:  return "APP7: Application specific segment";
        case 0xE8:  return "APP8: Application specific segment";
        case 0xE9:  return "APP9: Application specific segment";
        case 0xEA:  return "APP10: Application specific segment";
        case 0xEB:  return "APP11: Application specific segment";
        case 0xEC:  return "APP12: Application specific segment";
        case 0xED:  return "APP13: Application specific segment";
        case 0xEE:  return "APP14: Application specific segment";
        case 0xEF:  return "APP15: Application specific segment";
        
        case 0xF0:  return "JPG0: JPEG Extensions";
        case 0xF1:  return "JPG1: JPEG Extensions";
        case 0xF2:  return "JPG2: JPEG Extensions";
        case 0xF3:  return "JPG3: JPEG Extensions";
        case 0xF4:  return "JPG4: JPEG Extensions";
        case 0xF5:  return "JPG5: JPEG Extensions";
        case 0xF6:  return "JPG6: JPEG Extensions";
        case 0xF7:  return "JPG7: JPEG Extensions";
        case 0xF8:  return "JPG8: JPEG Extensions";
        case 0xF9:  return "JPG9: JPEG Extensions";
        case 0xFA:  return "JPG10: JPEG Extensions";
        case 0xFB:  return "JPG11: JPEG Extensions";
        case 0xFC:  return "JPG12: JPEG Extensions";
        case 0xFD:  return "JPG13: JPEG Extensions";
        
        case 0xFE:  return "COM: Comment";
        
        default:    break;
    }
    // We only get here if our segment type was not specified.
    if(segment->type >= 0x02 && segment->type <= 0xBF)
    {
        return "RES: Reserved";
    }
    fprintf(stderr, METHODNAME(getSegmentDescriptor)"Error, we found unknown segment type 0x%02x\n", segment->type);
    return "INVALID!!! This is not a valid segment, we have no idea what this is!";
}

static bool isValidSegmentType(JPEGSegment* segment)
{
    return (segment->type != 0x00 && segment->type != 0xff);
}
static bool isVariableSizeSegment(JPEGSegment* segment)
{
    return !((segment->type >= 0xD0 && segment->type <= 0xD9) || segment->type == 0x01);
}

static uint64_t getSegmentLength(JPEGSegment* segment)
{
    if(!isVariableSizeSegment(segment))
    {
        // These are the single opcodes, they don't have a real length encoded marker segment
        return 2;
    }
    uint16_t length = be16toh(segment->length);
    return length + 2;     // The segment length does not include segment magic and type.
}

static uint64_t scanForNextSegmentHeader(uint64_t startIndex, uint64_t remainingLength, FILE* image)
{
    fseek(image, startIndex, SEEK_SET);
    bool possibleHeaderFound = false;
    for(uint64_t index = 0; index < remainingLength; index ++)
    {
        int read = fgetc(image);
        if(read == EOF)
        {
            fprintf(stderr, METHODNAME(scanForNextSegmentHeader)"reached eof at file index %lx\n", startIndex + index);
            return INVALID_OFFSET;
        }
        unsigned char c = (unsigned char)read;
        if(possibleHeaderFound && c != 0x00)
        {
            if(c < 0xD0 || c > 0xD8)
            {
                return index - 1;
            }
        }
        possibleHeaderFound = (c == 0xFF);
    }
    fprintf(stderr, METHODNAME(scanForNextSegmentHeader)"Could not find next segment header scanning from %lx(%ld) for %ld bytes.\n", startIndex, startIndex, remainingLength);
    return INVALID_OFFSET;
}

static bool readAndValidateSegment(uint64_t offset, uint64_t remainingLength, FILE* image, JPEGSegment* segment)
{
    if(remainingLength < 2)
    {  
        // We need at least 2 bytes to have a valid jpeg segment header.
        return 0;
    }
    fseek(image, offset, SEEK_SET);
    if(!fread_check_error(METHODNAME(readAndValidateSegment), (unsigned char*)segment, 2, image))
    {
        return false;
    }
    if(segment->magic != (uint8_t)0xff)
    {
        fprintf(stderr, METHODNAME(readAndValidateSegment)"The header magic was supposed to be 0xff, found was 0x%02x\n", segment->magic);
        return false;
    }
    if(!isValidSegmentType(segment))
    {
        fprintf(stderr, METHODNAME(readAndValidateSegment)"The segment type 0x%02x is invalid\n", segment->type);
        return false;
    }
    if(!isVariableSizeSegment(segment))
    {
        // These are the single opcodes, they don't have a real length encoded marker segment
        return true;
    }
    if(remainingLength < 4)
    {
        fprintf(stderr, METHODNAME(readAndValidateSegment)"Not enough bytes left for a valid variable size segment.\n");
        return false;
    }
    if(!fread_check_error(METHODNAME(readAndValidateSegment), ((unsigned char*)segment) + 2, 2, image))
    {
        return false;
    }
    if(getSegmentLength(segment) > remainingLength)
    {
        // Our segment does not fit into the remaining byte range .
        fprintf(stderr, METHODNAME(readAndValidateSegment)"Segment too big to fit into the rest of the available memory.\n");
        return false;
    }
    if(getSegmentLength(segment) < 2)
    {
        fprintf(stderr, METHODNAME(readAndValidateSegment)"The segment length field itself is included in the length, so the segment length must be at least 2.");
        return false;
    }
    return true;
}

static bool verifyApp0Segment(uint64_t offset, uint64_t remainingLength, FILE* image, JPEGSegment* segment)
{
    if(getSegmentLength(segment) < 2 + 5)
    {
        fprintf(stderr, METHODNAME(verifyApp0Segment)"The APP0 segments length has to be at least 7 to hold the length field + the JFIF\\0 marker.\n");
        return 0;
    }
    
    unsigned char buf[5];
    
    fseek(image, offset + 4, SEEK_SET); // Skip magic, type and length
    if(!fread_check_error(METHODNAME(verifyApp0Segment), buf, 5, image))
    {
        return false;
    }
    
    if(memcmp(buf, "JFIF\0", 5) != 0)
    {
        fprintf(stderr, METHODNAME(verifyApp0Segment)"The APP0 Segment does not start with \"JFIF\\0\".\n");
        return false;
    }
    return true;
}

static bool verifyApp1Segment(uint64_t offset, uint64_t remainingLength, FILE* image, JPEGSegment* segment)
{
    if(getSegmentLength(segment) < 2 + 6)
    {
        fprintf(stderr, METHODNAME(verifyApp1Segment)"The APP1 segments length has to be at least 8 to hold the length field + the \"Exif\\0\\0\" marker.\n");
        return 0;
    }
    
    unsigned char buf[6];
    
    fseek(image, offset + 4, SEEK_SET); // Skip magic, type and length
    if(!fread_check_error(METHODNAME(verifyApp1Segment), buf, 6, image))
    {
        return false;
    }
    
    if(memcmp(buf, "Exif\0\0", 6) != 0)
    {
        fprintf(stderr, METHODNAME(verifyApp1Segment)"The APP1 Segment does not start with \"Exif\\0\\0\".\n");
        return false;
    }
    return true;
}

static bool verifyApp8Segment(uint64_t offset, uint64_t remainingLength, FILE* image, JPEGSegment* segment)
{
    if(segment->type != 0xE8)
    {
        return false;
    }
    if(getSegmentLength(segment) < 2 + 6)
    {
        fprintf(stderr, METHODNAME(verifyApp8Segment)"The APP8 segments length has to be at least 8 to hold the length field + the \"SPIFF\\0\" marker.\n");
        return 0;
    }
    
    unsigned char buf[6];
    
    fseek(image, offset + 4, SEEK_SET); // Skip magic, type and length
    if(!fread_check_error(METHODNAME(verifyApp8Segment), buf, 6, image))
    {
        return false;
    }
    
    if(memcmp(buf, "SPIFF\0", 6) != 0)
    {
        fprintf(stderr, METHODNAME(verifyApp8Segment)"The APP8 Segment does not start with \"SPIFF\\0\".\n");
        return false;
    }
    return true;
}

static bool verifySecondSegment(uint64_t offset, uint64_t length, FILE* image, JPEGSegment* segment)
{
    switch(segment->type)
    {
        case 0xE0:
            return verifyApp0Segment(offset, length, image, segment);
        case 0xE1:
            return verifyApp1Segment(offset, length, image, segment);
        case 0xE8:
            return verifyApp8Segment(offset, length, image, segment);
        case 0xF7:
            return true;
        default:
            fprintf(stderr, METHODNAME(verifySecondSegment)"In all known formats the first segment has types 0xE0, 0xE1, 0xE8 or oxF7, not 0x%02x\n", segment->type);
            return false;
    }
}


uint64_t VerifyJPEG::getValidFileLength(uint64_t startOffset, uint64_t length, FILE* image)
{
    if(length < 4)
    {
        fprintf(stderr, METHODNAME(getValidFileLength)"No valid jpeg file can have less than 4 bytes of size!\n");
        return 0;
    }
    if(image == NULL)
    {
        fprintf(stderr, METHODNAME(getValidFileLength)"The image file FILE* was NULL.");
        return 0;
    }
    
    LOG_DEBUG("Verifying JPEG of max size %lx at %lx\n", length, startOffset);
    
    fseek(image, startOffset, SEEK_SET);
    
    JPEGSegment segment;

    uint64_t currentFileIndex = startOffset;
    uint64_t checkedSegments = 0;
    while(currentFileIndex < startOffset + length)
    {  
        LOG_DEBUG(METHODNAME(getValidFileLength)"Checking Segment %ld: \n", checkedSegments);
        
        if(!readAndValidateSegment(currentFileIndex, length - currentFileIndex, image, &segment))
        {
            // We detected an invalid segment, abort
            return 0;
        }
        uint64_t currentSegmentLength = getSegmentLength(&segment);
        
        LOG_DEBUG(METHODNAME(getValidFileLength)"Found segment of type 0x%02x(%s) of length %ld at offset 0x%lx\n", segment.type, getSegmentDescriptor(&segment), currentSegmentLength, currentFileIndex);
        
        if((checkedSegments == 0) && (segment.type != 0xD8))
        {
            fprintf(stderr, METHODNAME(getValidFileLength)"The first segment was not a SOI segment, this should not happen.\n");
            return 0;
        }
        if(checkedSegments != 0 && segment.type == 0xD8)
        {
            fprintf(stderr, METHODNAME(getValidFileLength)"Found SOI segment as non-first segment.\n");
            return 0;
        }
        if(checkedSegments == 1 && !verifySecondSegment(currentFileIndex, length - currentFileIndex, image, &segment))
        {
            return 0;
        }
        
        
        
        if(segment.type == 0xDA)        // START_OF_SCAN
        {
            uint64_t startIndex = currentFileIndex + currentSegmentLength;
            uint64_t scanLength = length - startIndex;
            // After the start of scan byte follows data of unknown size, scan for end.
            
            LOG_DEBUG(METHODNAME(getValidFileLength)"Found SOS segment, scanning for next header from %lx for next header (maxLength: %lx)\n", startIndex, scanLength);
            
            uint64_t offset = scanForNextSegmentHeader(startIndex, scanLength, image);
            
            LOG_DEBUG(METHODNAME(getValidFileLength)"Scan gave offset %lx\n", offset);
            
            if(offset == INVALID_OFFSET)
            {
                return 0;
            }
            currentFileIndex += offset;
        }
        
        currentFileIndex += currentSegmentLength;
        
        
        if(segment.type == 0xD9)       // END_OF_IMAGE
        {
            // All segments so far were valid and we found END_OF_IMAGE =>  Success
            return currentFileIndex - startOffset;
        }
        
        checkedSegments += 1;
    }
    // If this is reached our segments exceeded the possible length without finding END_OF_IMAGE
    return 0;
}
















