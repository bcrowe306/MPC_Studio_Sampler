cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_TOOLCHAIN_FILE "/Users/brandoncrowe/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(MPCSBC VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)




add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)

#Sigslot
FetchContent_Declare(
  sigslot
  GIT_REPOSITORY https://github.com/palacaze/sigslot
  GIT_TAG        19a6f0f5ea11fc121fe67f81fd5e491f2d7a4637 # v1.2.0
)
FetchContent_MakeAvailable(sigslot)
target_link_libraries(${PROJECT_NAME} PRIVATE Pal::Sigslot)

# Include and Subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ui)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/device)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/ui)



find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIROMM REQUIRED IMPORTED_TARGET cairomm-1.16)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::CAIROMM)


find_package(rtmidi CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RtMidi::rtmidi)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/lib/choc
)


if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
  "-framework CoreMidi"
  "-framework Cocoa"
  "-framework Accelerate"
  "-framework AudioToolbox"
  "-framework AudioUnit"
  "-framework CoreServices"
  "-framework CoreAudio"
  "-framework Carbon"
  "-framework CoreFoundation"
    "-framework IOKit"
  )
endif()