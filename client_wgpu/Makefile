DIST_FOLDER := .deps
CLOSURE_FLAGS := --closure 1 --closure-args=--externs=webgpu-closure-externs.js
LIB_FLAGS := --js-library lib_webgpu.js -L libs/lib -lmupdf -lmupdf-third -I libs/include
MISC_FLAGS := -sWASM_WORKERS -O2
DEPFLAGS := -MT dist/app.js -MMD -MP -MF $(DIST_FOLDER)/main.d

CFLAGS = $(CLOSURE_FLAGS) $(LIB_FLAGS) $(MISC_FLAGS) $(DEPFLAGS) -sFETCH -msimd128 -mrelaxed-simd -sALLOW_MEMORY_GROWTH -fno-rtti -fno-exceptions -sEXPORT_KEEPALIVE=1 -s "EXPORTED_RUNTIME_METHODS=['dynCall', 'ccall', 'cwrap']"

all: serve

include $(DIST_FOLDER)/main.d

$(DIST_FOLDER):
	mkdir -p $(DIST_FOLDER)
dist:
	mkdir -p dist

dist/app.js: dist $(DIST_FOLDER) Makefile lib_webgpu.js
	em++ main.cpp $(CFLAGS) -o dist/app.js  

dist/index.html: $(DIST_FOLDER)
	cp index.html dist/

dist/fonts: $(DIST_FOLDER)
	cp -r fonts dist/

serve: dist/index.html dist/app.js dist/fonts
	cp -r dist/* ../

.PHONY: all
.PHONY: serve

