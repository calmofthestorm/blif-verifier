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

lint:
	cpplint.py *.h *.cc

test: clean coverage
	echo "*** Testing attempt to read non-existing file. ***"
	! ./blif-verifier test/does_not_exist.blif add8.blif a.c

	echo "*** Testing bad dependencies ***"
	! ./blif-verifier test/add8.blif test/add8_baddep.blif a.c

	echo "*** Testing bad section ***"
	! ./blif-verifier test/add8.blif test/add8_badsection.blif a.c

	echo "*** Testing dupe truth table ***"
	! ./blif-verifier test/add8_dupett.blif test/add8.blif a.c

	echo "*** Testing duplicate block ***"
	! ./blif-verifier test/add8_duplicateblock.blif test/add8.blif a.c

	echo "*** Testing names early 1 ***"
	! ./blif-verifier test/add8_namesearly1.blif test/add8.blif a.c

	echo "*** Testing names early 2 ***"
	! ./blif-verifier test/add8_namesearly2.blif test/add8.blif a.c

	echo "*** Testing undefined po ***"
	! ./blif-verifier test/add8_undefpo.blif test/add8.blif a.c

	./blif-verifier test/and16.blif test/and16.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/and16.blif == test/and16.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/and16.blif test/and32.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/and16.blif != and32.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/mul5.blif test/mul5_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/mul5.blif != test/mul5_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/mul5.blif test/mul5.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/mul5.blif == test/mul5.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/mul12.blif test/mul12_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/mul12.blif != test/mul12_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/exp.blif test/exp.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/exp.blif == test/exp.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/exp.blif test/exp_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/exp.blif != test/exp_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/log.blif test/log.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/log.blif == test/log.blif ***"
	./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	./blif-verifier test/log.blif test/log_bad.blif a.c 2> /dev/null > /dev/null || true
	gcc harness.c a.c -o verifier -lm
	echo "*** Testing test/log.blif != test/log_bad.blif ***"
	! ./verifier > /dev/null
	gcov -l -s c++/  -s /usr/include -r -f ${FILES} ${HEADERS} | tail -n 1
	rm -f verifier

	gcov -l -r -f ${FILES} ${HEADERS} > coverage.report
	lcov --capture --directory . --output-file coverage.info && genhtml coverage.info || echo "lcov not installed."

buildinternal: ${OBJECTS}
	g++ $(CPPFLAGS) ${OBJECTS} -o blif-verifier

clean:
	\rm -f ${OBJECTS} blif-verifier *.gcov *.gcda *.gcno

%.o : %.cc
	g++ $(CPPFLAGS) -c $< -o $@
