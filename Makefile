
#################################
#
#Copyright (c) 2015. Lukas Dresel, Julius Lohmann, Benedikt Lorch, Burkhard Ringlein, Andreas Rupp, Yuriy Sulima, Carola Touchy
#
#This file is part of GarbageCollector. 
#
#  GarbageCollector is free software: you can redistribute it and/or modify
#	 it under the terms of the GNU General Public License as published by
#	 the Free Software Foundation, either version 3 of the License, or
#	 (at your option) any later version.
#
#	 GarbageCollector is distributed in the hope that it will be useful,
#	 but WITHOUT ANY WARRANTY; without even the implied warranty of
#	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	 GNU General Public License for more details.
#
#	 You should have received a copy of the GNU General Public License
#	 along with GarbageCollector.  If not, see <http://www.gnu.org/licenses/>.
#
#################################




SOURCES	= main.c
PROG	=GarbageCollector
CC	=g++
DYNLIBS	= 

### misc. options #####
CFLAGS	= -Wall -DLINUX -lstdc++ -std=c++11 -Wconversion -Werror # -pedantic # -Wextra
CFLAGS	+= -fPIC -O3 -funroll-loops  # -g 
LIBFLAGS     = -pthread -lm
RM   	= rm -f
BUILD_TYPE = debug # oder production
DEBUGFLAGS = 

SOURCES  = $(shell ls *.cpp 2> /dev/null)
SOURCES += $(shell find modules/*.cpp 2> /dev/null)
OBJS     = $(SOURCES:%.cpp=%.o)


#################################
# explicit dependencies
all: 

debug: DEBUGFLAGS += -DDEBUG 
debug: CFLAGS += -g

spam: DEBUGFLAGS += -DDEBUG -DSPAM 
spam: CFLAGS += -g

main.o: jbuffer.o sem.o

test/VerifyJPEGTester: DEBUGFLAGS += -DDEBUG

########################

.PHONY: all clean debug

all: $(PROG)

clean:
	$(RM) $(PROG) $(OBJS) test/GarbageManTester test/VerifyJPEGTester test/*.o

debug: $(PROG)


spam: $(PROG)
	echo "You wanted extra verbose debug output. Don't blame me!" 

$(PROG): $(OBJS) $(DYNLIBS)
	$(CC)  $(CFLAGS) $(LIBFLAGS) -o $@ $^ 
	
%.o: %.cpp
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $< -o $@

GarbageManTester: GarbageManTester.o GarbageMan.o
	$(CC) $(CFLAGS) -g $(LIBFLAGS) -o $@ $^

test/VerifyJPEGTester: test/VerifyJPEGTester.o modules/VerifyJPEG.o
	$(CC) $(CFLAGS) -g $(LIBFLAGS) -o $@ $^
