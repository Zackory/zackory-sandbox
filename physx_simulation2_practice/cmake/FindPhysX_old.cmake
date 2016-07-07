# physics (physx)
#if(USE_PHYSX)
  message("Began finding PhysX!")
  #if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PHYSX_LIB_SUFFIX "")
  #endif()
  set( PHYSX_ROOT "/home/zackory/git/PhysXSDK" CACHE PATH "physx root" )
#  set( PHYSX_ROOT "/home/zackory/git/PhysX-3.3/PhysXSDK" CACHE PATH "physx root" )
  set( LIB_SUFFIX 64 )
  IF(NOT EXISTS ${PHYSX_ROOT})
    message(FATAL_ERROR "physx root directory does not exist: ${PHYSX_ROOT}")
  endif ()

  FIND_PATH(
    PHYSX_INCLUDE_DIR PxPhysicsAPI.h
    PATHS "/usr/local" ${PHYSX_ROOT}
    PATH_SUFFIXES "Include"
    DOC "physx include directory")
  set (INCLUDES ${INCLUDES}
    ${PHYSX_INCLUDE_DIR}
  )

    set(PHYSX_INCLUDE_DIR
        ${PHYSX_INCLUDE_DIR}
#        ${PHYSX_SDK_ROOT}/Samples/SampleFramework/renderer/include/
#        ${PHYSX_SDK_ROOT}/Samples/SampleBase/
#        ${PHYSX_SDK_ROOT}/Samples/SampleFramework/framework/include/
#        ${PHYSX_SDK_ROOT}/Samples/SampleFramework/platform/include/
        ${PHYSX_SDK_ROOT}/Samples/PxToolkit/include
        ${PHYSX_SDK_ROOT}/Source/foundation/include
    )

message("Include Dir:")
message(${PHYSX_INCLUDE_DIR})
message(${INCLUDES})
message(${PHYSX_LIB_SUFFIX} "Helo")

  set(PHYSX_LIBRARIES_TO_FIND LowLevel LowLevelCloth SceneQuery SimulationController PhysX3_x64 PhysX3Common_x64 PhysX3Cooking_x64 PhysX3Extensions PhysXProfileSDK PxTask PhysX3CharacterKinematic_x64 PhysX3Vehicle PvdRuntime)
  foreach(LIB_TO_FIND ${PHYSX_LIBRARIES_TO_FIND})
    FIND_LIBRARY(
      PHYSX_LIBRARY_${LIB_TO_FIND}
      NAMES "${LIB_TO_FIND}${PHYSX_LIB_SUFFIX}"
      HINTS ${INCLUDES}/..
      PATH_SUFFIXES "lib${LIB_SUFFIX}" "Lib/linux${LIB_SUFFIX}" "Bin/linux${LIB_SUFFIX}")
    set (LIBS_PHYSX ${LIBS_PHYSX}
      ${PHYSX_LIBRARY_${LIB_TO_FIND}}
    )
  endforeach()
#endif(USE_PHYSX)