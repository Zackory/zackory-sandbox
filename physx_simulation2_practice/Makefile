CC=gcc
CXX=g++

PHYSX=/home/zackory/git/PhysXSDK

#-Wall -Wextra 
CXXFLAGS=-DNDEBUG -DPHYSX_PROFILE_SDK -std=c++11 -I/usr/include/eigen3 -I$(PHYSX)/Include -I$(PHYSX)/Samples/PxToolkit/include -I$(PHYSX)/Source/foundation/include -I$(PHYSX)/Samples/SampleFramework/renderer/include -I$(PHYSX)/Samples/SampleBase -I$(PHYSX)/Samples/SampleFramework/framework/include -I$(PHYSX)/Samples/SampleFramework/platform/include -Isource -Isource/renderer
LDFLAGS=-L$(PHYSX)/Lib/linux64 -L$(PHYSX)/Bin/linux64 -Lbuild/lib
LDLIBS=-Wl,--start-group -lCMAES -lgui -lmyUtils -lpath -lrenderer -lsimulator -lvisualization -lm -lz -lpthread -lrt -lGL -lGLU -lglut -lboost_system -lboost_thread -lboost_filesystem -lSM -lICE -lX11 -lXext -lXmu -lXi -lpng -lglog -lPhysX3_x64 -lSimulationController -lLowLevel -lPhysXProfileSDK -lPhysX3Extensions -lSceneQuery -lPhysX3Common_x64 -lPhysX3CharacterKinematic_x64 -lPhysX3Vehicle -lPhysX3Cooking_x64 -lPvdRuntime -lPxTask -Wl,--end-group -Wl,--rpath=$(PHYSX)/Bin/linux64

all: evaluator fakearm gown material policy

evaluator: apps/evaluator.cpp
	mkdir Build
	$(CXX) $(CXXFLAGS) $(LDFLAGS) apps/evaluator.cpp -o Build/evaluator $(LDLIBS)

fakearm: apps/fakearm_test.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) apps/fakearm_test.cpp -o Build/fakearm_test $(LDLIBS)

gown: apps/gown_test.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) apps/gown_test.cpp -o Build/gown_test $(LDLIBS)

material: apps/material_test.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) apps/material_test.cpp -o Build/material_test $(LDLIBS)

policy: apps/policySearch.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) apps/policySearch.cpp -o Build/policySearch $(LDLIBS)

.PHONY: clean
clean:
	rm -rf Build
	mkdir Build
