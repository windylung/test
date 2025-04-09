CXX = g++
#CXXFLAGS = -Wall -g
CXXFLAGS = -std=c++11 -Wall -g

TARGET = lab1_skiplist
OBJS = src/skiplist_test.o src/zipf.o src/latest-generator.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

src/skiplist_test.o: src/skiplist_test.cc src/skiplist.h src/zipf.h src/latest-generator.h
	$(CXX) $(CXXFLAGS) -c src/skiplist_test.cc -o src/skiplist_test.o

src/zipf.o: src/zipf.cc src/zipf.h
	$(CXX) $(CXXFLAGS) -c src/zipf.cc -o src/zipf.o

src/latest-generator.o: src/latest-generator.cc src/latest-generator.h
	$(CXX) $(CXXFLAGS) -c src/latest-generator.cc -o src/latest-generator.o

clean:
	rm -f $(TARGET) $(OBJS)