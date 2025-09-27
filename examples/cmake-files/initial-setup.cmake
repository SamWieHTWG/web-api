set(CMAKE_BUILD_TYPE Debug)

# === Get Linux architecture ===
execute_process(COMMAND uname -m OUTPUT_VARIABLE LINUX_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)

# === Paths ===
set(DEMO_SOURCE_DIR    ${CMAKE_SOURCE_DIR})
set(DEMO_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/../include)
set(DEMO_HELPER_DIR    ${CMAKE_SOURCE_DIR}/../src/)
set(DEMO_NC_PROGRAMS   ${CMAKE_SOURCE_DIR}/../components/example-nc-programs)
set(LIB_HEADER_DIR     ${CMAKE_SOURCE_DIR}/../../include)
set(WIN_INCLUDE_DIR    ${DEMO_INCLUDE_DIR}/windows)
set(PROP_HEADER_DIR    ${CMAKE_SOURCE_DIR}/../include)
set(WINDOWS_LIB_DIR    ${CMAKE_SOURCE_DIR}/../../lib/win64)
set(LINUX_LIB_DIR      ${CMAKE_SOURCE_DIR}/../../lib/${LINUX_ARCHITECTURE})
set(ERROR_FILE_DIR     ${CMAKE_SOURCE_DIR}/../../components/error/err_text_version_eng.txt)
set(DRIVE_SIMU_PATH    ${CMAKE_SOURCE_DIR})

set(LIB_DIR             $<IF:$<BOOL:${WIN32}>,${WINDOWS_LIB_DIR},${LINUX_LIB_DIR}>)

if(WIN32)
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/windows/demo_helper_w32.c)
else()
  list(APPEND SOURCE_FILES ${DEMO_HELPER_DIR}/linux/keyboard_helper.c)
  add_compile_definitions(_GNU_SOURCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_SOURCE_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

message(STATUS "DEMO_SOURCE_DIR:         ${DEMO_SOURCE_DIR}")
message(STATUS "DEMO_INCLUDE_DIR:        ${DEMO_INCLUDE_DIR}")
message(STATUS "LIB_DIR:                 ${LIB_DIR}")
message(STATUS "OUTPUT_DIRECTORY:        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# === Compiler options ===
add_compile_options(${ARCH_COMPILE_OPTIONS} ${COMMON_COMPILE_OPTIONS})

# === Includes ===
include_directories(
  ${TOOLCHAIN_INCLUDE_DIRS}
  ${DEMO_INCLUDE_DIR}
  ${OS_INCLUDE_DIRS}
  ${TARGET_SPECIFIC_INCLUDES}
  ${LIB_HEADER_DIR}
  ${PROP_HEADER_DIR}
)

if(WIN32)
include_directories(${WIN_INCLUDE_DIR})
endif()