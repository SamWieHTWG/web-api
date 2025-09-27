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
  ${DEMO_SOURCE_DIR}/cnc_demo_main.c
  ${DEMO_HELPER_DIR}/cnc_demo_samples.c
  ${DEMO_SOURCE_DIR}/cnc_demo_drive_simulation.c
  ${DEMO_SOURCE_DIR}/cnc_demo_self_test.c
  ${DEMO_SOURCE_DIR}/cnc_demo_user_func.c
)
if(WIN32)
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/windows/demo_helper_w32.c)
else()
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/linux/keyboard_helper.c)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/include
)

# === Executable ===
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# === common Cmake ===
include(${CMAKE_SOURCE_DIR}/../cmake-files/final-setup.cmake)

set(DEMO_FILES_DIR    ${CMAKE_SOURCE_DIR}/demo-files)
if(EXISTS "${DEMO_FILES_DIR}" AND IS_DIRECTORY "${DEMO_FILES_DIR}")
  add_custom_command(TARGET CncSdkExample POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/listen"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${DEMO_FILES_DIR}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  COMMENT "Copying demo lists")
endif()


# file(GLOB CHILD_DIRS RELATIVE "${DEMO_FILES_DIR}" "${SRC_DDEMO_FILES_DIRIR}/*")

# foreach(dir ${CHILD_DIRS})
#   if(IS_DIRECTORY "${DEMO_FILES_DIR}/${dir}")
#     add_custom_command(
#       TARGET CncSdkExample POST_BUILD
#       COMMAND ${CMAKE_COMMAND} -E copy_directory
#               "${DEMO_FILES_DIR}/${dir}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
#       COMMENT "Copying subdir ${dir}"
#     )
#   endif()
# endforeach()
