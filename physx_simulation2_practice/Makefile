CC=gcc
CXX=g++

PHYSX=/home/zackory/git/PhysXSDK

CXXFLAGS=-DNDEBUG -DPHYSX_PROFILE_SDK -std=c++11 -Wall -Wextra -I/usr/include/eigen3 -I$(PHYSX)/Include -I$(PHYSX)/Samples/PxToolkit/include -I$(PHYSX)/Source/foundation/include -I$(PHYSX)/Samples/SampleFramework/renderer/include -I$(PHYSX)/Samples/SampleBase -I$(PHYSX)/Samples/SampleFramework/framework/include -I$(PHYSX)/Samples/SampleFramework/platform/include -Isource -Isource/renderer
LDFLAGS=-L$(PHYSX)/Lib/linux64 -L$(PHYSX)/Bin/linux64 -Lbuild/lib
LDLIBS=-Wl,--start-group -lCMAES -lgui -lmyUtils -lpath -lrenderer -lsimulator -lvisualization -lm -lz -lpthread -lrt -lGL -lGLU -lglut -lboost_system -lboost_thread -lboost_filesystem -lSM -lICE -lX11 -lXext -lXmu -lXi -lpng -lglog -lPhysX3_x64 -lSimulationController -lLowLevel -lPhysXProfileSDK -lPhysX3Extensions -lSceneQuery -lPhysX3Common_x64 -lPhysX3CharacterKinematic_x64 -lPhysX3Vehicle -lPhysX3Cooking_x64 -lPvdRuntime -lPxTask -Wl,--end-group -Wl,--rpath=$(PHYSX)/Bin/linux64

OBJDIR_DEBUG = Build
OBJ_DEBUG = $(OBJDIR_DEBUG)/apps/evaluator.o $(OBJDIR_DEBUG)/apps/fakearm_test.o $(OBJDIR_DEBUG)/apps/gown_test.o $(OBJDIR_DEBUG)/apps/material_test.o $(OBJDIR_DEBUG)/apps/policySearch.o

OBJECTS=apps/evaluator.o

#$(OBJ_DEBUG): $(OBJDIR_DEBUG)/%.o: %.cpp
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@ $(LDLIBS)

sample: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o Build/evaluator $(LDLIBS)

#all:$(OBJ_DEBUG)
all:sample

.PHONY: clean
clean:
	rm -rf *~ *.o Build/*.o
