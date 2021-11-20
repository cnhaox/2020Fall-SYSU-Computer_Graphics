#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for the Ray Tracing Weekend Series
#
# See README.md for guidance.
#---------------------------------------------------------------------------------------------------

cmake_minimum_required ( VERSION 3.1.0 )

project ( RTWeekend
  VERSION 3.0.0
  LANGUAGES CXX
)

# import assimp
find_package(ASSIMP 5.0)
IF (${ASSIMP_FOUND})
    MESSAGE(STATUS "Found assimp.")
ELSE (${ASSIMP_FOUND})
    MESSAGE(STATUS "Could not locate assimp.")
    add_subdirectory(vendor/assimp)
ENDIF (${ASSIMP_FOUND})

# import glm
set(glm_DIR vendor/glm/cmake/glm)
find_package(glm REQUIRED)

# import stb_image
add_library(stb vendor/stb/stb_image.cpp vendor/stb/stb_image_write.cpp)
target_include_directories(stb PUBLIC vendor/stb)

set ( SOURCE_ONE_WEEKEND
  src/InOneWeekend/PathTracer.cpp
  src/InOneWeekend/main.cpp
  src/InOneWeekend/hitable_list.cpp
  src/InOneWeekend/bvh_node.cpp
  src/InOneWeekend/model.cpp
  src/InOneWeekend/transformer.cpp
)
# Executables
add_executable(inOneWeekend ${SOURCE_ONE_WEEKEND})

# enable C++ 17 standard
target_compile_features(inOneWeekend PUBLIC cxx_std_17)
set_target_properties(inOneWeekend PROPERTIES CXX_EXTENSIONS OFF)
# target_compile_options(inOneWeekend PRIVATE -Wall -Wextra)

# compile options
target_compile_definitions(inOneWeekend PRIVATE MAX_SAMPLE=2000 CONTINUE_PROBABILITY=0.9)

# check POSIX API
include(CheckIncludeFileCXX)
check_include_file_cxx(unistd.h HAVE_UNISTD)
if(HAVE_UNISTD)
    message(STATUS "Found unistd.h.")
    target_compile_definitions(inOneWeekend PRIVATE HAVE_UNISTD)
endif()

include_directories(include)
include_directories(src/common)

# link OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    target_link_libraries(inOneWeekend PUBLIC OpenMP::OpenMP_CXX)
endif()

# link assimp
if (${ASSIMP_FOUND})
    target_link_directories(inOneWeekend PRIVATE ${ASSIMP_LIBRARY_DIRS})
    target_include_directories(inOneWeekend PRIVATE ${ASSIMP_INCLUDE_DIRS})
    target_link_libraries(inOneWeekend PRIVATE ${ASSIMP_LIBRARIES})
else (${ASSIMP_FOUND})
    target_link_libraries(inOneWeekend PRIVATE assimp)
endif (${ASSIMP_FOUND})

# link glm
target_link_libraries(inOneWeekend PRIVATE glm::glm)

# link stb
target_link_libraries(inOneWeekend PRIVATE stb)

# assets
function(add_assets)
    foreach(src ${ARGN})
        configure_file(assets/${src} assets/${src} COPYONLY)
    endforeach(src)
endfunction()

add_assets(
    2k_earth_daymap.jpg
    person.obj
    person.png
    tree.obj
    tree.png
    cube.obj
    diamond.obj
    sky.jpg
    lowpolytree.obj
    lowpolytree.mtl
    old_house.obj
    old_house.mtl
    house/house_body.jpg
    house/plants1.jpg
    house/plants2.jpg
    christmas4t.png
    snowman.obj
)
