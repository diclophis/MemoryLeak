# OSX Makefile, needs to know where the main src root is

#NOTE: wierd flags for doing static analysis
#CXX=/Users/jon/llvm/Debug+Asserts/bin/clang++
#CXX=g++
#CXX=~/Desktop/checker-249/libexec/c++-analyzer

build=build
CXXFLAGS=-Wall -g -D__MACOSX_CORE__ -D_iPhoneVersion -DDESKTOP -I../../src -I../../src/include -I../../src/assimp/BoostWorkaround -I../../src/yajl -I../../src/yajl/api
CFLAGS=$(CXXFLAGS)
LDFLAGS=-framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -lpthread -framework WebKit

objects   = $(patsubst %.mm,%.o,  $(wildcard ../../platforms/osx/*.mm))
objects   += $(patsubst %.m,%.o,  $(wildcard ../../platforms/osx/*.m))
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../../src/*.cpp))
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../../src/assimp/*.cpp)) 
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../../src/contrib/irrXML/*.cpp)) 
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/ConvertUTF/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/unzip/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/zlib/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/yajl/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/yajl/api/*.c))

raptor_island: $(build) $(objects)
	$(CXX) -o $(build)/raptor_island $(LDFLAGS) $(objects)

../../platforms/osx/main.o: main.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)

clean:
	touch $(build) && rm -R $(build) && rm -R ../../src/*.o
