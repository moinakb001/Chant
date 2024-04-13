CC = clang 
CXX = clang++
all:
	$(CXX) -static -O2 main.cpp -Iliburing/include -Lliburing/lib -luring -o server

test: all
	./server

