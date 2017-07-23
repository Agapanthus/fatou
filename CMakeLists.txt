#////////////////////////////////////////////////////////////////#
#*********************[       Fatou        ]*********************#
#
# Copyright 2017 by Eric Skaliks
#
 
# Version 3.6 is necessary to set the msvc startup project! Otherwise, 2.8.12 is sufficent.
cmake_minimum_required(VERSION 3.6)
project (fatou)
 
set(FATOU_BUILD_DIR ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Where to build fatou.")

set(CMAKE_BINARY_DIR ${FATOU_BUILD_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})


#////////////////////////////////////////////////////////////////#
#*********************[ PRECOMPILED HEADERS ]********************#

# Iarsmoa, 2009: https://stackoverflow.com/a/1380048/6144727
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)


#////////////////////////////////////////////////////////////////#
#*********************[       Files        ]*********************#

include_directories(third_party/glfw/include)
include_directories(third_party/glew/include)
include_directories(third_party/utfcpp/source)
include_directories(third_party/utfcpp/source/utf8)

file(GLOB_RECURSE SRC_MAIN src/*.cpp src/*.h)
file(GLOB_RECURSE SRC_NUKLEAR third_party/nuklear/*.cpp third_party/nuklear/*.h)
file(GLOB_RECURSE SRC_UTIL third_party/util/*.cpp)

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Add "use / create precompiled headers" (msvc only)
list(REMOVE_ITEM SRC_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.cpp)
ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "src/stdafx.cpp" SRC_MAIN)

add_executable(fatou ${SRC_MAIN} 
                     ${SRC_NUKLEAR} 
                     ${SRC_UTIL} 
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.cpp)

IF(WIN32) 
  IF(MSVC) 
    set_target_properties(fatou PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fatou)
  ENDIF()
ENDIF()


#////////////////////////////////////////////////////////////////#
#*********************[   Cpy Res to bin   ]*********************#

file(GLOB_RECURSE DATA RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" res/*)
file(GLOB_RECURSE HELP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" help/*)

foreach(p IN LISTS DATA) 
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${p} ${FATOU_BUILD_DIR}/${p} COPYONLY) 
endforeach() 

foreach(p IN LISTS HELP) 
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${p} ${FATOU_BUILD_DIR}/${p} COPYONLY) 
endforeach() 

#file(COPY ${DATA} ${FATOU_BUILD_DIR} res)
#file(COPY ${HELP} ${FATOU_BUILD_DIR} help)


#////////////////////////////////////////////////////////////////#
#*********************[    Dependencies    ]*********************#

#if (USE_OPENGL)
#    target_compile_definitions(fatou PUBLIC HAS_OPENGL_BACKEND=1)
#endif()

# hide those (otherwise, change INTERNAL to BOOL):
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(GLFW_DOCUMENT_INTERNALS OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_HYBRID_HPG OFF CACHE INTERNAL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "" FORCE)
set(LIB_SUFFIX OFF CACHE INTERNAL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE INTERNAL "" FORCE)

add_subdirectory(third_party/glfw)
target_link_libraries(fatou glfw)

###

set(glfw3_DIR "${CMAKE_SOURCE_DIR}/third_party/glfw" CACHE PATH "" FORCE)
# hide those (otherwise, change INTERNAL to BOOL):
set(OPTION_BUILD_GPU_TESTS OFF CACHE INTERNAL "" FORCE)
set(OPTION_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(OPTION_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
set(OPTION_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(OPTION_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(OPTION_SELF_CONTAINED OFF CACHE INTERNAL "" FORCE)

add_subdirectory(third_party/glbinding)
target_link_libraries(fatou glbinding)

###

#find_package(OpenGL REQUIRED)
#target_link_libraries(fatou ${OPENGL_LIBRARIES})

