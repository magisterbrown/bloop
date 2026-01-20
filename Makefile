CXX = g++
CXXFLAGS = -O0 -g3 -I./include -Wall
SRCS = src/lexer.cpp src/main.cpp src/expression.cpp src/parsing.cpp src/block.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = bloop

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp include/bloop.h include/lexer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -f $(TARGET) $(OBJS)
