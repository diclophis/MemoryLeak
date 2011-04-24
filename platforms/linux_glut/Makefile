# OSX Makefile, needs to know where the main src root is

build=build
CXXFLAGS=-Wall -g -D__MACOSX_CORE__ -D_iPhoneVersion -DDESKTOP -DHAVE_SETENV -I../../src -I../../src/include/curl -I../../src/include/ares -I../../src -I../../src/include -I../../src/include/libmodplug -I../../src/octree -I../../src/assimp/BoostWorkaround -I../../src/yajl -I../../src/yajl/api
CFLAGS=$(CXXFLAGS)
LDFLAGS=-lpthread -lresolv -lGL -lglut -lSOIL -lasound

objects   = $(patsubst %,build/%, $(patsubst %.cpp,%.o, $(wildcard *.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/ares/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/curl/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/assimp/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/contrib/irrXML/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/ConvertUTF/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/unzip/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/zlib/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/yajil/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/libmodplug/*.cpp)))

raptor_island: $(build) $(objects)
	$(CXX) -o $(build)/raptor_island $(LDFLAGS) $(objects)

$(build)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)
	mkdir -p $(build)/assimp
	mkdir -p $(build)/contrib/irrXML
	mkdir -p $(build)/contrib/ConvertUTF
	mkdir -p $(build)/contrib/zlib
	mkdir -p $(build)/contrib/unzip
	mkdir -p $(build)/yajil
	mkdir -p $(build)/libmodplug
	mkdir -p $(build)/curl
	mkdir -p $(build)/ares

clean:
	rm -Rf $(build)
