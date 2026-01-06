all: cmp
cmp: main.cpp
	g++-14 -std=c++23 -g3 main.cpp -o cmp
