Bloop
=====

Interpreter for the `bloop` primitive recursive programming language. This language is described in Chapter 13 of `Gödel, Escher, Bach: an Eternal Golden Braid` book by Douaglas Hofstadter. 


To interper a program find a `*.bloop` file. Couple examples are in `programs` directory. Then find last defined procedure and count number of arguments it has. This procedure is an enry point. Call `./bloop <program-name.bloop> <args>*`. Arguments should be positive integers.

## Build
Interpreter is written in c++17. Before building install a `g++` and `make`.
Then run `make`. It should produce `bloop` executable.


