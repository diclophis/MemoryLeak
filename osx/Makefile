#foobar

#edit: main.o

#main.o: main.cpp
CXXFLAGS=-D_iPhoneVersion -DDESKTOP -I../jni
LDFLAGS=-framework OpenGL -framework GLUT

objects = main.o Engine.o RaptorIsland.o CaptureTheFlag.o MD2_File.o MD2_Manager.o MD2_Model.o \
Clock.o \
Color.o \
Obstacle.o \
OldPathway.o \
Path.o \
Pathway.o \
PlugIn.o \
SegmentedPath.o \
SegmentedPathway.o \
SimpleVehicle.o \
TerrainRayTest.o \
Vec3.o \
Vec3Utilities.o

raptor_island: $(objects)
	g++ -o raptor_island $(LDFLAGS) $(objects)

#
#raptor_island: $(objects)
#	g++ -o raptor_island $(LDFLAGS) $(objects)

%.o: ../jni/%.cpp
	g++ $(CXXFLAGS) -c $<

%.o: ../jni/OpenSteer/%.cpp
	g++ $(CXXFLAGS) -c $<
