FLAGS+=-std=c++17 -g -Wall -Wextra -Wpedantic -D_GNU_SOURCE -Werror=all -lpthread
COMPILER=g++

main: smalldb.cpp sdbsh.cpp student.o db.o queries.o utils.o
	${COMPILER} -o smalldb smalldb.cpp student.o db.o queries.o utils.o ${FLAGS}

run:
	make main && ./smalldb


student.o: student.cpp student.hpp
	${COMPILER} -c student.cpp ${FLAGS}

queries.o: queries.cpp queries.hpp
	${COMPILER} -c queries.cpp ${FLAGS}

db.o: db.cpp db.hpp
	${COMPILER} -c db.cpp ${FLAGS}

tests: tests/run_tests.sh
	./tests/run_tests.sh

utils.o: utils.cpp utils.hpp
	${COMPILER} -c utils.cpp ${FLAGS}

clean:
	rm logs/*