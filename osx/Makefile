#foobar

#edit: main.o


#main.o: main.cpp
#CXX=/Users/jon/llvm/Debug+Asserts/bin/clang++
#CXX=g++
#CXX=~/Desktop/checker-249/libexec/c++-analyzer
build=../build/osx
CXXFLAGS=-Wall -g -D_iPhoneVersion -DDESKTOP -I../jni -I../jni/include -I../jni/assimp/BoostWorkaround
LDFLAGS=-framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -lpthread

objects = $(build)/main.o \
$(build)/Model.o \
$(build)/MachineGun.o \
$(build)/Engine.o \
$(build)/RunAndJump.o

#OBJECTS   := $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) 
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../jni/assimp/*.cpp)) 
objects   += $(patsubst %.cpp,%.o,  $(wildcard ../jni/contrib/irrXML/*.cpp)) 
objects   += $(patsubst %.c,%.o,    $(wildcard ../jni/contrib/ConvertUTF/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../jni/contrib/unzip/*.c))
objects   += $(patsubst %.c,%.o,    $(wildcard ../jni/contrib/zlib/*.c))

raptor_island: $(build) $(objects)
	$(CXX) -o $(build)/raptor_island $(LDFLAGS) $(objects)

$(build)/main.o: main.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../jni/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build)/%.o: ../jni/OpenSteer/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(build):
	mkdir -p $(build)

clean:
	touch $(build) && rm -R $(build) 
