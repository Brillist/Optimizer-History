cmake_minimum_required(VERSION 3.12)

#### clevor_se ###################################################################################

project(
    clevor_se VERSION 1.0
    DESCRIPTION "Clevor Scheduling Engine"
    LANGUAGES CXX
)

#### functions ###################################################################################

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
    # group into "Source Files"
    set(GROUP "Source Files${GROUP}")
    # group into "Source Files" and "Header Files"
    #if ("${FILE}" MATCHES ".*\\.cpp")
    #  set(GROUP "Source Files${GROUP}")
    #elseif("${FILE}" MATCHES ".*\\.h")
    #  set(GROUP "Header Files${GROUP}")
    #endif()
    #message("source_group(${GROUP} FILES ${FILE})")
    source_group("${GROUP}" FILES "${FILE}")
  endforeach()
endfunction()

#### libcse ######################################################################################

# TARGET: libcse (shared library)
add_library(libcse STATIC)

# find libUTL++
find_package(libutl 1.0 REQUIRED)

# users should link against libutl::libutl_static
target_link_libraries(libcse PUBLIC libutl::libutl_static)

# include directories
set(CSE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}")
target_include_directories(libcse PUBLIC ${CSE_INCLUDE_DIRS})

# ${CSE_CPPS}: library source files
file(GLOB_RECURSE CSE_CPPS_LUT RELATIVE ${CMAKE_SOURCE_DIR} "lut/*.cpp")
file(GLOB_RECURSE CSE_CPPS_GOP RELATIVE ${CMAKE_SOURCE_DIR} "gop/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CLP RELATIVE ${CMAKE_SOURCE_DIR} "clp/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CLS RELATIVE ${CMAKE_SOURCE_DIR} "cls/*.cpp")
file(GLOB_RECURSE CSE_CPPS_CSE RELATIVE ${CMAKE_SOURCE_DIR} "cse/*.cpp")
set(CSE_CPPS ${CSE_CPPS_LUT} ${CSE_CPPS_GOP} ${CSE_CPPS_CLP} ${CSE_CPPS_CLS} ${CSE_CPPS_CSE})

# ${CSE_HDRS}: library header files
file(GLOB_RECURSE CSE_HDRS_LUT RELATIVE ${CMAKE_SOURCE_DIR} "lut/*.h")
file(GLOB_RECURSE CSE_HDRS_GOP RELATIVE ${CMAKE_SOURCE_DIR} "gop/*.h")
file(GLOB_RECURSE CSE_HDRS_CLP RELATIVE ${CMAKE_SOURCE_DIR} "clp/*.h")
file(GLOB_RECURSE CSE_HDRS_CLS RELATIVE ${CMAKE_SOURCE_DIR} "cls/*.h")
file(GLOB_RECURSE CSE_HDRS_CSE RELATIVE ${CMAKE_SOURCE_DIR} "cse/*.h")
set(CSE_HDRS ${CSE_HDRS_LUT} ${CSE_HDRS_GOP} ${CSE_HDRS_CLP} ${CSE_HDRS_CLS} ${CSE_HDRS_CSE})

# ${CSE_CPPS_AND_HDRS}: source and header files
set(CSE_CPPS_AND_HDRS ${CSE_CPPS} ${CSE_HDRS})

target_sources(libcse PRIVATE ${CSE_CPPS_AND_HDRS})
sourceGroups("${CSE_CPPS_AND_HDRS}") # groupings for Visual Studio

# MSVC-only: downstream must link with /WHOLEARCHIVE
if (MSVC)
  target_link_options(libcse INTERFACE
                      -WHOLEARCHIVE:$<TARGET_LINKER_FILE_NAME:libcse>)
endif()

#### clevor_se ###################################################################################

# clevor_se (executable)
add_executable(clevor_se main/clevor_se.cpp)

# link against libcse
target_link_libraries(clevor_se libcse)

#### bt ##########################################################################################

# build-test tool (executable) 
add_executable(bt main/bt.cpp)

# link against libcse
target_link_libraries(bt libcse)

