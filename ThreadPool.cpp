
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



#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/syscall.h> 
#include <iostream>

#include "ThreadPool.h"
#include "GarbageMan.h"
#include "SewagePlant.h"
#include "modules/VerifyJPEG.h"
#include "modules/VerifyPDF.h"
#include "modules/VerifyPNG.h"  

#define THREADNICE -3

using namespace std;

ThreadPool::ThreadPool(int numberOfThreads, char *pathToImage) 
{
  this->numberOfThreads = numberOfThreads;
  this->pathToImage = pathToImage; 
}

ThreadPool::~ThreadPool() 
{
  threads.clear();
}

void try_nicing(pid_t tid) 
{
  int nicelevel = THREADNICE; 
  setpriority(PRIO_PROCESS, tid, nicelevel);
#ifdef DEBUG
	nicelevel = getpriority(PRIO_PROCESS, tid);
	cout << "Thread with ID " << tid << " started with nice " << nicelevel << endl; 
#endif

}

void runModules (BNDBUF *jbuf,uint id,char *pathToImage, SewagePlant *swp) 
{
  
#ifdef DEBUG
    cout << "run Modules started with id " << id << endl;
#endif

    pid_t tid = (pid_t) syscall(SYS_gettid);
    try_nicing(tid);
    VerificationJob *mj;

    FILE* myImageFile = fopen(pathToImage, "r");
    if(myImageFile == NULL)
    {
        perror("fopen");
        return; 
    }
	
    while (true) 
    {
    mj = bbGet(jbuf);
	VerifyJPEG jpegV;
	VerifyPDF pdfV;
	VerifyPNG pngV;
    uint64_t foundLength = 0; // error: ‘foundLength’ may be used uninitialized in this function [-Werror=maybe-uninitialized]
	switch(mj->type)
	{
		case pdf:
				foundLength = pdfV.getValidFileLength(mj->startOffset, mj->length, myImageFile);
	        break;
		case jpg:
            foundLength = jpegV.getValidFileLength(mj->startOffset, mj->length, myImageFile);
#ifdef DEBUG 
			cout << "Possible jpeg file at " << showbase << hex << mj->startOffset << " of length " << foundLength << endl;
#endif
			break; 
		case png: 
				foundLength = pngV.getValidFileLength(mj->startOffset, mj->length, myImageFile);
			break;
		case poison:
	        fclose(myImageFile);
	        free(mj);
#ifdef DEBUG
            cout << "PoisonPill recieved. id: " << id << endl;
#endif
            return;
		  
	}
	if(foundLength!= 0)
	{
		//output methode 
		swp->purify(mj->startOffset, foundLength, mj->type, myImageFile);
	} /*else{
		//logging 
	}*/
  }
}

void runParser (BNDBUF *jbuf, uint id, int numberOfThreads, char *pathToImage) {
	#ifdef DEBUG
		cout << "run parser started " << endl;
	#endif
 
	pid_t tid = (pid_t) syscall(SYS_gettid);
	try_nicing(tid);
	GarbageMan bene = GarbageMan();
	bene.work(pathToImage, jbuf); 
	
	for (int i = 1; i < numberOfThreads; i++) {
		struct VerificationJob *PoisonPill = (struct VerificationJob *)malloc(sizeof(struct VerificationJob));
		assert(PoisonPill != NULL);
		PoisonPill->type = poison;
		bbPut(jbuf,PoisonPill);
	}
	
	#ifdef DEBUG
		cout << "run Parser finished" << endl;
	#endif
}

void ThreadPool::start (BNDBUF *jbuf, SewagePlant *swp) {
/*#ifdef DEBUG
	cout << "numberOfThreads: " << numberOfThreads << endl;
#endif*/

  threads.push_back(thread(runParser,jbuf,0,numberOfThreads,pathToImage));
  for (int i = 1; i < numberOfThreads; i++) {
    threads.push_back(thread(runModules,jbuf,i,pathToImage,swp));
  }
 #ifdef DEBUG
			cout << "all threads started " << endl;
		#endif

}

void ThreadPool::waitForJoin () {
  for (int i = 0; i < numberOfThreads; i++) {
    threads[i].join();
  }
}

