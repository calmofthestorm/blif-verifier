####  Makefile 	####
# Usage:           #
#   >make          #
#   >make clean	   #
####################

FILES = truthtable.cc blif-verifier.cc blif.cc
OBJECTS = truthtable.o blif-verifier.o blif.o

CPPFLAGS = -Wall -pedantic --std=c++11

default: release

debug: ${OBJECTS}
	g++ $(CPPFLAGS) -g -ggdb ${OBJECTS} -o blif-verifier

release: ${OBJECTS}
	g++ $(CPPFLAGS) -O3 ${OBJECTS} -o blif-verifier

clean:
	\rm -f ${OBJECTS} blif-verifier

%.o : %.cc
	g++ $(CPPFLAGS) -c $< -o $@
