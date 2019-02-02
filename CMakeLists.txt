cmake_minimum_required(VERSION 3.5)

project(
    clevor_se VERSION 1.0
    DESCRIPTION "Clevor Scheduling Engine"
    LANGUAGES CXX
)

# sourceGroups(SRCS)
#   Create source groups based on sourcedir-relative paths of source files.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
function(sourceGroups SRCS)
  foreach(FILE ${SRCS}) 
    # get absolute path to source file
    get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
    # get absolute path to source file's parent directory
    get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
    # remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    # make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    # group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.cpp")
      set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
      set(GROUP "Header Files${GROUP}")
    endif()
    #message("source_group(${GROUP} FILES ${FILE})")
    source_group("${GROUP}" FILES "${FILE}")
  endforeach()
endfunction()

# targetCompileOptions(TARGET)
#   Set project-wide compile options for a target.
function(targetCompileOptions TARGET)
  target_compile_options(${TARGET} PUBLIC ${COMPILE_OPTIONS_COMMON})
  target_compile_options(${TARGET} PUBLIC "$<$<CONFIG:DEBUG>:${COMPILE_OPTIONS_DEBUG}>")
  target_compile_options(${TARGET} PUBLIC "$<$<CONFIG:RELEASE>:${COMPILE_OPTIONS_RELEASE}>")
endfunction()

# libutl vars
if (MSVC)
    set(UTL_INCLUDE_DIR ../libutl)
    set(UTL_STATIC_DEBUG ../../libutl/x64/debug/libutl)
    set(UTL_STATIC_RELEASE ../../libutl/x64/release/libutl)
else()
    set(UTL_STATIC_DEBUG utl_g)
    set(UTL_STATIC_RELEASE utl)
endif()

# compiler options
if (MSVC)
    set(COMPILE_OPTIONS_COMMON /W1
                               /std:c++17
                               /permissive-
                               /MP
                               /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
                               /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1
                               /D_CRT_SECURE_NO_DEPRECATE
                               /D_CRT_NONSTDC_NO_DEPRECATE
                               /DUTL_CC=UTL_CC_MSVC
                               /DUTL_HOST_OS=UTL_OS_WINDOWS)
    set(COMPILE_OPTIONS_DEBUG /DDEBUG)
    set(COMPILE_OPTIONS_RELEASE /DRELEASE)
else()
    set(COMPILE_OPTIONS_COMMON -Wall
                               -Wno-uninitialized
                               -std=gnu++17
                               -fstrict-aliasing
                               -DUTL_CC=UTL_CC_GCC
                               -DUTL_HOST_OS=UTL_OS_LINUX)
    set(COMPILE_OPTIONS_DEBUG -DDEBUG)
    set(COMPILE_OPTIONS_RELEASE -DRELEASE)
endif()

# TARGET: libcse (static library)
add_library(libcse STATIC)

# libcse: compile options
targetCompileOptions(libcse)

# libcse: users need ws2_32 (Winsock2) library
target_link_libraries(libcse INTERFACE ws2_32)

# libcse: include directories
set(CSE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}")
target_include_directories(libcse PUBLIC ${UTL_INCLUDE_DIR} ${CSE_INCLUDE_DIRS})

# libcse: source files
file(GLOB_RECURSE CSE_HDRS_LUT RELATIVE ${CMAKE_SOURCE_DIR} "lut/*.h")
file(GLOB_RECURSE CSE_HDRS_GOP RELATIVE ${CMAKE_SOURCE_DIR} "gop/*.h")
file(GLOB_RECURSE CSE_HDRS_CLP RELATIVE ${CMAKE_SOURCE_DIR} "clp/*.h")
file(GLOB_RECURSE CSE_HDRS_CLS RELATIVE ${CMAKE_SOURCE_DIR} "cls/*.h")
file(GLOB_RECURSE CSE_HDRS_CSE RELATIVE ${CMAKE_SOURCE_DIR} "cse/*.h")
file(GLOB_RECURSE CSE_HDRS_MPS RELATIVE ${CMAKE_SOURCE_DIR} "mps/*.h")
file(GLOB_RECURSE CSE_HDRS_MRP RELATIVE ${CMAKE_SOURCE_DIR} "mrp/*.h")
set(CSE_HDRS ${CSE_HDRS_LUT} ${CSE_HDRS_GOP} ${CSE_HDRS_CLP} ${CSE_HDRS_CLS}
             ${CSE_HDRS_CSE} ${CSE_HDRS_MPS} ${CSE_HDRS_MRP})
file(GLOB_RECURSE CSE_CPPS_LUT RELATIVE ${CMAKE_SOURCE_DIR} "lut/*.cpp")
file(GLOB_RECURSE CSE_CPPS_GOP RELATIVE ${CMAKE_SOURCE_DIR} "gop/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CLP RELATIVE ${CMAKE_SOURCE_DIR} "clp/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CLS RELATIVE ${CMAKE_SOURCE_DIR} "cls/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CSE RELATIVE ${CMAKE_SOURCE_DIR} "cse/*.cpp")
file(GLOB_RECURSE CSE_CPPS_MPS RELATIVE ${CMAKE_SOURCE_DIR} "mps/*.cpp")
file(GLOB_RECURSE CSE_CPPS_MRP RELATIVE ${CMAKE_SOURCE_DIR} "mrp/*.cpp")
set(CSE_CPPS ${CSE_CPPS_LUT} ${CSE_CPPS_GOP} ${CSE_CPPS_CLP} ${CSE_CPPS_CLS}
             ${CSE_CPPS_CSE} ${CSE_CPPS_MPS} ${CSE_CPPS_MRP})
set(CSE_HDRS_AND_CPPS ${CSE_HDRS} ${CSE_CPPS})
target_sources(libcse PRIVATE ${CSE_HDRS_AND_CPPS})
sourceGroups("${CSE_HDRS_AND_CPPS}") # groupings for Visual Studio

# TARGET: clevor_se (executable)
add_executable(clevor_se main/clevor_se.cpp)

# clevor_se: include directories
target_include_directories(clevor_se PRIVATE main ${CSE_INCLUDE_DIRS} ${UTL_INCLUDE_DIR})

# clevor_se: link libraries
target_link_libraries(clevor_se libcse debug ${UTL_STATIC_DEBUG} optimized ${UTL_STATIC_RELEASE})

# TARGET: bt (executable) 
add_executable(bt main/bt.cpp)

# bt: compile options
targetCompileOptions(bt)

# bt: include directories
target_include_directories(bt PRIVATE main ${CSE_INCLUDE_DIRS} ${UTL_INCLUDE_DIR})

# bt: link libraries
target_link_libraries(bt libcse debug ${UTL_STATIC_DEBUG} optimized ${UTL_STATIC_RELEASE})
