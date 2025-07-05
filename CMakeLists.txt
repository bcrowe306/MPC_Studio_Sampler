cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(MPCSBC VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)

#Sigslot
FetchContent_Declare(
  sigslot
  GIT_REPOSITORY https://github.com/palacaze/sigslot
)
FetchContent_MakeAvailable(sigslot)
target_link_libraries(${PROJECT_NAME} PRIVATE Pal::Sigslot)

# Include and Subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ui)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/control_surface)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/ui)

find_package(stduuid CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE stduuid)


# fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        11.2.0) # 10.2.1
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp) # The library or executable that require yaml-cpp library

# Cairo
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIROMM REQUIRED IMPORTED_TARGET cairomm-1.16)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::CAIROMM)

# RTMidi
find_package(rtmidi CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RtMidi::rtmidi)

# CHOC
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/lib/choc
)


# LabSound
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/LabSound)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/LabSound/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bin/LabSound_d.framework/LabSound_d)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bin/libLabSoundMiniAudio_d.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bin/libLabSoundRtAudio_d.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/lib/LabSound/third_party/libnyquist/lib/liblibnyquist_d.a)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/lib/LabSound/third_party/libsamplerate/src/libsamplerate.a)


# Resources

set(RESOURCE_DIRS assets config)

foreach(dir ${RESOURCE_DIRS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${dir}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${dir}"
        COMMENT "Copying ${dir} to output directory"
    )
endforeach()


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