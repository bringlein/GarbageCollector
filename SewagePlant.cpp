
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



#include "SewagePlant.h" 
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sstream>

using namespace std;

#define FRAGMENTSIZE (16*1024) 

SewagePlant::SewagePlant(const string pathToOutputFolder, unsigned int initValue)
{
	this->pathToOutputFolder = pathToOutputFolder;
	this->pdfCount = initValue;
	this->pngCount = initValue;
	this->jpgCount= initValue; 
}

SewagePlant::~SewagePlant()
{
}


void SewagePlant::purify(uint64_t startOffset, uint64_t realLength, JobType fileType,  FILE * imageFile)
{
	unsigned int fileNumber = 0;
	string subdir = " ";
	switch(fileType)
	{	
		case pdf:
				fileNumber = pdfCount++; 
				subdir = "pdf" ; break;
		case png: 
				fileNumber = pngCount++; 
				subdir = "png" ; break;
		case jpg: 
				fileNumber = jpgCount++; 
				subdir = "jpg" ;break;
		default:
				cerr << "printOutput default fail" << endl; 
	}

	int ret = fseek(imageFile, startOffset, SEEK_SET);
	
	if(ret == -1 )
	{
		cerr << "fseek : " << strerror(errno) << endl; 
		return;
	}

	stringstream  pathToNewFile;
	pathToNewFile << pathToOutputFolder.c_str() << subdir << "/" << fileNumber << "." << subdir;
#ifdef DEBUG 
	cout << "pathToNewFile : " << pathToNewFile.str().c_str() << endl;
#endif
	FILE* outputFile=fopen(pathToNewFile.str().c_str(), "w+");

	if(outputFile == NULL)
	{
		cerr << "fopen : " << strerror(errno) << endl; 
		return;
	}
	unsigned int numberOfFragments= ((unsigned int) realLength/FRAGMENTSIZE);
	size_t fragmentlength = 0;
	size_t currentOffset = 0;
	
	unsigned char buffer[FRAGMENTSIZE]; 

	for(unsigned int i =0; i < (numberOfFragments + 1); i++)
	{
#ifdef SPAM
		cout << "printOutput: iteration started : " << i << endl;
#endif 
		if(i<numberOfFragments)
		{
			fragmentlength = FRAGMENTSIZE;
		}else{
			fragmentlength = realLength - currentOffset;
			if(fragmentlength == 0 )
			{
				break;
			}
		}

		size_t readen = fread(buffer, 1, fragmentlength, imageFile);
		if(readen == 0)
		{
			if(feof(imageFile))
			{
				cerr << "feof imageFile" << endl;
			}else if(ferror(imageFile))
			{
				cerr << "ferror imageFile" << endl;
			}else{
				cerr << "unspecified error in SewagePlant while reading imageFile" << endl;
			}
		}
		
		size_t written = fwrite(buffer, 1, fragmentlength , outputFile);
		if(written == 0)
		{
			cerr << "unspecified error in SewagePlant while writing outputFile" << endl;
		}

		currentOffset += fragmentlength; 
	}

	fclose(outputFile);

}

void SewagePlant::condense()
{
	cout << endl << dec << "Valid Files Found: " << endl;
			       cout << "================== " << endl;
	cout << "\t PDF : " << pdfCount << endl;
	cout << "\t PNG : " << pngCount << endl;
	cout << "\t JPG : " << jpgCount << endl;
	cout << "------------------ " << endl; 

}

