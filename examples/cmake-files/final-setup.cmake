# === Linking ===
if(WIN32)
  include_directories(${DEMO_SOURCE_DIR}/windows)
  target_link_libraries(${EXECUTABLE_NAME} ${WINDOWS_LIB_DIR}/libCncSDK.lib)
else()
  include_directories(${DEMO_SOURCE_DIR}/linux)
  find_library(CNCSDK_LIB NAMES CncSDK CncKernel PATHS ${LINUX_LIB_DIR} NO_DEFAULT_PATH REQUIRED)
  message(STATUS "Resolved CNCSDK_LIB: ${CNCSDK_LIB}")
  target_link_libraries(${EXECUTABLE_NAME} ${CNCSDK_LIB} stdc++ pthread m dl rt)
endif()

# === Post-build copies ===
if(WIN32)
  add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WINDOWS_LIB_DIR}/libCncSDK.dll"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
  foreach(SO_FILE IN ITEMS libCncSDK.so libCncSDK.so.1.5.0.0)
    if(EXISTS "${SO_FILE}")
      add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LINUX_LIB_DIR}/${SO_FILE}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
  endforeach()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/cfg")
  add_custom_command(TARGET CncSdkExample POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/cfg" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/listen")
endif()

add_custom_command(TARGET CncSdkExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/../components/example-cfg/" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/listen")

add_custom_command(TARGET CncSdkExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/diagnose)

add_custom_command(TARGET CncSdkExample POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/error
  COMMAND ${CMAKE_COMMAND} -E copy ${ERROR_FILE_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/error/err_text_version_eng.txt)

add_custom_command(TARGET CncSdkExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${DEMO_NC_PROGRAMS}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prg")
