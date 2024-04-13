CC = clang 
CXX = clang++
all: dir
	$(CXX) -static -O2 main.cpp -Iliburing/include -Lliburing/lib -luring -o dist/server -g
debug: dir
	$(CXX) -static main.cpp -Iliburing/include -Lliburing/lib -luring -o dist/server_debug -g
dir:
	mkdir -p ./dist
test: all
	./dist/server

