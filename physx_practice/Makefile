CC=gcc
CXX=g++

CXXFLAGS=-DNDEBUG -DPHYSX_PROFILE_SDK -DRENDER_SNIPPET -std=c++11 -Wall -Wextra -I/home/zackory/git/PhysXSDK/Include -I/home/zackory/git/PhysXSDK/Snippets
# -I/home/zackory/git/PhysXSDK/Include/extensions
# -I/home/zackory/git/PhysXSDK/Include/foundation
LDFLAGS=-L/home/zackory/git/PhysXSDK/Lib/linux64 -L/home/zackory/git/PhysXSDK/Bin/linux64 -L/home/zackory/git/PhysXSDK/Snippets/lib/linux64
LDLIBS=-lSnippetRender -lSnippetUtils -lm -lz -lpthread -lrt -lGL -lGLU -lglut -lPhysX3_x64 -lSimulationController -lLowLevel -lPhysXProfileSDK -lPhysX3Extensions -lSceneQuery -lPhysX3Common_x64 -lPhysX3CharacterKinematic_x64 -lPhysX3Vehicle -lPhysX3Cooking_x64 -lPvdRuntime -lPxTask -Wl,--rpath=/home/zackory/git/PhysXSDK/Bin/linux64
# INC=-I/home/zackory/git/PhysXSDK/Include

#OBJECTS=SampleBox.o
OBJECTS=SnippetCommon/ClassicMain.o SnippetHelloWorld.o SnippetHelloWorldRender.o

sample: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o SnippetHelloWorld $(LDLIBS)
#	$(CXX) $(LDFLAGS) $(OBJECTS) -o SampleBox $(LDLIBS)

all:sample

.PHONY: clean
clean:
	rm -rf *~ *.o SnippetHelloWorld SnippetCommon/*.o
