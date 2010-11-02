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
$(build)/Engine.o \
$(build)/MachineGun.o \
$(build)/RaptorIsland.o \
$(build)/CaptureTheFlag.o \
$(build)/RunAndJump.o \
$(build)/MD2_File.o \
$(build)/MD2_Manager.o \
$(build)/MD2_Model.o \
$(build)/Clock.o \
$(build)/Color.o \
$(build)/Obstacle.o \
$(build)/OldPathway.o \
$(build)/Path.o \
$(build)/Pathway.o \
$(build)/SegmentedPath.o \
$(build)/SegmentedPathway.o \
$(build)/SimpleVehicle.o \
$(build)/TerrainRayTest.o \
$(build)/Vec3.o \
$(build)/Vec3Utilities.o

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
