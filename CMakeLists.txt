cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)

project(evolution VERSION 1.0.0)

add_executable(evolution
  src/glad.c 
  src/stb_image.h
  src/main.cpp

  src/Math.cpp
  src/Mesh.cpp
  src/Shader.cpp
  src/Terrain.cpp
  src/Texture.cpp
)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

if (WIN32)
  target_link_libraries(evolution PRIVATE 
    "${CMAKE_SOURCE_DIR}/dependencies/lib/libSDL2.a"
    freetype
    imm32
    version
    setupapi
    winmm
  )

  target_include_directories(
    evolution PRIVATE
    dependencies/include
    dependencies/include/SDL2
  )

  # set_target_properties(evolution PROPERTIES LINK_FLAGS "-mwindows")
else()
  target_link_libraries(
    evolution PRIVATE
    /Library/Frameworks/SDL2.framework/Versions/A/SDL2
    # freetype
  )

  target_include_directories(
    evolution PRIVATE
    /Library/Frameworks/SDL2.framework/Versions/A/Headers
    # /opt/homebrew/Cellar/freetype/2.13.2/include/freetype2
    dependencies/include
  )
endif()
