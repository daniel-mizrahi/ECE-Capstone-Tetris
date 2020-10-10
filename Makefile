all:
	clang++ -std=c++11 tetris.cpp -g -w -lncurses
clean: 
	rm a.out
