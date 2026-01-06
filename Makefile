all: cmp
cmp: main.cpp
	g++ -O0 -g3 lexer.cpp main.cpp -o cmp
