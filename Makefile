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
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier and16.blif and32.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing and16.blif != and32.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier mul5.blif mul5_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing mul5.blif != mul5_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier mul5.blif mul5.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing mul5.blif == mul5.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier mul12.blif mul12_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing mul12.blif != mul12_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier exp.blif exp.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing exp.blif == exp.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier exp.blif exp_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing exp.blif != exp_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	gcov -l -r -f ${FILES} ${HEADERS} > coverage.report
	lcov --capture --directory . --output-file coverage.info && genhtml coverage.info || echo "lcov not installed."

buildinternal: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

clean:
	\rm -f ${OBJECTS} blif-verifier

%.o : %.cc
	g++ $(CPPFLAGS) -c $< -o $@
