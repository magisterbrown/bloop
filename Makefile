all: cmp
cmp: main.cpp
	g++ -g3 lexer.cpp main.cpp -o cmp
