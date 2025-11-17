CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: libpngenc.a

libpngenc.a: PNGenc.o adler32.o crc32.o deflate.o trees.o zutil.o
	ar -rc libpngenc.a PNGenc.o adler32.o crc32.o deflate.o trees.o zutil.o ;\
	sudo cp libpngenc.a /usr/local/lib ;\
	sudo cp ../src/PNGenc.h /usr/local/include ;\
	sudo cp ../src/zutil.h /usr/local/include ;\
	sudo cp ../src/trees.h /usr/local/include ;\
	sudo cp ../src/deflate.h /usr/local/include

PNGenc.o: ../src/PNGenc.cpp ../src/png.inl ../src/PNGenc.h
	$(CXX) $(CFLAGS) -c ../src/PNGenc.cpp

adler32.o: ../src/adler32.c
	$(CC) $(CFLAGS) -c ../src/adler32.c

crc32.o: ../src/crc32.c
	$(CC) $(CFLAGS) -c ../src/crc32.c

deflate.o: ../src/deflate.c
	$(CC) $(CFLAGS) -c ../src/deflate.c

trees.o: ../src/trees.c
	$(CC) $(CFLAGS) -c ../src/trees.c

zutil.o: ../src/zutil.c
	$(CC) $(CFLAGS) -c ../src/zutil.c

clean:
	rm -rf *.o libpngenc.a
