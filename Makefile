
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: kvstore.o correctness.o skiplist.o SSTable.o index.o buffer.o

persistence: kvstore.o persistence.o skiplist.o SSTable.o index.o buffer.o

clean:
	-del -f correctness persistence *.o
