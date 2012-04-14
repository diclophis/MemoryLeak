# OSX Makefile

build=build

CXXFLAGS=\
-m64 \
-Wall \
-g \
-pg \
-DDESKTOP \
-DHAVE_SETENV \
-DMODPLUG_BASIC_SUPPORT \
-D__MACOSX_CORE__ \
-D_iPhoneVersion \
-I/usr/include \
-I../../src \
-I../../src/include \
-I../../src/include/libmodplug \
-I../../src/octree \
-I../../src/assimp/BoostWorkaround \
-I../../src/Box2D \
-I../../src/OpenSteer \
-DUSE_GLES2 \

CFLAGS=$(CXXFLAGS)
LDFLAGS=-m64 -arch x86_64 -framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -framework AudioToolbox -framework AudioUnit -framework CoreAudio -framework Accelerate -lpthread -lresolv

objects   = $(patsubst %,build/%, $(patsubst %.mm,%.o, $(wildcard *.mm)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/*.cpp)))
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.c,%.o, $(wildcard ../../src/*.c)))
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
objects   += $(patsubst ../../src/%,build/%, $(patsubst %.cpp,%.o, $(wildcard ../../src/OpenSteer/*.cpp)))

raptor_island: $(build) $(objects)
	$(CXX) $(LDFLAGS) -o $(build)/raptor_island $(objects)

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
	touch $(build) && rm -R $(build)

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
