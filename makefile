CXX=c++
CXXFLAGS=-I. -std=c++14 -g -ggdb -lboost_system -pthread

BINS=tsmap

all: $(BINS)

tsmap: b.cpp b.hpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf *.o *.a $(BINS)

