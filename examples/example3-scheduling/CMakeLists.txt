cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

if(${CMAKE_VERSION} VERSION_LESS 3.0)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(CncSdkExample)


# === common Cmake ===
include(${CMAKE_SOURCE_DIR}/../cmake-files/initial-setup.cmake)

# === Source files ===
set(EXECUTABLE_NAME CncSdkExample)
set(SOURCE_FILES
  ${OS_FILES}
  ${CMAKE_SOURCE_DIR}/main.c
  ${DEMO_HELPER_DIR}/cnc_demo_samples.c
  ${CMAKE_SOURCE_DIR}/../src/cli_flags.c
)
if(WIN32)
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/windows/demo_helper_w32.c)
else()
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/linux/keyboard_helper.c)
endif()

# === Executable ===
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# === common Cmake ===
include(${CMAKE_SOURCE_DIR}/../cmake-files/final-setup.cmake)
