# OSX Makefile, needs to know where the main src root is

#NOTE: wierd flags for doing static analysis
#CXX=/Users/jon/llvm/Debug+Asserts/bin/clang++
#CXX=g++
#CXX=~/Desktop/checker-249/libexec/c++-analyzer

build=build
CXXFLAGS=-Wall -g -D_iPhoneVersion -DDESKTOP -I../../src -I../../src/include -I../../src/assimp/BoostWorkaround
LDFLAGS=-framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -lpthread

objects = $(build)/main.o \
$(build)/Model.o \
$(build)/MachineGun.o \
$(build)/Engine.o \
$(build)/RunAndJump.o

objects   += $(patsubst %.cpp,%.o,  $(wildcard ../../src/assimp/*.cpp)) 
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../../src/contrib/irrXML/*.cpp)) 
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/ConvertUTF/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/unzip/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../../src/contrib/zlib/*.c))

raptor_island: $(build) $(objects)
	$(CXX) -o $(build)/raptor_island $(LDFLAGS) $(objects)

$(build)/main.o: main.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)

clean:
	touch $(build) && rm -R $(build) 
