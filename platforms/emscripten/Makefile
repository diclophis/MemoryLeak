# OSX Makefile, needs to know where the main src root is

build=build

CXXFLAGS=\
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
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/contrib/irrXML/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/ConvertUTF/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/unzip/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/zlib/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/assimp/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/libmodplug/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Collision/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Collision/Shapes/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/Contacts/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/Joints/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Common/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/OpenSteer/*.cpp)))

HASH=$(shell md5sum build/raptor_island.js | cut -d" " -f 1)
hash_dir=~/emscripten.risingcode.com/public/$(HASH)

raptor_island: $(build) $(objects)
	EMCC_DEBUG=1 $(CXX) $(objects) -o $(build)/raptor_island.bc
	EMCC_DEBUG=1 $(CXX) --closure 0 -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free', '_start_game']" --js-library library_sink.js $(build)/raptor_island.bc -o $(build)/raptor_island.js

$(build)/%.bc: %.cpp
	EMCC_DEBUG=1 $(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.cpp
	EMCC_DEBUG=1 $(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.c
	EMCC_DEBUG=1 $(CC) $(CFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)
	mkdir -p $(build)/contrib/irrXML
	mkdir -p $(build)/contrib/ConvertUTF
	mkdir -p $(build)/contrib/zlib
	mkdir -p $(build)/contrib/unzip
	mkdir -p $(build)/assimp
	mkdir -p $(build)/libmodplug
	mkdir -p $(build)/Box2D
	mkdir -p $(build)/Box2D/Collision
	mkdir -p $(build)/Box2D/Collision/Shapes
	mkdir -p $(build)/Box2D/Common
	mkdir -p $(build)/Box2D/Dynamics
	mkdir -p $(build)/Box2D/Dynamics/Contacts
	mkdir -p $(build)/Box2D/Dynamics/Joints
	mkdir -p $(build)/Box2D/Dynamics/Rope
	mkdir -p $(build)/OpenSteer

clean:
	rm -R /tmp/emscripten_temp
	rm -Rf $(build)

$(hash_dir):
	mkdir $(hash_dir)

install: $(hash_dir)
	ln -nsf $(hash_dir) ~/emscripten.risingcode.com/public/latest
	ruby cache_manifest.rb ../../assets > ~/emscripten.risingcode.com/public/latest/index.appcache
	cp -R ../../assets ~/emscripten.risingcode.com/public/latest
	touch ~/emscripten.risingcode.com/public/latest/favicon.ico
	install -C index.html ~/emscripten.risingcode.com/public/latest
	install -C sink.js ~/emscripten.risingcode.com/public/latest
	install -C library_sink.js ~/emscripten.risingcode.com/public/latest
	install -C $(build)/raptor_island.js ~/emscripten.risingcode.com/public/latest
