# Guard against rebuilding by accident.
if(NOT DEFINED UNICODE_BRIDGE_LIBRARY_INCLUDED)
  set(UNICODE_BRIDGE_LIBRARY_INCLUDED TRUE)
  # Set library name so that, if we change it, we only have to change this line
  set(UNICODE_BRIDGE_LIBRARY_NAME unicode_bridge)

  # Set the header files variable. This is a list of all the header files.
  set(UNICODE_BRIDGE_INTERNAL_HEADERS
  # Contains everything the library exposes. Most will use this.
    include/unicode_bridge.hpp
  )

  # Set the cpp files variable. This is a list of all the cpp files.
  set(UNICODE_BRIDGE_INTERNAL_SOURCES
    src/unicode_bridge.cpp
  )

  # Set the external header files variable. This is a list of all the external header files. They are libraries not built by abc_test.
  set(UNICODE_BRIDGE_EXTERNAL_HEADER_FILES
  )

  # Set the external cpp files variable. This is a list of all the external cpp files. Not made by abc_test.
  set(UNICODE_BRIDGE_EXTERNAL_SOURCE_FILES
  )

  # Sets how the IDE organises the files.
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. FILES
    ${UNICODE_BRIDGE_INTERNAL_SOURCES}
    ${UNICODE_BRIDGE_INTERNAL_HEADERS}
    ${UNICODE_BRIDGE_EXTERNAL_SOURCE_FILES}
    ${UNICODE_BRIDGE_EXTERNAL_HEADER_FILES}
  )

  # Adds the library itself.
  add_library(${UNICODE_BRIDGE_LIBRARY_NAME})

  # Sets the sources of the library.
  target_sources(${UNICODE_BRIDGE_LIBRARY_NAME}
    PRIVATE
      ${UNICODE_BRIDGE_INTERNAL_SOURCES}
      ${UNICODE_BRIDGE_EXTERNAL_SOURCE_FILES}
    PUBLIC
      FILE_SET public_headers TYPE HEADERS
      BASE_DIRS include
      FILES ${UNICODE_BRIDGE_INTERNAL_HEADERS}
    PRIVATE
      FILE_SET private_external_headers TYPE HEADERS
      BASE_DIRS include
      FILES ${UNICODE_BRIDGE_EXTERNAL_HEADER_FILES}
  )

  set_common_compile_options(${UNICODE_BRIDGE_LIBRARY_NAME})

  # Add the include directories to the target.
  target_include_directories(${UNICODE_BRIDGE_LIBRARY_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )

  target_link_libraries(${UNICODE_BRIDGE_LIBRARY_NAME} PUBLIC)

  # Required for gcc toolchain and the Clang toolchain under linux.
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    target_link_libraries(${UNICODE_BRIDGE_LIBRARY_NAME} PUBLIC stdc++_libbacktrace)
  endif()
endif()
