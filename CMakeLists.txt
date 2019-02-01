cmake_minimum_required(VERSION 3.5)

project(
    clevor_se VERSION 1.0
    DESCRIPTION "Clevor Scheduling Engine"
    LANGUAGES CXX
)

# libutl vars
if (MSVC)
    set(UTL_INCLUDE_DIR ../../libutl/libutl)
    set(UTL_STATIC_DEBUG ../../libutl/x64/debug/libutl.a)
    set(UTL_STATIC_RELEASE ../../libutl/x64/release/libutl.a)
else()
    set(UTL_INCLUDE_DIR /usr/include/libutl)
    set(UTL_STATIC_DEBUG libutl_g.a)
    set(UTL_STATIC_RELEASE libutl.a)
endif()

# cse target (static library)
add_library(cse STATIC)

# cse: include directories
set(CSE_INCLUDE_DIRS "include lut gop clp cls cse")
target_include_directories(cse PUBLIC ${CSE_INCLUDE_DIRS} ${UTL_INCLUDE_DIR})

# cse: source files
file(GLOB_RECURSE CSE_SOURCES_LUT RELATIVE ${CMAKE_SOURCE_DIR} "lut/*.cpp")
file(GLOB_RECURSE CSE_SOURCES_GOP RELATIVE ${CMAKE_SOURCE_DIR} "gop/*.cpp")
file(GLOB_RECURSE CSE_SOURCES_CLP RELATIVE ${CMAKE_SOURCE_DIR} "clp/*.cpp")
file(GLOB_RECURSE CSE_SOURCES_CLS RELATIVE ${CMAKE_SOURCE_DIR} "cls/*.cpp")
file(GLOB_RECURSE CSE_SOURCES_CSE RELATIVE ${CMAKE_SOURCE_DIR} "cse/*.cpp")
set(CSE_SOURCES ${CSE_SOURCES_LUT} ${CSE_SOURCES_GOP} ${CSE_SOURCES_CLP} ${CSE_SOURCES_CLS} ${CSE_SOURCES_CSE})
target_sources(cse PRIVATE ${CSE_SOURCES})

# clevor_se target (executable)
add_executable(clevor_se main/clevor_se.cpp)

# clevor_se: include directories
target_include_directories(clevor_se PRIVATE main ${CSE_INCLUDE_DIRS} ${UTL_INCLUDE_DIR})

# clevor_se: link libraries
target_link_libraries(clevor_se cse debug ${UTL_STATIC_DEBUG} release ${UTL_STATIC_RELEASE})

# bt target (executable)
add_executable(bt main/bt.cpp)

# bt: include directories
target_include_directories(bt PRIVATE main ${CSE_INCLUDE_DIRS} ${UTL_INCLUDE_DIR})

# bt: link libraries
target_link_libraries(bt cse debug ${UTL_STATIC_DEBUG} release ${UTL_STATIC_RELEASE})
