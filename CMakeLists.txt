cmake_minimum_required(VERSION 3.8...3.18)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
project(CMMonitor)

LIST( APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake/Modules
)

set(CMAKE_CXX_FLAGS "-g -Wall -pg -Wno-shadow -Wno-pedantic -Wno-unused-parameter -Wno-overloaded-virtual -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable")

find_package(ROOT REQUIRED)

set(RDATA_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/build/libRData.a")
set(RDATA_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/include")
set(ZMQ_LIBRARY "/usr/local/lib/libzmq.a")
set(ZMQ_INCLUDE_DIR "/usr/local/include") 

# ROOT version 6 required
if(ROOT_FOUND)
  message(STATUS "ROOT include: ${ROOT_INCLUDE_DIRS}")
  STRING(REGEX MATCH "6.*" VERSION6MATCH ${ROOT_VERSION})
  if(NOT VERSION6MATCH)
    message(FATAL_ERROR "MOLLEROpt requires ROOT 6")
  endif()
endif()

include(${ROOT_USE_FILE}) 

if(EXISTS "${ROOT_DIR}/modules/RootNewMacros.cmake")
  include("${ROOT_DIR}/modules/RootNewMacros.cmake")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${ROOT_INCLUDE_DIRS}
		    ${RDATA_INCLUDE_DIRS}
		    ${ZMQ_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/source/CMMonitor.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/CMMonitor.h)

message(STATUS "RData Library:  ${RDATA_LIBRARIES}")
message(STATUS "ROOT Library:  ${ROOT_LIBRARIES}")

ROOT_GENERATE_DICTIONARY(CMMonitorDict ${PROJECT_SOURCE_DIR}/include/CMMonitor.h LINKDEF ${PROJECT_SOURCE_DIR}/include/CMMonitorDictLinkDef.h)

ROOT_GENERATE_DICTIONARY(CMDataDict ${PROJECT_SOURCE_DIR}/include/CMData.h LINKDEF ${PROJECT_SOURCE_DIR}/include/CMDataDictLinkDef.h)


add_executable(CMMonitor ${PROJECT_SOURCE_DIR}/source/CMMonitor.cxx ${sources} ${headers} CMMonitorDict.cxx)
add_library(CMMonitorDictionaries SHARED ${sources} ${headers} CMMonitorDict.cxx)
target_link_libraries(CMMonitorDictionaries ${ROOT_LIBRARIES})

add_executable(CMData ${PROJECT_SOURCE_DIR}/source/CMData.cxx ${PROJECT_SOURCE_DIR}/include/CMData.h CMDataDict.cxx)
add_library(CMDataDictionaries SHARED ${PROJECT_SOURCE_DIR}/source/CMData.cxx ${PROJECT_SOURCE_DIR}/include/CMData.h CMDataDict.cxx)
target_link_libraries(CMDataDictionaries ${ROOT_LIBRARIES})

target_link_libraries(CMMonitor CMMonitorDictionaries ${ZMQ_LIBRARY} ${RDATA_LIBRARIES} ${ROOT_LIBRARIES} ROOT::Gui ROOT::Core ROOT::Imt ROOT::RIO ROOT::Net ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::ROOTVecOps ROOT::Tree ROOT::TreePlayer ROOT::Rint ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread ROOT::MultiProc ROOT::ROOTDataFrame -pthread)

target_link_libraries(CMData CMDataDictionaries ${ZMQ_LIBRARY} ${ROOT_LIBRARIES} ROOT::Gui ROOT::Core ROOT::Imt ROOT::RIO ROOT::Net ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::ROOTVecOps ROOT::Tree ROOT::TreePlayer ROOT::Rint ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread ROOT::MultiProc ROOT::ROOTDataFrame -pthread)
