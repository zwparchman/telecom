OFLAGS = 
CC=g++
STD=-std=c++14
CFLAGS= -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				-fopenmp -march=native \
				$(OFLAGS)
LFLAGS= -g $(OFLAGS) -fopenmp -pthread $(STD) 

PROG=./try2

.PHONY:clean 

Objects= main.o Timer.o

all : $(Objects) program gen raw eatram try2

raw : $(Objects) raw.cpp
	$(CC) raw.cpp $(LFLAGS) $(STD) -o raw -g -W -Wall -Wextra -Wshadow -fopenmp -march=native

try2: test2.cpp
	$(CC) $(STD) $(CFLAGS) test2.cpp Timer.cpp
	$(CC) $(STD) $(LFLAGS) test2.o -o try2 -lboost_iostreams Timer.o

eatram : eatram.cpp
	g++ eatram.cpp -o eatram -O3 

gen: ./generate.cpp
	g++ generate.cpp -o gen -O3 -fopenmp --std=c++14

program : $(Objects)
	$(CC) $(Std) $(LFLAGS) $(Objects) -o program

$(Objects): %.o: %.cpp
	$(CC) $(CFLAGS) $<

dbg: $(PROG)
	gdb $(PROG)

run: $(PROG)
	$(PROG)

time: $(PROG)
	time $(PROG)

cache: $(PROG)
	rm c*grind* -f
	valgrind --tool=cachegrind $(PROG)

call: $(PROG)
	rm c*grind* -f
	valgrind --tool=callgrind $(PROG)

inspect: 
	kcachegrind c*grind\.out\.*

clean:
	rm -f *o 
	rm -f program
	rm -f c*grind\.out\.*
	rm -f dump
	rm -f gen try2 raw
