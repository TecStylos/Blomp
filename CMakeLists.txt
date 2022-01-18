cmake_minimum_required(VERSION 3.8)

project("Blomp")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE_RELEASE)
    
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
    Blomp
    "src/BitStream.cpp"
    "src/BlockPool.cpp"
    "src/Blocks.cpp"
    "src/BlockTree.cpp"
    "src/BlockTreeGen.cpp"
    "src/Blomp.cpp"
    "src/Image.cpp"
    "src/ImgCompare.cpp"
    "vendor/stb_image/stb_image_write.cpp"
    "vendor/stb_image/stb_image.cpp"
)

target_include_directories(
    Blomp PUBLIC
    "src"
    "vendor/stb_image"
)

string(
	TOUPPER
	${CMAKE_BUILD_TYPE}
	UPPER_BUILD_TYPE
)

set(
    BLOMP_COMPILE_DEFINITIONS
    "BLOMP_BUILD_TYPE_${UPPER_BUILD_TYPE}"
)

target_compile_definitions(
    Blomp PUBLIC
    ${BLOMP_COMPILE_DEFINITIONS}
)