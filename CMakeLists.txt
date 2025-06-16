cmake_minimum_required(VERSION 3.10)
project(Single_pion_production_cmp)
set(CMAKE_CXX_STANDARD 17)

# Project directories
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_FILES
    ${SOURCE_DIR}/single_pion_main.cpp
    ${SOURCE_DIR}/Utils.cpp
)

# Check required environment variables
foreach(var GENIE GENIE_COMPARISONS ROOTSYS LOG4CPP)
    if(NOT DEFINED ENV{${var}})
        message(FATAL_ERROR "Missing environment variable: ${var}")
    endif()
endforeach()

# Environment variables
set(GENIE $ENV{GENIE})
set(GENIE_COMPARISONS $ENV{GENIE_COMPARISONS})
set(ROOTSYS $ENV{ROOTSYS})
set(LOG4CPP $ENV{LOG4CPP})

message(STATUS "GENIE: ${GENIE}")
message(STATUS "GENIE_COMPARISONS: ${GENIE_COMPARISONS}")
message(STATUS "ROOTSYS: ${ROOTSYS}")
message(STATUS "LOG4CPP: ${LOG4CPP}")

# ROOT
find_package(ROOT REQUIRED COMPONENTS Core Hist Gpad Graf MathCore Tree Physics)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

# Include and link directories
include_directories(
    ${INCLUDE_DIR}
    ${GENIE}/src
    ${GENIE_COMPARISONS}/src
    # ${ROOT_INCLUDE_DIRS}
    ${ROOTSYS}/include
    ${LOG4CPP}/include
    ${LIBXML2_INCLUDE_DIRS}
)

link_directories(
    ${GENIE}/lib
    ${GENIE_COMPARISONS}/lib
    ${LOG4CPP}/lib
)

# Execute `genie-config` and `genie-comparisons-config`
execute_process(COMMAND ${GENIE}/src/scripts/setup/genie-config --libs OUTPUT_VARIABLE GENIE_GENERATOR_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${GENIE_COMPARISONS}/src/scripts/setup/genie-comparisons-config --libs OUTPUT_VARIABLE GENIE_COMPARISONS_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Convert the output to CMake list format
# The output is initially a long string of commands that we need to split.
separate_arguments(GENIE_GENERATOR_LIBS)
separate_arguments(GENIE_COMPARISONS_LIBS)

# Output library list for debugging
message(STATUS "GENIE_GENERATOR_LIBS: ${GENIE_GENERATOR_LIBS}")
message(STATUS "GENIE_COMPARISONS_LIBS: ${GENIE_COMPARISONS_LIBS}")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Set linker flags like in Makefile
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -Wl,--no-undefined")

# Full ROOT library list manually (to match Makefile)
set(ROOT_EXTRA_LIBS
    Gui Core Imt RIO Net Hist Graf Graf3d Gpad ROOTVecOps Tree TreePlayer Rint Postscript
    Matrix Physics MathCore Thread MultiProc ROOTDataFrame Minuit Geom EG EGPythia6
    GenVector MathMore
)

# Extra GENIE modules that `genie-config` might miss
set(GENIE_EXTRA_LIBS
    -lGPhHadTens -lGPhNDcy -lGTlGeo -lGTlFlx -lGTlMcls
)

# Final linking
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GENIE_GENERATOR_LIBS}
    ${GENIE_COMPARISONS_LIBS}
    ${GENIE_EXTRA_LIBS}
    ${ROOT_LIBRARIES}
    ${ROOT_EXTRA_LIBS}
    ${LIBXML2_LIBRARIES}
    # missing libraries that caused the cmake and make nightmare I had.
    -llog4cpp 
    -L${GENIE}/lib
    -L${GENIE_COMPARISONS}/lib
    -L${LOG4CPP}/lib
    -L${ROOTSYS}/lib
    -L/home/laamiy/HEP/v6_428/lib -lPythia6
    -L/home/laamiy/HEP/LHAPDF6/BUILD/src/.libs -lLHAPDF
    -lgsl -lgslcblas -lm -ldl -rdynamic -lnsl
)

# Optional: output binary in bin/
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
