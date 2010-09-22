#foobar

#edit: main.o

#main.o: main.cpp
#CXX=/Users/jon/llvm/Debug+Asserts/bin/clang++
CXX=g++
build=../build/osx
CXXFLAGS=-Wall -g -D_iPhoneVersion -DDESKTOP -I../jni
LDFLAGS=-framework OpenGL -framework GLUT -framework Foundation -framework ApplicationServices -framework AppKit -lpthread

objects = $(build)/main.o \
$(build)/Engine.o \
$(build)/RaptorIsland.o \
$(build)/CaptureTheFlag.o \
$(build)/MD2_File.o \
$(build)/MD2_Manager.o \
$(build)/MD2_Model.o \
$(build)/Clock.o \
$(build)/Color.o \
$(build)/Obstacle.o \
$(build)/OldPathway.o \
$(build)/Path.o \
$(build)/Pathway.o \
$(build)/PlugIn.o \
$(build)/SegmentedPath.o \
$(build)/SegmentedPathway.o \
$(build)/SimpleVehicle.o \
$(build)/TerrainRayTest.o \
$(build)/Vec3.o \
$(build)/Vec3Utilities.o

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
