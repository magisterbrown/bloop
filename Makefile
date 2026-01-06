all: cmp
cmp: main.cpp
	g++ -g3 main.cpp -o cmp
