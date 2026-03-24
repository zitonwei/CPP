CXX = g++
TARGET = hello
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

CXXFLAGS = -c -Wall


%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(TARGET)