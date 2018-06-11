cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(DHTAnalyzer)

include (Dart)
enable_testing()

option(ENABLE_ASTYLE  "Set to ON to enable AStyle formating of the code" ON)


set(SOURCES source/DHTAnalyzer.cpp
            source/DHTAnalyzer.h
            source/DHTAnalyzerSettings.cpp
            source/DHTAnalyzerSettings.h
            source/DHTAnalyzerResults.cpp
            source/DHTAnalyzerResults.h
            source/DHTSimulationDataGenerator.cpp
            source/DHTSimulationDataGenerator.h
            source/DHTAnalyzerHelpers.cpp
            source/DHTAnalyzerHelpers.h
)

add_library(DHTAnalyzer SHARED ${SOURCES})

# TODO - make an imported target for the AnalyzerLib
set (ANALYZER_SDK_ROOT "${PROJECT_SOURCE_DIR}/AnalyzerSDK")
if (APPLE)
    target_link_libraries(DHTAnalyzer ${ANALYZER_SDK_ROOT}/lib/libAnalyzer.dylib)
elseif(MSVC)
    if( CMAKE_CL_64 )
        target_link_libraries(DHTAnalyzer ${ANALYZER_SDK_ROOT}/lib/Analyzer64.lib)
    else()
         target_link_libraries(DHTAnalyzer ${ANALYZER_SDK_ROOT}/lib/Analyzer.lib)
    endif()
elseif(UNIX AND NOT APPLE)
 	target_link_libraries(DHTAnalyzer ${ANALYZER_SDK_ROOT}/lib/libAnalyzer.so)
endif()

target_include_directories(DHTAnalyzer PRIVATE ${ANALYZER_SDK_ROOT}/include)

#------------------------------------------------------------------------
# Testing

add_subdirectory(AnalyzerSDK/testlib)

add_executable(DHTTest tests/DHTTestDriver.cpp ${SOURCES})
target_link_libraries(DHTTest AnalyzerTestHarness)

add_test(DHTTest ${EXECUTABLE_OUTPUT_PATH}/DHTTest)

#------------------------------------------------------------------------
# AStyle 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include( ExternalAStyle )
