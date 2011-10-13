# OSX Makefile, needs to know where the main src root is

build=build
CXXFLAGS=\
-Wno-unknown-pragmas -g -pg -DDESKTOP -DHAVE_SETENV \
-I/usr/include \
-I../../src \
-I../../src/include \
-I../../src/include/libmodplug \
-I../../src/octree \
-I../../src/assimp/BoostWorkaround \
-I../../src/Box2D

#-I../../src/include/curl \
#-I../../src/include/ares \
#-I../../src/yajl \
#-I../../src/yajl/api \
#-I../../src/lua \
#-I../../src/oolua \
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/lua/*.c)))
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/oolua/*.cpp)))
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/ares/*.c)))
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/curl/*.c)))
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/yajil/*.c)))

CFLAGS=$(CXXFLAGS)
LDFLAGS=-g -pg -lpthread -lresolv -lGL -lglut -lSOIL -lasound

objects   = $(patsubst %,build/%, $(patsubst %.cpp,%.o, $(wildcard *.cpp)))
#objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/assimp/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/contrib/irrXML/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/ConvertUTF/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/unzip/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/contrib/zlib/*.c)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/libmodplug/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Collision/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Collision/Shapes/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Dynamics/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Dynamics/Contacts/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Dynamics/Joints/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/Box2D/Common/*.cpp)))

raptor_island: $(build) $(objects)
	$(CC) -o $(build)/raptor_island $(LDFLAGS) $(objects)

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
	mkdir -p $(build)/libmodplug
	mkdir -p $(build)/Box2D
	mkdir -p $(build)/Box2D/Collision
	mkdir -p $(build)/Box2D/Collision/Shapes
	mkdir -p $(build)/Box2D/Common
	mkdir -p $(build)/Box2D/Dynamics
	mkdir -p $(build)/Box2D/Dynamics/Contacts
	mkdir -p $(build)/Box2D/Dynamics/Joints
	mkdir -p $(build)/Box2D/Dynamics/Rope

clean:
	rm -Rf $(build)
