#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.8...3.18)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
project(RDATA VERSION 1.0.0 DESCRIPTION "General library for data visulatization and basic computation")

#message(STATUS "Project source: ${PROJECT_SOURCE_DIR}")
#message(STATUS "Binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_CXX_FLAGS "-g -Wall -pg -Wno-shadow -Wno-pedantic -Wno-unused-parameter -Wno-overloaded-virtual -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable")

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT REQUIRED)

# ROOT version 6 required
if(ROOT_FOUND)
  STRING(REGEX MATCH "6.*" VERSION6MATCH ${ROOT_VERSION})
  if(NOT VERSION6MATCH)
    message(FATAL_ERROR "RDataLib requires ROOT 6")
  endif()
endif()

# Include ROOT's CMake functions for dictionary generation
#  since root6.20, the file is renamed and included by default, so include
#  only when we find the *old* name
if(EXISTS "${ROOT_DIR}/modules/RootNewMacros.cmake")
  include("${ROOT_DIR}/modules/RootNewMacros.cmake")
endif()

#include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${ROOT_INCLUDE_DIRS})
		    #${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/source/*.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB libsources ${PROJECT_SOURCE_DIR}/source/*.cxx)
file(GLOB libheaders ${PROJECT_SOURCE_DIR}/include/*.h)

#----------------------------------------------------------------------------
# Generate dictionaries, add ROOT libraries properties
#
add_library(RData STATIC ${sources})
set(libsuffix .a)
set(ROOT_LIBRARY_PROPERTIES ${ROOT_LIBRARY_PROPERTIES} SUFFIX ${libsuffix})
set_target_properties(RData PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
target_link_libraries(RData  PUBLIC ${ROOT_LIBRARIES} ROOT::Gui ROOT::Core ROOT::Imt ROOT::RIO ROOT::Net ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::ROOTVecOps ROOT::Tree ROOT::TreePlayer ROOT::Rint ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread ROOT::MultiProc ROOT::ROOTDataFrame -pthread)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
#install(TARGETS MOLLEROpt DESTINATION bin)
#install(TARGETS RData DESTINATION ${CMAKE_INSTALL_PREFIX})
#install(FILES ${PROJECT_BINARY_DIR}/RDataDictClasses_rdict.pcm DESTINATION ${PROJECT_SOURCE_DIR}/lib)
