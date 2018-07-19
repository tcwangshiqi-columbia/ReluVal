CC=g++
DEPS=nnet.h ../matrix.h
CFLAGS=-I /opt/OpenBLAS/include/ -L/opt/OpenBLAS/lib -lopenblas -lpthread -lgfortran -O3

network_test: network_test.cpp nnet.cpp ../matrix.cpp $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)