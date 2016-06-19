cmake_minimum_required(VERSION 2.8.3)
get_filename_component(dir_name ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${dir_name})

######## set the compilers and paths to boost and physx ######
if (APPLE)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_C_COMPILER clang)
elseif (UNIX)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_C_COMPILER gcc)
endif()

if (NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT /usr/lib/x86_64-linux-gnu)
endif()
#if (NOT DEFINED PHYSX_SDK_ROOT)
#    set(PHYSX_SDK_ROOT /home/zackory/git/PhysXSDK)
#endif()
##############################################################

add_definitions("-Wall -Wextra")

set(CMAKE_DEBUG_POSTFIX "d")
if (APPLE)
  list(APPEND CMAKE_CXX_FLAGS "-DNDEBUG -O3 -std=c++11 -stdlib=libc++ -lz ${CMAKE_CXX_FLAGS}")
else ()
  list(APPEND CMAKE_CXX_FLAGS "-DNDEBUG -std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

set(HEADER_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/Include)


# Configure files
message(STATUS "")
message(STATUS " [ Configured files ] ")

set(px_config_in ${CMAKE_SOURCE_DIR}/source/config.h.in)
set(px_config_out ${CMAKE_BINARY_DIR}/source/config.h)
message(STATUS ${px_config_out})
configure_file(${px_config_in} ${px_config_out} @ONLY)
install(FILES ${px_config_out} DESTINATION ${CMAKE_INSTALL_PREFIX}/source)

message(STATUS "")

###############
# Find dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Eigen REQUIRED)
find_package(glog REQUIRED)
#find_package(PhysX REQUIRED)
#find_package(png REQUIRED)
#if (APPLE)
#  find_package(OpenGL REQUIRED)
#else ()
#  find_package(GL REQUIRED)
#endif()
find_package(DART NO_MODULE)


# GLUT
#if(WIN32 AND NOT CYGWIN)
#    set(GLUT_INCLUDE_DIR "@CMAKE_INSTALL_PREFIX@/include")
#    set(GLUT_LIBRARIES glut32)
#else()
#    find_package(GLUT REQUIRED)
#    set(GLUT_LIBRARIES ${GLUT_glut_LIBRARY})
#endif()

# Boost
find_package(Boost REQUIRED system thread filesystem)

message(${Boost_LIBRARY_DIRS})
message(${Boost_LIBRARIES})

include_directories(BEFORE ${PROJECT_SOURCE_DIR})
include_directories(${Eigen_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
#include_directories(${OPENGL_INCLUDE_DIR})
#include_directories(${GLUT_INCLUDE_DIR})
#include_directories(${PHYSX_INCLUDE_DIR})
#include_directories(${PNG_INCLUDE_DIRS})
include_directories(${GLOG_INCLUDE_DIRS})

include_directories("${CMAKE_BINARY_DIR}/source/")
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/source/")

message("WOOH!")
#message(${PHYSX_LIB_DIR})
#message(${LIBS_PHYSX})

link_directories("${CMAKE_BINARY_DIR}/lib")
link_directories("${CMAKE_INSTALL_PREFIX}/lib")
link_directories(${Boost_LIBRARY_DIRS})
#link_directories(${PHYSX_LIB_DIR})
#link_directories(${LIBS_PHYSX})

# Main library

set(PHYSX_DEPENDENCIES ${FCL_LIBRARIES}
#                           ${Assimp_LIBRARIES}
                           ${Boost_LIBRARIES}
#                           ${OPENGL_LIBRARIES}
#                           ${GLUT_LIBRARY}
#			   ${PHYSX_LIBRARIES}
#			   ${LIBS_PHYSX}
#			   ${PNG_LIBRARIES}
			   ${GLOG_LIBRARIES}
			   )

message("GT!!")
message(${PHYSX_DEPENDENCIES})
message("Next!")

add_subdirectory(source)

set (PHYSX_DEPENDENCIES ${PHYSX_DEPENDENCIES}
			CMAES
#                        simulator
#                        gui
			myUtils
#			renderer
#			path
#			visualization
)

message(${PHYSX_DEPENDENCIES})
message("Final!")

file(GLOB apps_source "apps/*.cpp")
LIST(SORT apps_source)

message(STATUS "")
message(STATUS " [ Configuring Applications: ] ")
foreach(app_src_file ${apps_source})
    get_filename_component(app_base ${app_src_file} NAME_WE)
    message(STATUS "Adding application ${app_base}")
    add_executable(${app_base} ${app_src_file})
    target_link_libraries(${app_base} -Wl,--start-group ${PHYSX_DEPENDENCIES} z rt m pthread -Wl,--end-group)
#    target_link_libraries(${app_base} ${PHYSX_DEPENDENCIES} z rt m pthread)
endforeach(app_src_file)

