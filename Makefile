all: cmp
cmp: main.cpp lexer.cpp lexer.h
	g++ -O0 -g3 lexer.cpp main.cpp -o cmp
