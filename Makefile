####  Makefile 	####
# Usage:           #
#   >make          #
#   >make clean	   #
####################

FILES = truthtable.cc blif-verifier.cc blif.cc tokenizer.cc
OBJECTS = truthtable.o blif-verifier.o blif.o tokenizer.o

CPPFLAGS = -Wall -pedantic --std=c++11

default: release

debug: CPPFLAGS += -g -ggdb
debug: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

release: CPPFLAGS += -O3
release: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

clean:
	\rm -f ${OBJECTS} blif-verifier

%.o : %.cc
	g++ $(CPPFLAGS) -c $< -o $@
