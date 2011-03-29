# OSX Makefile, needs to know where the main src root is

#NOTE: wierd flags for doing static analysis
#CXX=/Users/jon/llvm/Debug+Asserts/bin/clang++
#CXX=g++
#CXX=~/Desktop/checker-249/libexec/c++-analyzer

build=build
CXXFLAGS=-Wall -g -D__MACOSX_CORE__ -D_iPhoneVersion -DDESKTOP -DHAVE_SETENV -I../../src -I../../src/include/curl -I../../src/include/ares -I../../src -I../../src/include -I../../src/include/libmodplug -I../../src/octree -I../../src/assimp/BoostWorkaround -I../../src/yajl -I../../src/yajl/api
CFLAGS=$(CXXFLAGS)
LDFLAGS=-framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -lpthread -framework WebKit -lresolv

objects   = $(patsubst %,build/%, $(patsubst %.mm,%.o, $(wildcard *.mm)))
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

$(build)/%.o: %.mm
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
	touch $(build) && rm -R $(build) && rm -R ../../src/*.o

wang:
	echo 1 $(objects) 

YourApp.app: Info.plist $(build)/raptor_island version.plist InfoPlist.strings
	mkdir YourApp.app    
	mkdir YourApp.app/Contents
	mkdir YourApp.app/Contents/MacOS
	mkdir YourApp.app/Contents/Resources
	mkdir YourApp.app/Contents/Resources/English.lproj
	cp Info.plist YourApp.app/Contents/
	cp version.plist YourApp.app/Contents/
	cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
	echo -n 'APPL????' > YourApp.app/Contents/PkgInfo
	cp $(build)/raptor_island YourApp.app/Contents/MacOS/YourApp
	cp Icons.icns YourApp.app/Contents/Resources/
