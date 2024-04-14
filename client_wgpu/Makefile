all:
	em++ main.cpp -O2 --closure 1 --js-library lib_webgpu.js --closure-args=--externs=webgpu-closure-externs.js -o dist/app.js
	cp index.html dist/
	cp dist/* ../