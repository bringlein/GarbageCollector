
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


#include <cassert>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <cstdlib>
#include "jbuffer.h" 
#include "ThreadPool.h"
#include "SewagePlant.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <thread>
#include <vector> 
#include <sstream>
#include <time.h>
#include <algorithm>
#include <limits.h>


#include "util.h"


typedef unsigned int uint;
using namespace std; 

#define THREADFACTOR 2
#define BUFFERFACTOR 3

#define OUTPUTFOLDER_NAME "output"


void printUsageAndExit(char *argv_0 )
{
	cerr << "Usage: " << argv_0 << " image_file output_destination [-N] [-T] " << endl; 
		exit(1); 
}


int main(int argc, char** argv) 
{
#ifdef DEBUG
	cout << "GarbageCollector started! \n with " << argc << " arguments: "; 
	for(int i = 0; i<argc; i++)
	{
		cout << argv[i] << " "; 
	}
	cout << endl; 
#endif 

	if(argc < 3 || argc > 5)
	{
		printUsageAndExit(argv[0]);
	}
	
	//init SewagePlant
	stringstream outputDestination;
	outputDestination << argv[2];
	if(argv[2][ strlen(argv[2]) -1] != '/')
	{
		outputDestination << "/" ;
	} 
	outputDestination << OUTPUTFOLDER_NAME;

	string timestamp_option = "-T";
	string naming_option = "-N";
	if((argc == 4 || argc == 5) && (naming_option.compare(argv[3]) == 0 ) )
	{
		string imageFileName(argv[1]);
		replace(imageFileName.begin(), imageFileName.end(), '/','-');
		replace(imageFileName.begin(), imageFileName.end(), '.','_');
		outputDestination << "_" << imageFileName;
	
	} 

	if( (argc == 4 && (timestamp_option.compare(argv[3])==0) ) ||  (argc == 5 && (timestamp_option.compare(argv[4])==0) ) )
	{
		time_t rawtime;
		struct tm * timeinfo;
		char timeBuffer [30];

		 time (&rawtime);
		timeinfo = localtime (&rawtime);

		strftime (timeBuffer,30,"_%F_%H-%M-%S",timeinfo);
		
		/*if(argc == 5 )
		{
			outputDestination << "_";
		}*/

		outputDestination << timeBuffer;
	} else if(argc == 4 || argc == 5)
	{
		printUsageAndExit(argv[0]);
	}

	
	int ret = mkdir(outputDestination.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(ret != 0)
	{
		cerr << "failed to create directories at output_destination: " << strerror(errno) << endl;
		exit(1);
	}

	outputDestination << "/";
	vector<string> targetDirectories;
	targetDirectories.push_back("pdf");
	targetDirectories.push_back("jpg");
	targetDirectories.push_back("png");
	targetDirectories.push_back("sqlite");


	for(uint i = 0; i< targetDirectories.size(); i++)
	{
		stringstream nextFolder;
		nextFolder << outputDestination.str() << targetDirectories[i];
		ret += mkdir(nextFolder.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}


	if(ret != 0)
	{
		cerr << "failed to create subdirectories at output_destination: " << strerror(errno) << endl;
		exit(1);
	}


		// Resolve path
	char* realPath = realpath(argv[1], NULL);
	if (NULL == realPath) {
		perror("realpath");
		exit(1);
	}

	int fd = open(realPath, O_RDONLY, 0);
	if(fd == -1)
	{
		cerr << "GarbageMan open failed: " << strerror(errno) << endl;
		exit(1);

	}

	uint64_t imageSize = getFileOrDeviceSize(realPath, fd);
	if (0 == imageSize) {
		cerr << "GarbageMan getFilesize returned 0" << endl;
		close(fd);
		exit(1);
	}

	// We don't need realPath any more
	free(realPath);
	close(fd);


	SewagePlant swp(outputDestination.str(), 0,imageSize);
	cout << "Recovered Files are saved in : " << outputDestination.str() << endl;

	//starting ThreadPool
	int numberOfThreads = thread::hardware_concurrency();
	cout << numberOfThreads << " concurrent threads are supported.\n\n";
    
	if(numberOfThreads == 0 ) // If the value is not well defined or not computable, returns 0. 
	{
		numberOfThreads = 4;
	}
	numberOfThreads = numberOfThreads * THREADFACTOR; 
	
	ThreadPool tp(numberOfThreads,argv[1]);
	int numberOfPlaces = numberOfThreads * BUFFERFACTOR; // Anzahl der Plaetze in Jbuffer
	BNDBUF *jbuf = bbCreate(numberOfPlaces);
	assert(jbuf != NULL);
#ifdef DEBUG
	cout << numberOfThreads << " should be started. " << endl;
#endif

	tp.start(jbuf, &swp);

		//wait thrads for join 
	tp.waitForJoin();
	
	cout << "all threads joined " << endl; 

	swp.condense();


	//Cleanup
	//TODO: Destructors??
	bbDestroy(jbuf);

	//Greetings ;) 
	cout << "\n To help is our mission! Bye" << endl; 
	return 0; 
}

