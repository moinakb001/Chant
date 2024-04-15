all:
	em++ main.cpp -O2 --closure 1 --js-library lib_webgpu.js --closure-args=--externs=webgpu-closure-externs.js -o dist/app.js -L libs/lib -lmupdf -lmupdf-third -I libs/include -sWASM_WORKERS
	cp index.html dist/
	cp dist/* ../