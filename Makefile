all: cmp
cmp:
	g++ -O0 -g3 -I./include lexer.cpp main.cpp src/expression.cpp src/parsing.cpp -o cmp
