CC=gcc
CFLAGS=-I /opt/OpenBLAS/include/ 
LDFLAGS= -L/opt/OpenBLAS/lib -lopenblas -lpthread -lm
DEPS=matrix.h nnet.h split.h

all: network_test 
all: CFLAGS += -O3
all: LDFLAGS += -O3

debug: network_test
debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -DDEBUG -g

network_test: matrix.o nnet.o network_test.o split.o
	$(CC) -o $@ $^ $(LDFLAGS)

c.o: 
	$(CC) $(CFLAGS) $<  -o $@

clean:
	rm -f *.o network_test

