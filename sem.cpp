/*
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



#include "sem.h"
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

struct SEM {
  volatile int val;
  pthread_mutex_t m;
  pthread_cond_t c;
};
  
SEM *semCreate(int initVal) {
  SEM *sem = (SEM*) malloc(sizeof(SEM));
  if(sem == NULL) {
    return NULL;
  }
  sem->val = initVal;
  errno = pthread_mutex_init(&(sem->m),NULL);
  if (errno != 0) {
    free(sem);
    return NULL;
  }
  errno = pthread_cond_init(&(sem->c),NULL);
  if(errno != 0) {
    pthread_mutex_destroy(&(sem->m));
    free(sem);
    return NULL;
  }
  return sem;
}

void semDestroy(SEM *sem) {
  if(sem == NULL) {
    return;
  }
  int temp = 0;
  errno =  pthread_mutex_destroy(&sem->m);
  if(errno != 0) {
    temp = errno;
  }
  errno = pthread_cond_destroy(&sem->c);
  if (temp != 0 && errno == 0) {
    errno = temp;
  }
  free(sem);
}

void P(SEM *sem) {
  pthread_mutex_lock(&sem->m);
  while(sem->val <= 0) {
    pthread_cond_wait(&sem->c,&sem->m);
  }
  sem->val = sem->val - 1;
  pthread_mutex_unlock(&sem->m);
}

void V(SEM *sem) {
  pthread_mutex_lock(&sem->m);
  sem->val = sem->val + 1;
  pthread_cond_broadcast(&sem->c);
  pthread_mutex_unlock(&sem->m);
}
