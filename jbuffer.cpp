
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





#include "jbuffer.h"
#include <stdio.h>
#include <stdbool.h>
#include "sem.h"
#include "VerificationJob.h" 

struct BNDBUF {
  volatile int indNextIn;
  volatile int indNextOut;
  struct VerificationJob **values;
  int size;
  SEM *anz_frei;
  SEM *anz_bel;
};

BNDBUF *bbCreate (size_t size) {
  BNDBUF *buf = (BNDBUF *)malloc(sizeof(BNDBUF));
  if (buf == NULL) {
    return NULL;
  }
  buf->values = (struct VerificationJob**) malloc(sizeof(struct VerificationJob*)*size);
  if (buf->values == NULL) {
    free(buf);
    return NULL;
  }
  buf->indNextIn = 0;
  buf->indNextOut = 0;
  int size_int = (int)size;
  buf->size = size_int;
  buf->anz_bel = semCreate(0);
  if(buf->anz_bel == NULL) {
    free(buf->values);
    free(buf);
    return NULL;
  }
  buf->anz_frei = semCreate(size_int);
  if(buf->anz_frei == NULL) {
    semDestroy(buf->anz_bel);
    free(buf->values);
    free(buf);
    return NULL;
  }
  return buf;
}

void bbDestroy (BNDBUF *buf) {
  if(buf == NULL) {
    return;
  }
  free(buf->values);
  semDestroy(buf->anz_frei);
  semDestroy(buf->anz_bel);
  free(buf);
}

void bbPut (BNDBUF *buf, struct VerificationJob *value) {
  P(buf->anz_frei);
  buf->values[buf->indNextIn] = value;
  buf->indNextIn = ((buf->indNextIn)+1)%(buf->size);
  V(buf->anz_bel);
}

struct VerificationJob *bbGet(BNDBUF *buf) {
  int indNextOutCopy;
  struct VerificationJob *value;
  P(buf->anz_bel);
  do {
    indNextOutCopy = buf->indNextOut;
    value = buf->values[buf->indNextOut];
  } while(!__sync_bool_compare_and_swap(&buf->indNextOut,indNextOutCopy,((buf->indNextOut)+1)%(buf->size)));
  V(buf->anz_frei);
  return value;
}
