
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




#include "GarbageMan.h"
#include <stack>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fs.h>

#ifdef DEBUG
	#define __STDC_FORMAT_MACROS
	#include <inttypes.h>
#endif

#include "util.h"


#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define ALPHABET_SIZE 256	// upper case letters and %

#define FRAGMENTSIZE (500*1024*1024)
#define OVERLAPPINGBOUNDER 8 //=length of longest search pattern

using namespace std;

GarbageMan::GarbageMan()
{
}

GarbageMan::~GarbageMan()
{
}

uint64_t GarbageMan::initialHash(const char* const text, uint64_t length) {
	uint64_t hash = 0;
	for (uint64_t i=0; i<length; i++) {
            hash += ((uint64_t) ((unsigned char) text[i])) << (8 * (length - i - 1));
//			#ifdef DEBUG
//				printf("[initialHash] char %c (%X)\t%u (%X)\n", (unsigned char) text[i], (unsigned char) text[i], hash, hash);
//			#endif
	}
	hash = hash % q;
//#ifdef DEBUG
//	printf("[initialHash] %s: %" PRIu64 " (0x%" PRIx64 ")\n", text, hash, hash);
//#endif
	return hash;
}

uint64_t GarbageMan::rollingHash(uint64_t previousHash, char kickOut, char next, uint64_t length) {
        // See https://www2.cs.fau.de/teaching/SS2015/HalloWelt/ZK_2015.pdf#24
        uint64_t dump = (((uint64_t) ((unsigned char) kickOut)) << (8 * (length - 1))) % q;
//#ifdef DEBUG
//			printf("[rollingHash] dump %" PRIu64 " (0x%" PRIx64 ")\n", dump, dump);
//#endif
        uint64_t hash = (previousHash - dump);
	if (dump > previousHash) {
		 // correct for overflow if dump was larger than previousHash
		 hash = hash + q;
	}
	hash = (hash << 8) % q;
	hash = hash + ((uint64_t) ((unsigned char) next));
//#ifdef DEBUG
//			printf("[rollingHash] new hash %" PRIu64 " (0x%" PRIx64")\n", hash, hash);
//#endif
        hash = hash % q;
	return hash;
}


struct VerificationJob* createJob(PatternType patternType, uint64_t startOffset, uint64_t length) {
	struct VerificationJob* job = (VerificationJob*) malloc(sizeof(struct VerificationJob));
	switch(patternType) {
		case PatternType::pdfHeader:
		case PatternType::pdfFooter:
			job->type = pdf;
			break;
		case PatternType::pngHeader:
			job->type = png;
			break;
		case PatternType::jpgHeader:
			job->type = jpg;
			break;
		case PatternType::sqliteHeader:
			job->type = sqlite;
			break;
	}
	job->startOffset = startOffset;
	job->length = length;
	return job;
}


/**
 * Get the maximum number of bytes which we can use for the matching. This number equals the number of bytes of the shortest pattern.
 */
uint64_t getMaxPatternLength(map<PatternType, const char*> patterns) {
	uint64_t maxPatternLength = 65535;
	for (map<PatternType, const char*>::iterator it = patterns.begin(); it != patterns.end(); it++) {
		maxPatternLength = MIN(maxPatternLength, strlen(it->second));
	}
	// As we're deailing with 64bit integers, we cannot use more than 8 bytes for comparison
	// It would be even more efficient if we adapted q here
	return MIN(8, maxPatternLength);
}


/**
 * text: haystack to look into
 * patterns: map of pattern type and pattern as constant char array. Patterns must not contain \0-bytes except for the terminating byte
 * jbuf: where to put the validation jobs as a result
 * fragmentLength: length of the current text to be processed
 * fragmentOffset: offset of the first byte in the current fragment to the whole image
 * globalLength: size of the complete image
 **/
uint64_t GarbageMan::rabinKarp(const char* const text, map<PatternType, const char*> patterns, BNDBUF* jbuf, uint64_t fragmentLength, uint64_t fragmentOffset, uint64_t globalLength, uint64_t lastFoundAddressPar) {
	// Find maximum length of patterns
	uint64_t patternLength = getMaxPatternLength(patterns);

	// Precalculate hashes of patterns and store them in a hashmap
	multimap<uint64_t, PatternType> patternBins;
	for (map<PatternType, const char*>::iterator it = patterns.begin(); it != patterns.end(); it++) {
		uint64_t bin = GarbageMan::initialHash(it->second, patternLength);
		patternBins.insert(make_pair(bin, it->first));
	}

	uint64_t lastFoundAddress = lastFoundAddressPar - fragmentOffset; // avoid duplicate jobs when seeking in same address space

	bool init = false;
	uint64_t hash = 0;
	pair<multimap<uint64_t, PatternType>::iterator, multimap<uint64_t, PatternType>::iterator> hit;
	multimap<uint64_t, PatternType>::iterator hitIter;

	// Iterate over each byte (interpreted as character) in the text
	for (uint64_t pos = 0; pos < fragmentLength - patternLength; pos++) {
		// Compute the hash depending on the value calculated in the previous iteration
		if (!init) {
			hash = GarbageMan::initialHash(text + pos, patternLength);
			init = true;
		} else {
			hash = rollingHash(hash, text[pos-1], text[pos + patternLength - 1], patternLength);
//			#ifdef DEBUG
//				uint64_t validateHash = GarbageMan::initialHash(text + pos, patternLength);
//				printf("%zx\tInitial hash method: %" PRIu64 " (0x%" PRIx64 "), Rolling hash method: %" PRIu64" (0x%" PRIx64 ")\n", pos, validateHash, validateHash, hash, hash);
//				if (validateHash != hash) exit(1);
//			#endif
		}

//		#ifdef SPAM
//			printf("[GarbageMan] pos: 0x%" PRIu64 "\t%c%c%c\n", pos, text[pos], text[pos+1], text[pos+2]);
//		#endif

		// Check whether the hash matches the hash of one of the patterns
		hit = patternBins.equal_range(hash);
		for (hitIter = hit.first; hitIter != hit.second; hitIter++) {
			// If it matches, compare char by char as collisions may occur
			const char* pattern = patterns[hitIter->second];
//			#ifdef DEBUG
//				printf("[charCompare] %X %X %X\n", (unsigned char) pattern[0], (unsigned char) pattern[1], (unsigned char) pattern[2]);
//			#endif

			#ifdef DEBUG
				cout << "[GarbageMan] MAYBE Found " << hitIter->second << " at address ";
				printf("0x%lx \n", (uint64_t) pos + fragmentOffset);
			#endif

			// Additionally, patterns can be longer than max pattern length. Thus, we need need to compare the pattern's remaining chars
			bool match = true;
			uint64_t actualLength = strlen(pattern);	// watch out for \0 byte!!
			for (uint64_t i=0; i<actualLength && match; i++) {
//				#ifdef DEBUG
//					printf("[charCompare] %X ?= %X\n", (unsigned char) pattern[i], (unsigned char) text[i + pos]);
//				#endif
				if (i + pos >= fragmentLength) {
					match = false;
				}
				else if (pattern[i] != text[i + pos]) {
					match = false;
				}
			}

			// lastFoundAddress
			if (match && (lastFoundAddress == 0 || lastFoundAddress != pos) )
			{
				// Create job(s) and put them into jbuf
				#ifdef DEBUG
					cout << "[GarbageMan] Found " << hitIter->second << " at address ";
					printf("0x%lx \n", (uint64_t) pos + fragmentOffset); 
				#endif
				if (PatternType::pdfHeader == hitIter->second) {
					pdfStart.push_back(pos + fragmentOffset);
				}
				else {
					struct VerificationJob* job = nullptr;
					if (PatternType::pdfFooter == hitIter->second) {
						for (vector<uint64_t>::iterator pdfIter = pdfStart.begin(); pdfIter != pdfStart.end(); pdfIter++) 
						{
							uint64_t pdfLength = (fragmentOffset + pos + actualLength) - *pdfIter;
							job = createJob(hitIter->second, *pdfIter, pdfLength);
							bbPut(jbuf, job);
						}
					}
					else {
						// png or jpg: length unknown
						job = createJob(hitIter->second, fragmentOffset + pos, globalLength - pos);
						bbPut(jbuf, job);
					}

					// Update last found address pointer
					lastFoundAddress = pos;
				}


			}
		}
	}

	// return absolute address of the pattern which was found last
	return (lastFoundAddress + fragmentOffset);
}



void GarbageMan::work(const char* pathToImg, BNDBUF* jbuf) {
	map<PatternType, const char*> patterns;
	patterns.insert(make_pair(pdfHeader, "%PDF"));	// PDF header
	patterns.insert(make_pair(pdfFooter, "%EOF"));	// PDF footer
	patterns.insert(make_pair(jpgHeader, "\xff\xd8\xff"));	// JPG header ff d8 ff
	patterns.insert(make_pair(pngHeader, "\x89PNG\x0d\x0a\x1a\x0a"));	// PNG header 89 50 4e 47 0d 0a 1a 0
	patterns.insert(make_pair(sqliteHeader, "\x53\x51\x4c\x69\x74\x65\x20\x66\x6f\x72\x6d\x61\x74\x20\x33\x00"));	// SQLite format 3

	// Resolve path
	char* realPath = realpath(pathToImg, NULL);
	if (NULL == realPath) {
		perror("realpath");
		return;
	}

	int fd = open(realPath, O_RDONLY, 0);
	if(fd == -1)
	{
		cerr << "GarbageMan open failed: " << strerror(errno) << endl;
		return;
	}

	uint64_t imageSize = getFileOrDeviceSize(realPath, fd);
	if (0 == imageSize) {
		cerr << "GarbageMan getFilesize returned 0" << endl;
		close(fd);
		return;
	}

	// We don't need realPath any more
	free(realPath);

	unsigned int numberOfFragments = ((unsigned int) imageSize/FRAGMENTSIZE);
	uint64_t defaultFragmentLength = FRAGMENTSIZE;
	uint64_t fragmentlength = 0;
	uint64_t currentOffset = 0;
//	uint64_t OverlappingBytes = 0;
#ifdef DEBUG
	cout << dec << "image Size: " << imageSize << " numberOfFragments : " << numberOfFragments << endl;
#endif

	char cache[2*OVERLAPPINGBOUNDER];
	uint64_t lastFoundAddress = 0;

	for(unsigned int j=0; j < 2*OVERLAPPINGBOUNDER; j++)
	{
		cache[j]=0;
	}

	for(unsigned int i =0; i < (numberOfFragments + 1); i++)
	{
		if(i < numberOfFragments)
		{
			fragmentlength = defaultFragmentLength;
		} else {
			fragmentlength = imageSize - currentOffset;
#ifdef DEBUG
			cout << dec << "last iteration: fragmentlength : " << fragmentlength << endl; 
#endif
			if(fragmentlength==0)
			{
				break;
			}
		}
#ifdef DEBUG
		cout << dec << "iteration started : " << i << " current fragmentlength : " << fragmentlength << " currentOffset : " << currentOffset << endl;
#endif
		char* mmappedData = (char*) mmap(NULL, fragmentlength, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, currentOffset);

		for(unsigned int j=0; j<OVERLAPPINGBOUNDER; j++)
		{
			cache[j]=cache[j+OVERLAPPINGBOUNDER];
			cache[j+OVERLAPPINGBOUNDER]=mmappedData[fragmentlength - 1 - OVERLAPPINGBOUNDER + j]; 
		}

		if(i!=0)
		{
#ifdef DEBUG
			cout << "call rabinKarp for cache" << endl;
#endif 
			lastFoundAddress = rabinKarp(cache, patterns, jbuf, 2*OVERLAPPINGBOUNDER, currentOffset-OVERLAPPINGBOUNDER, imageSize, lastFoundAddress);
		}

		if(mmappedData == MAP_FAILED )
		{
			if(errno == ENOMEM)
			{
				defaultFragmentLength /= 2;
				i=i-1;
				cerr << "recieved ENOMEM, trying the half" << endl; 
				continue;
			}
			cerr << "GarbageMan mmap failed " << strerror(errno) << endl; 
			return;
		}
		#ifdef DEBUG
			cout << "mmapded Data " << hex << &mmappedData << endl; 
		#endif

		lastFoundAddress = rabinKarp(mmappedData, patterns, jbuf, fragmentlength, currentOffset, imageSize, lastFoundAddress);

		currentOffset = currentOffset + fragmentlength; 
		#ifdef DEBUG
			cout << dec << "rabinKarp returned in iteration " << i << " currentOffset : " << currentOffset  << " fragmentlength : " << fragmentlength << endl;
		#endif

		int ret = munmap(mmappedData, fragmentlength);
		if (-1 == ret)
		{
			cerr << "munmap : " << strerror(errno) << std::endl;
			return ;
		}
	}

	close(fd);
}
