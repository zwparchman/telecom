OFLAGS =  -Ofast
CC=g++
STD=-std=c++14
CFLAGS= -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				-fopenmp -march=native \
				$(OFLAGS)
LFLAGS= -g $(OFLAGS) -fopenmp -pthread $(STD) 

PROG=./program

.PHONY:clean 

Objects= main.o Timer.o

all : $(Objects) program gen eatram 

eatram : eatram.cpp
	g++ eatram.cpp -o eatram -O3 

gen: ./generate.cpp
	g++ generate.cpp -o gen -O3 -fopenmp --std=c++14

program : $(Objects)
	$(CC) $(Std) $(LFLAGS) $(Objects) -o program -lboost_iostreams

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
	rm -f gen 
