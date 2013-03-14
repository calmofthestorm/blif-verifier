####  Makefile 	####
# Usage:           #
#   >make          #
#   >make clean	   #
####################

FILES = truthtable.cc blif-verifier.cc blif.cc tokenizer.cc error.cc
HEADERS = truthtable.h blif.h tokenizer.h error.h
OBJECTS = truthtable.o blif-verifier.o blif.o tokenizer.o error.o

CPPFLAGS = -Wall -pedantic --std=c++11

default: release

debug: CPPFLAGS += -g -ggdb
debug: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

release: CPPFLAGS += -O3
release: buildinternal

coverage: CPPFLAGS += --coverage -O0
coverage: buildinternal

test: clean coverage
	./blif-verifier and16.blif and16.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing and16.blif == and16.blif ***"
	./verifier > /dev/null
	gcov ${FILES} ${HEADERS} > /dev/null

	./blif-verifier and16.blif and32.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing and16.blif != and32.blif ***"
	! ./verifier > /dev/null
	gcov ${FILES} ${HEADERS} | tail -n 1

	./blif-verifier mul5.blif mul5_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing mul5.blif != mul5_bad.blif ***"
	! ./verifier > /dev/null
	gcov ${FILES} ${HEADERS} | tail -n 1

buildinternal: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

clean:
	\rm -f ${OBJECTS} blif-verifier

%.o : %.cc
	g++ $(CPPFLAGS) -c $< -o $@
