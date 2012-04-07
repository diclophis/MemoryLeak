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
-O0 \

#-m64 -g -pg -DDESKTOP -DHAVE_SETENV -D_iPhoneVersion \
-DMODPLUG_BASIC_SUPPORT \
-I/usr/include \
-I../../src/OpenSteer

CFLAGS=$(CXXFLAGS)

#LDFLAGS=-lpthread -lresolv -lGL -lglut -lSOIL -lasound -lGLU -lm -lstdc++

objects   = $(patsubst %,build/%, $(patsubst %.cpp,%.bc, $(wildcard *.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/contrib/irrXML/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/ConvertUTF/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/unzip/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.bc, $(wildcard ../../src/contrib/zlib/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/assimp/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/libmodplug/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Collision/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Collision/Shapes/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/Contacts/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Dynamics/Joints/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/Box2D/Common/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.bc, $(wildcard ../../src/OpenSteer/*.cpp)))

#~/emscripten/emcc --llvm-opts 0 -O0 -s LINKABLE=1 $(build)/raptor_island.bc -o $(build)/raptor_island.js
#~/emscripten/emcc --llvm-opts 0 -O0 --preload-file ../../assets -s LINKABLE=1 $(build)/raptor_island.bc -o $(build)/raptor_island.js

raptor_island: $(build) $(objects)
	#$(CXX) -s LINKABLE=1 --llvm-opts 0 -O0 $(objects) -o $(build)/raptor_island.bc
	#~/emscripten/emcc --llvm-opts 0 -O0 -s LINKABLE=1 $(build)/raptor_island.bc -o $(build)/raptor_island.html
	$(CXX) $(objects) -o $(build)/raptor_island.bc
	~/emscripten/emcc $(build)/raptor_island.bc -o $(build)/raptor_island.js

#$(CXX) $(objects)
#$(CXX) $(build)/raptor_island.bc -o $(build)/raptor_island.js
#$(CXX) $(build)/raptor_island.bc -o $(build)/raptor_island.html
  
$(build)/%.bc: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.bc: ../../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

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
	rm -Rf $(build)

install:
	cp index.html ~/emscripten.risingcode.com/public/index.html
	cp $(build)/raptor_island.js ~/emscripten.risingcode.com/public/raptor_island.js
