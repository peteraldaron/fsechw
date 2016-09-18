CXX=c++
CXXFLAGS=-I. -std=c++14 -g -ggdb -lboost_system -pthread

BINS=tsmap

all: $(BINS)

tsmap: b.cpp TSMap.hpp KVPairList.hpp
	$(CXX) $(CXXFLAGS) -o $@ b.cpp

clean:
	rm -rf *.o *.a $(BINS)

