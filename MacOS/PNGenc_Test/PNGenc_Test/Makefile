CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: png_test

png_test: main.o
	$(CC) main.o $(LIBS) -o png_test 

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o png_test
