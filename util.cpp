
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



#include "util.h"
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

size_t getFilesize(const char* filename) 
{
    struct stat st;
    int ret= stat(filename, &st);
		
	if(ret == -1)
	{
		std::cerr << "getFilesize : " << strerror(errno) << std::endl;
		return -1;
	}
    return st.st_size;   
}

/**
 * fd is an open file descriptor
 * returns 0 on error
 */
uint64_t getFileOrDeviceSize(const char* realPath, int fd) {
	const char devSubstr[] = "/dev/";
	if (strncmp(devSubstr, realPath, strlen(devSubstr)) == 0) {
		// realPath points to a device
		unsigned long numBlocks;
		int ret = ioctl(fd, BLKGETSIZE64, &numBlocks);
		if (-1 == ret) {
			std::cerr << "ioctl " << strerror(errno) << std::endl;
			return 0;
		}
		return (uint64_t) numBlocks; // < 9
	}
	else {
		// realPath points to a file
		struct stat st;
		int ret = stat(realPath, &st);
		if(ret == -1) {
			std::cerr << "getFilesize : " << strerror(errno) << std::endl;
			return 0;
		}
		return (uint64_t) st.st_size; 
	}
}



bool fread_check_error(const char* descriptor, unsigned char* buf, size_t numBytes, FILE* file)
{
    size_t readBytes = fread(buf, sizeof(unsigned char), numBytes, file);    // We do it this way to accurately detect how many bytes we actually read.
    if(readBytes != numBytes)
    {
        if(feof(file))
        {
            fprintf(stderr, "%s: fread: Premature end of file reached\n", descriptor);
        }
        else if(ferror(file))
        {
            fprintf(stderr, "%s: fread: %s\n", descriptor, strerror(errno));
        }
        else
        {
            fprintf(stderr, "%s: This should not be reachable!.\n", descriptor);
        }
        return false;
    }
    return true;
}
