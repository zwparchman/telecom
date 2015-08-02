OFLAGS =   -DNDEBUG -O3
BOOST_DEFINE=0
BOOST_LIBS=#-lboost_iostreams
CC=g++
STD=-std=c++14
CFLAGS= -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				-fopenmp -march=native -DWILL_USE_BOOST=$(BOOST_DEFINE) \
				$(OFLAGS)
LFLAGS= -g $(OFLAGS) -fopenmp -pthread $(STD) 

PROG=./real

.PHONY:clean 

Objects= Timer.o entry_pool.o MappedFile.o

all : $(Objects) gen eatram real

eatram : eatram.cpp
	g++ eatram.cpp -o eatram -O3 

gen: ./generate.cpp
	g++ generate.cpp -g -o gen -fopenmp --std=c++14

real : real.o entry_pool.o Timer.o MappedFile.o
	$(CC) $(Std) $(LFLAGS) real.o Timer.o entry_pool.o MappedFile.o -o real $(BOOST_LIBS)

real.o : real.cpp entry_pool.h 
	$(CC) $(CFLAGS) $<

$(Objects): %.o: %.cpp %.h
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
	rm -f c*grind\.out\.*
	rm -f dump
	rm -f gen 
