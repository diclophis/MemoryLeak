# OSX Makefile, needs to know where the main src root is

build=build

CXXFLAGS=\
-O2 \
-s ASM_JS=1 \
-s TOTAL_MEMORY=268435456 \
-I../../src/assimp/BoostWorkaround \
-I../../src/include \
-I../../src/include/libmodplug \
-I../../src \
-DDESKTOP \
-I../../src/Box2D \
-I../../src/octree \
-DMODPLUG_BASIC_SUPPORT \
-DMODPLUG_FASTSOUNDLIB \
-DUSE_GLES2 \

CFLAGS=$(CXXFLAGS)

objects   = $(patsubst %,build/%, $(patsubst %.cpp,%.bc, $(wildcard *.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/ConvertUTF/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/unzip/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/zlib/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/yajl/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/libmodplug/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/*.c)))

HASH=$(shell md5sum build/raptor_island.js | cut -d" " -f 1)
hash_dir=~/emscripten.risingcode.com/public/$(HASH)

raptor_island: $(build) $(objects)
	EMCC_DEBUG=1 $(CXX) -O2 -s ASM_JS=1 -s TOTAL_MEMORY=268435456 $(objects) -o $(build)/raptor_island.bc
	EMCC_DEBUG=1 $(CXX) -O2 -s ASM_JS=1 -s TOTAL_MEMORY=268435456 -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free', '_start_game', '_command']" --js-library library_sink.js $(build)/raptor_island.bc -o $(build)/raptor_island.js

$(build)/%.bc: %.cpp
	EMCC_DEBUG=1 $(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.cpp
	EMCC_DEBUG=1 $(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.c
	EMCC_DEBUG=1 $(CC) $(CFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)
	mkdir -p $(build)/yajl
	mkdir -p $(build)/contrib/ConvertUTF
	mkdir -p $(build)/contrib/zlib
	mkdir -p $(build)/contrib/unzip
	mkdir -p $(build)/libmodplug

clean:
	rm -R /tmp/emscripten_temp
	rm -Rf $(build)

$(hash_dir):
	mkdir $(hash_dir)

install: $(hash_dir) build/raptor_island.js
	ln -nsf $(hash_dir) ~/emscripten.risingcode.com/public/latest
	ruby cache_manifest.rb ../../assets > ~/emscripten.risingcode.com/public/latest/index.appcache
	cp -R ../../assets ~/emscripten.risingcode.com/public/latest
	touch ~/emscripten.risingcode.com/public/latest/favicon.ico
	install -C index.html ~/emscripten.risingcode.com/public/latest
	install -C $(build)/raptor_island.js ~/emscripten.risingcode.com/public/latest
	#ruby s3_uploader.rb $(HASH)
